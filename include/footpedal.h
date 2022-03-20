/**
 ***************************************************************************
 * \file  footpedal.h
 * \brief Header file includes footpedal class for opening port and reading
 * data from foot pedal; reads one analog and three digital values from
 * pedal.Uses Soft SPI protocol to communicate with MCP3004 ADC.
 *
 * Last Updated : 2022.03.16 Chunwoo Kim (CKim) & Veysi ADIN (VysAdn)
 * Contact Info : cwkim@kist.re.kr & veysi.adin@kist.re.kr
 ****************************************************************************
**/


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
    /**
     * @brief OpenSPIPort Opens SPI port, by assigning inputs and outputs of SPI pins..
     */
    void OpenSPIPort();
    /**
     * @brief CloseSPIPort Closes SPI port by assigning all pins as INPUTs.
     */
    void CloseSPIPort();
    /**
     * @brief GetSPIError Gets SPI error.
     * @return error flag.
     */
    bool GetSPIError()                      {   return adc_error_flag;    }
    /**
     * @brief GetLeftPedalValue Gets analog pedal value by reading ADC value via SPI.
     * @return Analog pedal value.
     */
    int  GetLeftPedalValue()                {   return L_pedal_value;     }
    /**
     * @brief SetSPIError Sets SPI error, in case there is an error in SPI communication
     * @param s boolean val true if error exists, false if no error.
     */
    void SetSPIError(bool s);
    /**
     * @brief ClearSPIPort Clears all parameters acquired via SPI.
     */
    void ClearSPIPort();

private:
    /**
     * @brief run SPI reading loop implemented here. This is thread entrance point.
     */
    virtual void run();
    /**
     * @brief ReadSPIAnalog Reads data from spefied pin channel in MCP3004 ADC.
     * @param pin_channel ADC channel
     * @note MCP3004 has 4 channels from 0 to 3.
     * @return Analog value in specified channel.
     */
    int  ReadSPIAnalog(int pin_channel);
    /**
     * @brief GetCmdOutInfo This function is an helper function to read ADC value.
     * It is specific to MCP3004. For more details check MCP3004 datasheet.
     * @param pin_channel pin channel of ADC.
     * @param cmd_out Command output
     * @param cmd_out_bits_count number of command output bits.
     */
    void GetCmdOutInfo(const int pin_channel, Int8U& cmd_out, int& cmd_out_bits_count);
    /**
     * @brief TickClock Tick clock is to manually send clock signal to SPI CLK pin.
     * Currently period is 2ms (500Hz). (1ms HIGH, 1ms LOW).
     */
    void TickClock();
    /**
     * @brief ReadButtons Reads connected pedal buttons via digitalRead function of wiringPi
     * and emits signal when there's a change in the pedal value in rising edge.
     */
    void ReadButtons();

    /// Error flag for SPI communication errors.
    bool adc_error_flag = false;

    /// Button parameters, to detect rising edge of buttons.
    int L_button, prev_L_button;

    /// Button parameters, to detect rising edge of buttons.
    int R_button, prev_R_button;

    /// Analog value parameter.
    /// @todo This must be changed to R_pedal_value, in new pedal analog input is in right.
    int L_pedal_value;

    /// Currently this parameter is unused, will be used if the pedal is purely digital.
    int R_pedal, prev_R_pedal;

    /// Button parameters, to detect rising edge of buttons.
    int L_pedal, prev_L_pedal;

    /// Flag for SPI communication state.
    int adc_spi_running;

signals:
    /// This signal will be emitted when right button is clicked
    void RButtonClicked();

    ///  This signal will be emitted when left button is clicked
    void LButtonClicked();

    ///  This signal will be emitted when right pedal is clicked
    void RPedalClicked();

    ///  This signal will be emitted when left pedal is clicked
    void LPedalClicked();

};

#endif // FOOTPEDAL_H
