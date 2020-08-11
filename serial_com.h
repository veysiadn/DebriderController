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
    bool serialErrorFlag = false;               // Error flag for serial communication errors.
    QSerialPort serialArduinoNanoEvery;
    
    // ### VysADN 2 byte serial communication protocol with arduino nano every variables for reciever (Raspberry Pi3b+) side.  ### (Details about these variables is explained in readData() function)
    unsigned char FirstByte, SecondByte;
    int numReadByte=0 , numTotalReadByte=0;
    char recievedSerialPack[50]={};

    // ### Arduino Nano Every product id and vendor id , universal for all Arduino Nano Every ###
    static const quint16 ArduinoNanoEvery_vendor_id =9025;
    static const quint16 ArduinoNanoEvery_product_id =88;

public:
    serial_com(){
        pedalBtnMaxRPM=0;
        pedalBtnCloseBlade=0;
        pedalBtnChangeDirection=0;
        pedalAnalogBLDCval=0;
        serialArduinoRunning=1;
    }

    int pedalBtnMaxRPM;

    int pedalBtnCloseBlade;

    int pedalBtnChangeDirection;

    int pedalAnalogBLDCval;

    int  serialArduinoRunning;

    void setSerialError(bool s);

    bool getSerialError();

    void clearSerialPort();

    void readSerialPort();

    void openSerialPort();

    void closeSerialPort();
};

#endif // SERIAL_COM_H
