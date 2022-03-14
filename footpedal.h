/*****************************************************************************
 * \file  footpedal.h
 * \brief Header file includes footpedal class for opening port and reading
 * data from foot pedal; reads one analog and three digital values from
 * pedal.Uses Soft SPI protocol to communicate with MCP3004 ADC.
 *
 * Last Updated : 2021.10.18 Chunwoo Kim (CKim) & Veysi ADIN (VysAdn)
 * Contact Info : cwkim@kist.re.kr & veysi.adin@kist.re.kr
 *******************************************************************************/


#ifndef FOOTPEDAL_H
#define FOOTPEDAL_H

#include <QThread>
#include <wiringPi.h>
#include "m_defines.h"
#include <time.h> // nanosleep

#ifndef Int8U
    typedef unsigned char Int8U;
#endif

class FootPedal : public QThread
{   
    Q_OBJECT

public:
    FootPedal();
    ~FootPedal();
    void OpenSPIPort();
    void CloseSPIPort();
    bool GetSPIError()                      {   return adc_error_flag;    }
    int  GetLeftPedalValue()                {   return L_pedal_value;     }

    void SetSPIError(bool s);
    void ClearSPIPort();

private:
    virtual void run();
    int  ReadSPIAnalog(int pin_channel);
    void GetCmdOutInfo(const int pin_channel, Int8U& cmd_out, int& cmd_out_bits_count);
    void TickClock();
    void ReadButtons();

    bool adc_error_flag = false;               // Error flag for SPI communication errors.
    int L_button, prev_L_button;
    int R_button, prev_R_button;
    int L_pedal_value;
    int R_pedal, prev_R_pedal;
    int L_pedal, prev_L_pedal;
    int adc_spi_running;

signals:
    void RButtonClicked();      // CKim - This signal will be emitted when right button is clicked
    void LButtonClicked();      // CKim - This signal will be emitted when left button is clicked
    void RPedalClicked();       // CKim - This signal will be emitted when right pedal is clicked
    void LPedalClicked();       // CKim - This signal will be emitted when left pedal is clicked

};

#endif // FOOTPEDAL_H
