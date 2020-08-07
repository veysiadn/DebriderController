#ifndef SERIAL_COM_H
#define SERIAL_COM_H

#include <QSerialPort>
#include <QSerialPortInfo>
#include <qserialportinfo.h>
#include <QThread>

class serial_com : public QThread
{   
private:

    virtual void run();
    bool s_errorFlag = false;               // Error flag for serial communication errors.
    QSerialPort serial;
    
    // ### VysADN 2 byte serial communication protocol with arduino nano every variables for reciever (Raspberry Pi3b+) side.  ### (Details about these variables is explained in readData() function)
    unsigned char FirstByte, SecondByte;
    int NumRead=0 ,numReadTotal=0;
    char get_Pack[50]={};

    // ### Arduino Nano Every product id and vendor id , universal for all Arduino Nano Every ###
    static const quint16 ArduinoNanoEvery_vendor_id =9025;
    static const quint16 ArduinoNanoEvery_product_id =88;

public:
    serial_com(){
        btn_MAXRPM=0;
        btn_CloseBlade=0;
        btn_ChangeDirection=0;
        Get_Analog_Pedal_Val=0;
    }

    int btn_MAXRPM;

    int btn_CloseBlade;

    int btn_ChangeDirection;

    int Get_Analog_Pedal_Val;

    void SerialSetError(bool s);

    bool SerialGetError();

    void ClearPort();

    void readData();

    void openSerialPort();

    void closeSerialPort();
};

#endif // SERIAL_COM_H
