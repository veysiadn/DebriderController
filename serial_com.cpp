#include "serial_com.h"
#include <iostream>
void serial_com::run()
{
    for (;;)
    {
        while(s_run)
        {
            if(!serial.isOpen()) openSerialPort();
            if(!s_errorFlag && serial.isOpen()){ readData(); }
            else
            {
                closeSerialPort();
                break;
            }
        }
    }
}


// VYSADN Serial PORT OPEN START
void serial_com::openSerialPort()
{
       bool arduino_is_available = false;
       QString ArduinoNanoEvery_port_name;
        //
        foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
        {
         //  check if the serialport has both a product identifier and a vendor identifier
            if(serialPortInfo.hasProductIdentifier() && serialPortInfo.hasVendorIdentifier())
            {
                //  check if the product ID and the vendor ID match those of the arduino uno
                if((serialPortInfo.productIdentifier() == ArduinoNanoEvery_product_id) && (serialPortInfo.vendorIdentifier() == ArduinoNanoEvery_vendor_id))
                {
                    arduino_is_available = true; //    arduino uno is available on this port
                    ArduinoNanoEvery_port_name = serialPortInfo.portName();
                }
            }
        }
        if(arduino_is_available)
        {
        serial.setPortName(ArduinoNanoEvery_port_name);
        //std::cout<< ArduinoNanoEvery_port_name.toStdString() << std::endl;
        s_errorFlag=false;
        }
        else
        {
            std::cout<< "Arduino not available" << std::endl;
            s_errorFlag=true;
        }

        if(!serial.setBaudRate(QSerialPort::Baud9600))
        {
            std::cout<< "BAUD RATE PROBLEM"<<std::endl;
            s_errorFlag=true;
        }

        if(!serial.setDataBits(QSerialPort::Data8))
        {
        std::cout<< "DATA BIT PROBLEM"<<std::endl;
        s_errorFlag=true;
        }

        if(!serial.setParity(QSerialPort::NoParity))
        {
        std::cout<< "PARITY SET PROBLEM"<<std::endl;
        s_errorFlag=true;
        }

        if(!serial.setFlowControl(QSerialPort::NoFlowControl))
        {
        std::cout<< "FLOW PROBLEM"<<std::endl;
        s_errorFlag=true;
        }

        if(!serial.setStopBits(QSerialPort::OneStop))
        {
         std::cout<< "STOP BIT PROBLEM"<<std::endl;
         s_errorFlag=true;
        }
        if(!serial.open(QIODevice::ReadOnly))
        {
         //qDebug() << serial.errorString();
         std::cout<< "OPEN DEVICE PROBLEM"<<std::endl;
         s_errorFlag=true;
        }
}
// SerialPortOpen Finish

void serial_com::closeSerialPort()
{
        serial.close();
        if(serial.isOpen())
        {
            std::cout << "Serial Close failed, port is already open.Error closeSerialPort" << std::endl;
            s_errorFlag=true;
        }
}
void serial_com::readData()
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
        Before transmitting the values arduino shifts FirstByte_transmitter 6 bit left to and shifts SecondByte_transmitter 2 bit to the right   
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
   NumRead = serial.read(get_Pack,2);

    if(numReadTotal % 2 == 1)
    {
            if(NumRead==1)
                SecondByte = (unsigned char)get_Pack[0];
            if(NumRead==2) 
            {
                FirstByte = (unsigned char)get_Pack[1];
                SecondByte = (unsigned char)get_Pack[0];

            }
    }
    else 
    {
        if (NumRead==1)
        {
            FirstByte = (unsigned char)get_Pack[0];
        }
        if(NumRead==2)
        {
            FirstByte = (unsigned char)get_Pack[0];
            SecondByte = (unsigned char)get_Pack[1];
        }


    }
        numReadTotal+=NumRead;
    // ## when we recieve 2 bytes, we can procces recieved package.
    if((numReadTotal % 2 == 0 && NumRead!=0))
    {           
            btn_MAXRPM              = (FirstByte) & 1U;       //   {|A1|A0|RS|RS|RS|D2|D1|D0|}  &   00000001 = D0   = ButtonMaxState     
            btn_CloseBlade          = (FirstByte >> 1) & 1U;  //   {|0|A1|A0|RS|RS|RS|D2|D1|}   &  00000001  = D1   = ButtonCloseBladeState
            btn_ChangeDirection     = (FirstByte >> 2) & 1U;  //   {|0|0|A1|A0|RS|RS|RS|D2|}    & 00000001   = D2   = ButtonChangeDirState
            Get_Analog_Pedal_Val    = ((FirstByte >> 6) | (SecondByte << 2)) & 1023;  
            // {|0|0|0|0|0|0|A1|A0|} | {A9|A8|A7|A6|A5|A4|A3|A2|0|0|} & 111111111  = {|A9|A8|A7|A6|A5|A4|A3|A2|A1|A0|} = Analog value from pedal

            if(Get_Analog_Pedal_Val < 20) Get_Analog_Pedal_Val=0;
                //           std::cout<< "Number of Read bytes =  " << NumRead << std::endl;
                //           std::cout<< "bytes available = " << bytes_Avail << std::endl;
                //          std::cout<< "Analog Pedal Val  = " << Get_Analog_Pedal_Val << std::endl;
                //          std::cout<< "Button state max = " << btn_MAXRPM << std::endl;
                //          std::cout<< "Button state close = " << btn_CloseBlade << std::endl;
                //          std::cout<< "Button Change Direction = " << btn_ChangeDirection << std::endl;
                //           std::cout<< "------------------------------------------------------------" << std::endl;
    }
    if (!NumRead && !serial.waitForReadyRead())
    {
        std::cout<< "Serial Timeout " << std::endl;
        s_errorFlag=true;
        return;
    }

}
void serial_com::ClearPort()
{
    if(!serial.clear(QSerialPort::AllDirections))
    {
        std::cout<< "Serial Clear Failed ..." << std::endl;
        s_errorFlag=true;
    }
}
void serial_com::SerialSetError(bool s)
{
    s_errorFlag = s;
}
bool serial_com::SerialGetError()
{
    return s_errorFlag;
}



