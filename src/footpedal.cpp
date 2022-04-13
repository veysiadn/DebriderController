#include "include/footpedal.h"
#include "include/m_defines.h"

#include <iostream>
#include <wiringPi.h>
#include <time.h>

FootPedal::FootPedal()
{

    adc_spi_running=1;

    L_button = prev_L_button = 0;
    R_button = prev_R_button = 0;
    L_pedal_value = R_pedal = 0;

}

void FootPedal::run()
{
    int left_pedal_val;
    adc_spi_running=1;
    while(true)
    {
        while(adc_spi_running)
        {
            left_pedal_val = ReadSPIAnalog(0);
            ReadButtons();
            L_pedal_value = left_pedal_val;

        }
    }

}

int FootPedal::ReadSPIAnalog(int pin_channel)
{
    Int8U cmd_out;
      int cmd_out_bits;
      int value_out = 0;

      if (pin_channel < 0 || pin_channel > ADC_MAX_CHANNEL) {
          adc_error_flag = true ;
          return -1;
      }

      // obtain command out bits + pinChannel, based on chip
      GetCmdOutInfo(pin_channel, cmd_out, cmd_out_bits);

      // initiate communication with device
      // toggle cs and start clock low
      digitalWrite(ADC_SPI_CS, HIGH);
      digitalWrite(ADC_SPI_CLK, LOW);
      digitalWrite(ADC_SPI_CS, LOW);

      for (int i = 0; i < cmd_out_bits; ++i) {
        digitalWrite(ADC_SPI_MOSI, (cmd_out & 0x80) ? HIGH : LOW);
        cmd_out <<= 1; // shift out bit just used
        TickClock();
      }

      TickClock(); // read (skip) one empty bit

      // read 10 ADC bits
      for (int i = 0; i < 10; ++i) {
        TickClock();
        value_out <<= 1; // make room for next bit
        if (digitalRead(ADC_SPI_MISO) == HIGH) {
          value_out |= 1;
        }
      }

      TickClock(); // read (skip) null bit
      digitalWrite(ADC_SPI_CS, HIGH);

      return value_out;
}

void FootPedal::ReadButtons()
{
//    R_pedal = digitalRead(FOOT_PEDAL_R_PEDAL_BUTTON);
//   if(R_pedal > 0 ) emit(RPedalClicked());

      L_pedal  = digitalRead(FOOT_PEDAL_L_PEDAL_BUTTON);
    if(L_pedal !=  prev_L_pedal && L_pedal > 0)     emit(LPedalClicked());

      L_button = digitalRead(FOOT_PEDAL_L_BUTTON);
    if(L_button !=  prev_L_button && L_button > 0)  emit(LButtonClicked());

      R_button = digitalRead(FOOT_PEDAL_R_BUTTON);
    if(R_button !=  prev_R_button && R_button > 0)  emit(RButtonClicked());

    prev_L_pedal  = L_pedal;
    prev_L_button = L_button;
    prev_R_button = R_button ;
}

void FootPedal::OpenSPIPort()
{
    pinMode(ADC_SPI_CLK,OUTPUT);
    pinMode(ADC_SPI_MOSI,OUTPUT);
    pinMode(ADC_SPI_CS,OUTPUT);
    pinMode(ADC_SPI_MISO,INPUT);
    adc_spi_running = 1;
}

void FootPedal::CloseSPIPort()
{
    pinMode(ADC_SPI_CLK,INPUT);
    pinMode(ADC_SPI_MOSI,INPUT);
    pinMode(ADC_SPI_CS,INPUT);
    pinMode(ADC_SPI_MISO,INPUT);
    L_button = prev_L_button = 0;
    R_button = prev_R_button = 0;
    L_pedal_value = R_pedal = 0;
    adc_spi_running = 0;

}

void FootPedal::ClearSPIPort()
{
    pinMode(ADC_SPI_CLK,OUTPUT);
    pinMode(ADC_SPI_MOSI,OUTPUT);
    pinMode(ADC_SPI_CS,OUTPUT);
    pinMode(ADC_SPI_MISO,INPUT);
    L_button = prev_L_button = 0;
    R_button = prev_R_button = 0;
    L_pedal_value = R_pedal = 0;
}

void FootPedal::SetSPIError(bool s)
{
    adc_error_flag = s;
}

void FootPedal::GetCmdOutInfo(const int pin_channel, Int8U &cmd_out, int &cmd_out_bits_count)
{
    cmd_out = 0x18 | pin_channel;  // sgl and odd bits, plus the channel to be used
    cmd_out_bits_count = 5;
    cmd_out <<= (8 - cmd_out_bits_count);
}

void FootPedal::TickClock()
{
    digitalWrite(ADC_SPI_CLK, HIGH);
    nanosleep((const struct timespec []){{0,1000}},nullptr);
    digitalWrite(ADC_SPI_CLK, LOW);
    nanosleep((const struct timespec []){{0,1000}},nullptr);

}

FootPedal::~FootPedal()
{
    adc_spi_running = 0;
    // VysAdn ADC SPI input/output init.
    pinMode(ADC_SPI_CLK,INPUT);
    pinMode(ADC_SPI_MOSI,INPUT);
    pinMode(ADC_SPI_CS,INPUT);
    pinMode(ADC_SPI_MISO,INPUT);
    L_button = prev_L_button = 0;
    R_button = prev_R_button = 0;
    L_pedal_value = R_pedal = 0;
}
