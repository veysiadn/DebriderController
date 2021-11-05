#include <footpedal.h>
#include <iostream>
#include<wiringPi.h>
#include<m_defines.h>

FootPedal::FootPedal()
{
    arduinoSerialRunning=1;

    LButton = prevLButton = 0;
    RButton = prevRButton = 0;
    LPedal = RPedal = 0;

    pSerialPort = NULL;
}

void FootPedal::run()
{
    pSerialPort = new QSerialPort();
    closeSerialPort();
    openSerialPort();
    arduinoSerialRunning=1;
    while(true)
    {
        if(pSerialPort->isOpen()) closeSerialPort();
        while(arduinoSerialRunning)
        {
            if(!pSerialPort->isOpen())
            {
                openSerialPort();
            }
            if(!serialErrorFlag && pSerialPort->isOpen())
            {
                readSerialPort();
            }
            else if (serialErrorFlag)
            {
                arduinoSerialRunning=0;
                std::cout << "Serial Error detected" << std::endl;
                break;
            }
        }
    }

}

void FootPedal::openSerialPort()
{
    bool arduino_is_available = false;
    QString ArduinoNanoEvery_port_name;
    //
    foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
    {
        if(serialPortInfo.hasProductIdentifier() && serialPortInfo.hasVendorIdentifier())
        {
            if((serialPortInfo.productIdentifier() == ArduinoNanoEvery_product_id) && (serialPortInfo.vendorIdentifier() == ArduinoNanoEvery_vendor_id))
            {
                arduino_is_available = true; //    arduino uno is available on this port
                ArduinoNanoEvery_port_name = serialPortInfo.portName();
            }
        }
    }

    if(arduino_is_available)
    {
        pSerialPort->setPortName(ArduinoNanoEvery_port_name);
        //std::cout<< ArduinoNanoEvery_port_name.toStdString() << std::endl;
        serialErrorFlag=false;
    }
    else
    {
        std::cout<< "Arduino not available" << std::endl;
        serialErrorFlag=true;
    }

    if(!pSerialPort->setBaudRate(QSerialPort::Baud9600))
    {
        std::cout<< "BAUD RATE PROBLEM"<<std::endl;
        serialErrorFlag=true;
    }

    if(!pSerialPort->setDataBits(QSerialPort::Data8))
    {
        std::cout<< "DATA BIT PROBLEM"<<std::endl;
        serialErrorFlag=true;
    }

    if(!pSerialPort->setParity(QSerialPort::NoParity))
    {
        std::cout<< "PARITY SET PROBLEM"<<std::endl;
        serialErrorFlag=true;
    }

    if(!pSerialPort->setFlowControl(QSerialPort::NoFlowControl))
    {
        std::cout<< "FLOW PROBLEM"<<std::endl;
        serialErrorFlag=true;
    }

    if(!pSerialPort->setStopBits(QSerialPort::OneStop))
    {
        std::cout<< "STOP BIT PROBLEM"<<std::endl;
        serialErrorFlag=true;
    }
    if(!pSerialPort->open(QIODevice::ReadOnly))
    {
        //qDebug() << serial.errorString();
        std::cout<< "OPEN DEVICE PROBLEM"<<std::endl;
        serialErrorFlag=true;
    }
    std::cout<< "Serial port successfully opened"<<std::endl;
}

void FootPedal::closeSerialPort()
{
    pSerialPort->close();
    if(pSerialPort->isOpen())
    {
        std::cout << "Serial Close failed, port is already open.Error closeSerialPort" << std::endl;
        serialErrorFlag=true;
    }
}

