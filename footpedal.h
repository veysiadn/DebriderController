// --------------------------------------------------------------- //
// CKim - class "footpedal" for opening and reading serial
// communication from the foot pedal
// Last Updated : 2020.10.20 CKim & VysADN
// --------------------------------------------------------------- //

#ifndef FOOTPEDAL_H
#define FOOTPEDAL_H

#include <QSerialPort>
#include <QSerialPortInfo>
#include <qserialportinfo.h>
#include <QThread>

class FootPedal : public QThread
{   
    Q_OBJECT

public:
    FootPedal();

    void openSerialPort();
    void closeSerialPort();
    bool getSerialError()               {   return serialErrorFlag;         }
    void getLeftPedalValue(int& L)      {   L = LPedal;                     }

    void setSerialError(bool s);
    void clearSerialPort();


private:
    virtual void run();
    void readSerialPort();

    bool serialErrorFlag = false;               // Error flag for serial communication errors.
    //QSerialPort serialArduinoNanoEvery;
    QSerialPort* pSerialPort;
    
    // ### VysADN 2 byte serial communication protocol with arduino nano every variables for reciever (Raspberry Pi3b+) side.  ### (Details about these variables is explained in readData() function)
    unsigned char FirstByte, SecondByte;
    int numReadByte=0 , numTotalReadByte=0;
    char recievedSerialPack[50]={};

    // ### Arduino Nano Every product id and vendor id , universal for all Arduino Nano Every ###
    static const quint16 ArduinoNanoEvery_vendor_id =9025;
    static const quint16 ArduinoNanoEvery_product_id =88;

//    int pedalBtnMaxRPM;
//    int pedalBtnCloseBlade;
//    int pedalBtnChangeDirection;
//    int pedalAnalogBLDCval;

    int LButton, prevLButton;
    int RButton, prevRButton;
    int LPedal;
    int RPedal, prevRPedal;

    int  arduinoSerialRunning;

signals:
    void RbuttonClicked();      // CKim - This signal will be emitted when right button is clicked
    void LbuttonClicked();      // CKim - This signal will be emitted when left button is clicked
    void RPedalClicked();       // CKim - This signal will be emitted when right pedal is clicked

};

#endif // FOOTPEDAL_H
