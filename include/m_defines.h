/**
 ***************************************************************************
 * \file  m_defines.h
 * \brief Header file includes global definitions, including configuration
 * values, pin mapping, states and maximum/minumum values.
 *
 * Last Updated : 2022.03.16 Chunwoo Kim (CKim) & Veysi ADIN (VysAdn)
 * Contact Info : cwkim@kist.re.kr & veysi.adin@kist.re.kr
 *****************************************************************************
**/

#ifndef M_DEFINES_H
#define M_DEFINES_H

#define ENCODER_RESOLUTION  1024            /// 1024 Counts Per Turn
#define GEAR_RATIO          5               /// 5 : 1 Gear Ration
#define CLOSE_BLADE_VELOCITY 100            /// VeysiAdn - Velocity that will be used in close blade mode.
#define INC_PER_ROTATION    ENCODER_RESOLUTION*GEAR_RATIO*4 /// Encoder increment value per rotation.
#define OSC_MODE_AMP    INC_PER_ROTATION     /// VeysiAdn - Oscillation mode position oscillation range. -15k ~ +15k


#define MAX_ANALOG_OUTPUT           310     /// Maximum analog value acquired from pedal.
#define BLDC_MAX_RPM                12000   /// Maximum RPM value for BLDC Motor
#define CHANGE_RPM_RATE             1000    /// Increment and decrement values for BLDC MOTOR RPM
#define PUMP_PWM_RANGE              480     /// Maximum PWM value for Pump Motor (%100)
#define OSC_MODE_MAX_RPM            2500

#define MIN_ANALOG_PEDAL_OUTPUT      1      /// Minimum output voltage of analog pedal.
#define MAX_ANALOG_PEDAL_OUTPUT     3.3     /// Maximim output voltage of analog pedal (unpressed).
#define ADC_MAX_OUTPUT              1023    /// Maximum output of ADC.
#define ADC_MIN_OUTPUT               0      /// Minimum output of ADC.

#define ADC_MIN_PEDAL_OUTPUT        310     /// This value corresponds to 1V (min voltage of analog pedal) in ADC.
/// To calculate this value you can use function:
/// long map(long x, long in_min, long in_max, long out_min, long out_max){
///     return (x-in_min)*(out_max-out_min)/(in_max - in_min) + out_min;
/// }
/// val = map(1,0,3.3,0,1023). here (0-3.3V range mapped to 0-1023, and ADC value of 1V will return 310.)
///
#define ADC_MAX_CHANNEL                  3         /// ADC has 4 analog inputs.
#define ADC_VALUE_THRESHOLD             950       /// ADC threshold value to move the motor.

//************************** PINMAP RASPBERRY PI STARTS ******************************************/
#define SUCTION_MOTOR_PWM          12       /// CKim   - GPIO12, Physically Pin 32 / For suction pump
#define SOLENOID_VALVE_ENABLE      13       /// VysAdn - GPIO13, Physically Pin 33 / PWM1/ For selonoid valve.

#define EMERGENCY_RELAY_CONTROL     4       /// VysAdn - GPIO 4,  Physically Pin 7 / Watchdog && Emergency button outputs.
#define WATCHDOG_PIN                17      /// VysAdn - GPIO 17, Physically Pin 11 / Watchdog pin. 5ms periodic pulse.
#define INITIALIZATION_SWTICH       5       /// VysADN - GPIO5, Physically Pin 29  / Init switch for init screen.
#define VDD_RESET                   18      /// VysAdn - GPIO18, Physically Pin 12 / Watchdog power reset/enable pin.
//VysAdn Soft SPI Pins definitions.
#define ADC_SPI_CLK                 21      /// VysAdn - GPIO21, Physically Pin 40  / SPI clock pin for ADC
#define ADC_SPI_MISO                19      /// VysAdn - GPIO19, Physically Pin 35  / SPI MISO pin for ADC
#define ADC_SPI_MOSI                20      /// VysAdn - GPIO20, Physically Pin 38  / SPI MOSI pin for ADC
#define ADC_SPI_CS                  16      /// VysAdn - GPIO16, Physically Pin 36  / SPI CS pin for ADC

#define  FOOT_PEDAL_L_BUTTON        22      /// VysAdn - GPIO22, Physically Pin 15  / Foot pedal left button pin
#define  FOOT_PEDAL_R_BUTTON        23      /// VysAdn - GPIO23, Physically Pin 16  / Foot pedal right button pin
#define  FOOT_PEDAL_L_PEDAL_BUTTON  24      /// VysAdn - GPIO24, Physically Pin 18  / Foot pedal left pedal button pin
#define  FOOT_PEDAL_R_PEDAL_BUTTON  25      /// VysADN - GPIO25, Physically Pin 22  / Foot pedal right pedal button pin
#define  PEDAL_BUZZER               27      /// VysADN - GPIO27, Physically Pin 13  / Foot pedal buzzer pin, needs PWM input.
//************************** PINMAP RASPBERRY PI ENDS ******************************************/


//************************** DEBRIDER STATES STARTS ******************************************/
///  DEBRIDER_STATES  Starts
#define DEBRIDER_STATE_EMERGENCY        -5
#define DEBRIDER_STATE_SPI_ERROR        -4
#define DEBRIDER_STATE_EPOS_ERROR       -3
#define DEBRIDER_STATE_UNINIT           -2
#define DEBRIDER_STATE_INITIALIZING     -1
#define DEBRIDER_STATE_INIT              0
#define DEBRIDER_STATE_READY             1

#define DEBRIDER_STATE_ENABLED          2
#define DEBRIDER_STATE_RUNNING          3
#define DEBRIDER_STATE_OSC              4
#define DEBRIDER_STATE_CLOSE_BLADE      5
#define DEBRIDER_STATE_BLADE_CLOSED     6
//************************** DEBRIDER STATES ENDS ******************************************/
/// Transition types in epos thread.
enum TransitionTypes{
    kInit=1,
    kCloseBlade,
    kRunOscillation
};

//#define TRUE 1
//#define FALSE 0


#endif // M_DEFINES_H