void FootPedal::readSerialPort()
{
    /* Explanation of reading from serial port. Arduino sends 2 byte package in every 30ms.
       Pack that Raspberry Pi receives should be :  {|A9|A8|A7|A6|A5|A4|A3|A2|A1|A0|RSV|RSV|RSV|D2|D1|D0|}
    Since USB communication can send 1 byte (8 bit) at one cycle we're dividing our package into 2 parts in transmitter (ARDUINO NANO) side ; 2nd byte  : {|A9|A8|A7|A6|A5|A4|A3|A2|}
                                                                                                                                              1st byte  : {|A1|A0|RS|RS|RS|D2|D1|D0|}
    D0 = ButtonMaxState             ; 1 bit 0-1
    D1 = ButtonCloseBladeState      ; 1 bit 0-1
    D2 = ButtonChangeDirState       ; 1 bit 0-1
    RSV = Reserved bits.            ; Unused
    A0-A9 = Analog value from pedal ; 10 bit 0-1023 (2byte value)
        Before transmitting the values, arduino shifts FirstByte_transmitter(1stByte) 6 bit left to and shifts SecondByte_transmitter(2ndByte) 2 bit to the right
    //Explanation of shifting :  since analog value is 10 bit and we can only send 8 bit at one cycle we need to divide it to 8 + 2 bits.
        // 8 bit will be SecondByte  {|A9|A8|A7|A6|A5|A4|A3|A2|}   and FirstByte :  {|A1|A0|X|X|X|X|X|X|}
      // Analog value before shifting :              {|X|X|X|X|X|X|A9|A8|A7|A6|A5|A4|A3|A2|A1|A0|}
      // Analog value after 6 to the left shifting : {|A9|A8|A7|A6|A5|A4|A3|
                                                    FirstByte : {A2|A1|A0|0|0|0|0|0|0|}
                                                    the last 3 bits of FirstByte also contains ;
                                                        D0 = ButtonMaxState             ; 1 bit 0-1
                                                        D1 = ButtonCloseBladeState      ; 1 bit 0-1
                                                        D2 = ButtonChangeDirState       ; 1 bit 0-1
                                                        FirstByte final = {|A1|A0|RS|RS|RS|D2|D1|D0|}
      // Analog value after 2 to the right shifting : {|0|0|0|0|0|0|0|0|}
                                                    SecondByte  : {A9|A8|A7|A6|A5|A4|A3|A2|}
*/

    // ### Reading Serial PORT and making sure that we recieved 2 bytes, before proccesing analog and digital variables.
   numReadByte = pSerialPort->read(recievedSerialPack,2);

    if(numTotalReadByte % 2 == 1)
    {
            if(numReadByte==1)
                SecondByte = (unsigned char)recievedSerialPack[0];
            if(numReadByte==2)
            {
                FirstByte = (unsigned char)recievedSerialPack[1];
                SecondByte = (unsigned char)recievedSerialPack[0];

            }
    }
    else
    {
        if (numReadByte==1)
        {
            FirstByte = (unsigned char)recievedSerialPack[0];
        }
        if(numReadByte==2)
        {
            FirstByte = (unsigned char)recievedSerialPack[0];
            SecondByte = (unsigned char)recievedSerialPack[1];
        }
    }
    numTotalReadByte+=numReadByte;

    // ## when we recieve 2 bytes, we can procces recieved package.
    if((numTotalReadByte % 2 == 0 && numReadByte!=0))
    {
            LButton         = (FirstByte) & 1U;       //   {|A1|A0|RS|RS|RS|D2|D1|D0|}  &   00000001 = D0   = ButtonMaxState
            RPedal          = (FirstByte >> 1) & 1U;  //   {|0|A1|A0|RS|RS|RS|D2|D1|}   &  00000001  = D1   = ButtonCloseBladeState
            RButton     = (FirstByte >> 2) & 1U;  //   {|0|0|A1|A0|RS|RS|RS|D2|}    & 00000001   = D2   = ButtonChangeDirState
            LPedal    = ((FirstByte >> 6) | (SecondByte << 2)) & 1023;
            // {|0|0|0|0|0|0|A1|A0|} | {A9|A8|A7|A6|A5|A4|A3|A2|0|0|} & 111111111  = {|A9|A8|A7|A6|A5|A4|A3|A2|A1|A0|} = Analog value from pedal

            if(LPedal < 23) LPedal = 0;
                //           std::cout<< "Number of Read bytes =  " << numReadByte << std::endl;
                //           std::cout<< "bytes available = " << bytes_Avail << std::endl;
                //          std::cout<< "Analog Pedal Val  = " << Get_Analog_Pedal_Val << std::endl;
                //          std::cout<< "Button state max = " << LButton << std::endl;
                //          std::cout<< "Button state close = " << btn_CloseBlade << std::endl;
                //          std::cout<< "Button Change Direction = " << RButton << std::endl;
                //           std::cout<< "------------------------------------------------------------" << std::endl;
            numTotalReadByte=0;

            // CKim - L/R Button switches between 0 and 1 at each button press
            if(LButton != prevLButton)     emit LbuttonClicked();
            if(RButton != prevRButton)     emit RbuttonClicked();

            // CKim - Right Pedal is 1 when pressed, 0 when released
            if(prevRPedal == 0 && RPedal == 1)     emit RPedalClicked();

            prevLButton = LButton;
            prevRButton = RButton;
            prevRPedal = RPedal;
    }

    if (!numReadByte && !pSerialPort->waitForReadyRead())
    {
        std::cout<< "Serial Timeout " << std::endl;
        serialErrorFlag = true;
        return;
    }
}

void FootPedal::clearSerialPort()
{
    if(!pSerialPort->clear(QSerialPort::AllDirections))
    {
        std::cout<< "Serial Clear Failed ..." << std::endl;
        serialErrorFlag=true;
    }
}

void FootPedal::setSerialError(bool s)
{
    serialErrorFlag = s;
}



