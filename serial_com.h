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
    bool s_errorFlag = false;
    unsigned char FirstByte, SecondByte;
    int NumRead=0 ,numReadTotal=0;
    char get_Pack[50]={};
    static const quint16 arduino_uno_vendor_id =9025;
    static const quint16 arduino_uno_product_id =88;

public:
    serial_com(){
        btn_MAXRPM=0;
        btn_CloseBlade=0;
        btn_ChangeDirection=0;
        Get_Analog_Pedal_Val=0;
    }

    QSerialPort serial;

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
