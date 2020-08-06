#include "serial_com.h"
#include <iostream>
void serial_com::run()
{
    openSerialPort();
    for (;;) 
    {
        if(!s_errorFlag && serial.isOpen())
        {
        readData();
        } 
        else 
        {
            if(serial.isOpen())closeSerialPort();
            if(serial.isOpen()) ClearPort();
            if(!serial.isOpen()) openSerialPort();
        }
    }
}


// VYSADN Serial PORT OPEN START
void serial_com::openSerialPort()
{
       bool arduino_is_available = false;
       QString arduino_uno_port_name;
        //
        foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
        {
         //  check if the serialport has both a product identifier and a vendor identifier
            if(serialPortInfo.hasProductIdentifier() && serialPortInfo.hasVendorIdentifier())
            {
                //  check if the product ID and the vendor ID match those of the arduino uno
                if((serialPortInfo.productIdentifier() == arduino_uno_product_id) && (serialPortInfo.vendorIdentifier() == arduino_uno_vendor_id))
                {
                    arduino_is_available = true; //    arduino uno is available on this port
                    arduino_uno_port_name = serialPortInfo.portName();
                }
            }
        }
        if(arduino_is_available)
        {
        serial.setPortName(arduino_uno_port_name);
        //std::cout<< arduino_uno_port_name.toStdString() << std::endl;
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
        if(serial.isOpen()) serial.close();
        if (!serial.isOpen()) std::cout << "Serial Port Closed" << std::endl;
}
void serial_com::readData()
{
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

    if((numReadTotal % 2 == 0 && NumRead!=0))
    {
            btn_MAXRPM              = (FirstByte) & 1U;
            btn_CloseBlade          = (FirstByte >> 1) & 1U;
            btn_ChangeDirection     = (FirstByte >> 2) & 1U;
            Get_Analog_Pedal_Val    = ((FirstByte >> 6) | (SecondByte << 2)) & 1023;

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



