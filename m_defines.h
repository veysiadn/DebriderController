#ifndef M_DEFINES_H
#define M_DEFINES_H

#define BLDC_MAX_RPM        15000   // Maximum RPM value for BLDC Motor
#define CHANGE_RPM_RATE     1000    // Increment and decrement values for BLDC MOTOR RPM
#define CLOSE_BLADE_VELOCITY 500    // VeysiAdn - Velocity that will be used in close blade mode.
#define PUMP_PWM_RANGE      480     // Maximum RPM value for Pump Motor
#define PUMP_MAX_PWM        480
// Pump Motor PIN MAP
#define PUMP_HARDPWM    26          // CKim - GPIO 12,Physically Pin 32
#define PUMP_ENABLE     1           // CKim - GPIO 18, Physically Pin 12
#define PUMP_DIR        5           // CKim - GPIO 24, Physically Pin 18
#define VALVE_ENABLE    21          // VysAdn - GPIO5, Physically Pin 29

#define EMERGENCY_RELAY_CONTROL    7       // VysAdn - GPIO 4,  Physically Pin 7
#define WATCHDOG_PIN               0       // VysAdn - GPIO 17, Physically Pin 11

//  DEBRIDER_STATES CKim Starts 
#define DEBRIDER_STATE_EMERGENCY        -5
#define DEBRIDER_STATE_SERIAL_ERROR     -4
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