//void FootPedal::readSerialPort()
//{
//    /* Explanation of reading from serial port. Arduino sends 2 byte package in every 30ms.
//       Pack that Raspberry Pi receives should be :  {|A9|A8|A7|A6|A5|A4|A3|A2|A1|A0|RSV|RSV|RSV|D2|D1|D0|}
//    Since USB communication can send 1 byte (8 bit) at one cycle we're dividing our package into 2 parts in transmitter (ARDUINO NANO) side ; 2nd byte  : {|A9|A8|A7|A6|A5|A4|A3|A2|}
//                                                                                                                                              1st byte  : {|A1|A0|RS|RS|RS|D2|D1|D0|}
//    D0 = ButtonMaxState             ; 1 bit 0-1
//    D1 = ButtonCloseBladeState      ; 1 bit 0-1
//    D2 = ButtonChangeDirState       ; 1 bit 0-1
//    RSV = Reserved bits.            ; Unused
//    A0-A9 = Analog value from pedal ; 10 bit 0-1023 (2byte value)
//        Before transmitting the values arduino shifts FirstByte_transmitter 6 bit left to and shifts SecondByte_transmitter 2 bit to the right
//    //Explanation of shifting :  since analog value is 10 bit and we can only send 8 bit at one cycle we need to divide it to 8 + 2 bits.
//        // 8 bit will be SecondByte  {|A9|A8|A7|A6|A5|A4|A3|A2|}   and FirstByte :  {|A1|A0|X|X|X|X|X|X|}
//      // Analog value before shifting :              {|X|X|X|X|X|X|A9|A8|A7|A6|A5|A4|A3|A2|A1|A0|}
//      // Analog value after 6 to the left shifting : {|A9|A8|A7|A6|A5|A4|A3|
//                                                    FirstByte : {A2|A1|A0|0|0|0|0|0|0|}
//                                                    the last 3 bits of FirstByte also contains ;
//                                                        D0 = ButtonMaxState             ; 1 bit 0-1
//                                                        D1 = ButtonCloseBladeState      ; 1 bit 0-1
//                                                        D2 = ButtonChangeDirState       ; 1 bit 0-1
//                                                        FirstByte final = {|A1|A0|RS|RS|RS|D2|D1|D0|}
//      // Analog value after 2 to the right shifting : {|0|0|0|0|0|0|0|0|}
//                                                    SecondByte  : {A9|A8|A7|A6|A5|A4|A3|A2|}
//*/

//    // ### Reading Serial PORT and making sure that we recieved 2 bytes, before proccesing analog and digital variables.
//   numReadByte = pSerialPort->read(recievedSerialPack,2);

//    if(numTotalReadByte % 2 == 1)
//    {
//            if(numReadByte==1)
//                SecondByte = (unsigned char)recievedSerialPack[0];
//            if(numReadByte==2)
//            {
//                FirstByte = (unsigned char)recievedSerialPack[1];
//                SecondByte = (unsigned char)recievedSerialPack[0];

//            }
//    }
//    else
//    {
//        if (numReadByte==1)
//        {
//            FirstByte = (unsigned char)recievedSerialPack[0];
//        }
//        if(numReadByte==2)
//        {
//            FirstByte = (unsigned char)recievedSerialPack[0];
//            SecondByte = (unsigned char)recievedSerialPack[1];
//        }
//    }
//    numTotalReadByte+=numReadByte;

//    // ## when we recieve 2 bytes, we can procces recieved package.
//    if((numTotalReadByte % 2 == 0 && numReadByte!=0))
//    {
//            pedalBtnMaxRPM              = (FirstByte) & 1U;       //   {|A1|A0|RS|RS|RS|D2|D1|D0|}  &   00000001 = D0   = ButtonMaxState
//            pedalBtnCloseBlade          = (FirstByte >> 1) & 1U;  //   {|0|A1|A0|RS|RS|RS|D2|D1|}   &  00000001  = D1   = ButtonCloseBladeState
//            pedalBtnChangeDirection     = (FirstByte >> 2) & 1U;  //   {|0|0|A1|A0|RS|RS|RS|D2|}    & 00000001   = D2   = ButtonChangeDirState
//            pedalAnalogBLDCval    = ((FirstByte >> 6) | (SecondByte << 2)) & 1023;
//            // {|0|0|0|0|0|0|A1|A0|} | {A9|A8|A7|A6|A5|A4|A3|A2|0|0|} & 111111111  = {|A9|A8|A7|A6|A5|A4|A3|A2|A1|A0|} = Analog value from pedal

//            if(pedalAnalogBLDCval < 23) pedalAnalogBLDCval=0;
//                //           std::cout<< "Number of Read bytes =  " << numReadByte << std::endl;
//                //           std::cout<< "bytes available = " << bytes_Avail << std::endl;
//                //          std::cout<< "Analog Pedal Val  = " << Get_Analog_Pedal_Val << std::endl;
//                //          std::cout<< "Button state max = " << pedalBtnMaxRPM << std::endl;
//                //          std::cout<< "Button state close = " << btn_CloseBlade << std::endl;
//                //          std::cout<< "Button Change Direction = " << btn_ChangeDirection << std::endl;
//                //           std::cout<< "------------------------------------------------------------" << std::endl;
//            numTotalReadByte=0;
//    }
//    if (!numReadByte && !pSerialPort->waitForReadyRead())
//    {
//        std::cout<< "Serial Timeout " << std::endl;
//        serialErrorFlag=true;
//        return;
//    }

//}

