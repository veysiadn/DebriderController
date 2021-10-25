/*****************************************************************************
 * \file  m_defines.h
 * \brief Header file includes global definitions, including configuration
 * values, pin mapping, states and maximum/minumum values.
 *
 * Last Updated : 2021.10.18 Chunwoo Kim (CKim) & Veysi ADIN (VysAdn)
 * Contact Info : cwkim@kist.re.kr & veysi.adin@kist.re.kr
 *******************************************************************************/

#ifndef M_DEFINES_H
#define M_DEFINES_H

#define BLDC_MAX_RPM                12000   // Maximum RPM value for BLDC Motor
#define CHANGE_RPM_RATE             1000    // Increment and decrement values for BLDC MOTOR RPM
#define INC_PER_ROTATION            20480   // VysAdn   1024(EncoderResolution)*5(Gear Ration) * 4

#define PUMP_PWM_RANGE              480     // Maximum RPM value for Pump Motor

//************************** PINMAP RASPBERRY PI STARTS ******************************************/
#define PUMP_HARDPWM                12      // CKim   - GPIO12, Physically Pin 32 / For suction pump
#define VALVE_ENABLE                13      // VysAdn - GPIO13, Physically Pin 33 / PWM1/ For selonoid valve.

#define EMERGENCY_RELAY_CONTROL     4       // VysAdn - GPIO 4,  Physically Pin 7
#define WATCHDOG_PIN                17      // VysAdn - GPIO 17, Physically Pin 11
//VysAdn Soft SPI Pins definitions.
#define ADC_SPI_CLK                 18      // VysAdn - GPIO16, Physically Pin 36
#define ADC_SPI_MISO                19      // VysAdn - GPIO18, Physically Pin 12
#define ADC_SPI_MOSI                20      // VysAdn - GPIO19, Physically Pin 35
#define ADC_SPI_CS                  16      // VysAdn - GPIO20, Physically Pin 38

#define  FOOT_PEDAL_L_BUTTON        22      // VysAdn - GPIO22, Physically Pin 15
#define  FOOT_PEDAL_R_BUTTON        23      // VysAdn - GPIO23, Physically Pin 16
#define  FOOT_PEDAL_R_PEDAL         24      // VysAdn - GPIO24, Physically Pin 18

//************************** PINMAP RASPBERRY PI ENDS ******************************************/
#define ADC_MAX_CHANNEL                 3       // ADC has 4 analog inputs.
#define ADC_VALUE_THRESHOLD             770     // ADC threshold value to convert digital HIGH 0.75*1023
//  DEBRIDER_STATES CKim Starts 
#define DEBRIDER_STATE_EMERGENCY        -5
#define DEBRIDER_STATE_SPI_ERROR        -4
#define DEBRIDER_STATE_EPOS_ERROR       -3
#define DEBRIDER_STATE_UNINIT           -2
#define DEBRIDER_STATE_INITIALIZING     -1
#define DEBRIDER_STATE_INIT             0
#define DEBRIDER_STATE_ENABLED          1

#define DEBRIDER_STATE_RUNNING          2
#define DEBRIDER_STATE_OSC              3
#define DEBRIDER_STATE_CLOSE_BLADE      4
#define DEBRIDER_STATE_BLADE_CLOSED     5

//#define TRUE 1
//#define FALSE 0


#endif // M_DEFINES_H
