/**
 ***************************************************************************
 * \file  motorthread.h
 * \brief Header file includes motorThread class blueprint which is
 * encapsulation of main thread of debrider controller, using QThread.
 *
 * Last Updated : 2022.03.16 Chunwoo Kim (CKim) & Veysi ADIN (VysAdn)
 * Contact Info : cwkim@kist.re.kr & veysi.adin@kist.re.kr
 *****************************************************************************
**/


#ifndef MOTORTHREAD_H
#define MOTORTHREAD_H

#include <QThread>
#include <QElapsedTimer>
#include "epos4_can.h"
#include "m_defines.h"
#include "wiringPi.h"
#include "footpedal.h"
#include "eposthread.h"

class MotorThread : public QThread
{
    Q_OBJECT

public:

    MotorThread();


    ///  Control loop is implemented in run() function
    virtual void run();

    void ReInitialize();

public:

    /// Value describes target velocity value to send to motor after calculating based on analog pedal input.
    int m_DebriderDesiredSpeed;

    /// Value describes current velocity of debirder motor.
    int m_DebriderInstantSpeed;

    /// Value describes target velocity value set by user.
    int m_DebriderTargetSpeed;

    /// Oscillation mode activation/deactivation flag.
    int m_Oscillate;

    /// Target position to move motor to desired position.
    int m_TargetPos;

    /// Emergency mode activation/deactivation flag.
    int m_Emergency;

    ///  - Flag representing Close blade button pressed in GUI.
    int m_GuiBtnCloseBlade;

    ///  - Flag representing left button clicked in pedal to notify GUI.
    int m_GuiChangePresetRPM;

    ///  - Flag representing right button clicked in pedal to notify GUI.
    int m_GuiBtnChangeDirection;

    ///  - Flag indicating notification of the emergency to GUI. 1 if notified.
    int m_GuiEmergencyMode;

private:
    /// - Motor class instance to access interface for controlling motor motion.
    MaxonMotor m_Motor;

    /// - SPI Communication class to get button and analog pedal data from foot pedal.
    FootPedal m_FootPedal;

    /// Timer class for pulsing watchdog in specific interval. Current interval is 5ms
    QElapsedTimer m_WatchdogTimer;

    /// - Thread class for doing time consuming jobs to not miss watchdog pulse.
    EposThread m_EposThread;

    /// Values representing current and previous state of debrider.
    int  m_CurrState, m_PrevState;

    /// Flag set by foot pedal class when right pedal is clicked.
    int  m_RightPedalClicked;

    /// Flag set by foot pedal class when left pedal is clicked.
    int  m_LeftPedalClicked;

    /// Flag set by foot pedal class when right button is clicked.
    int  m_RightButtonClicked;

    /// Flag set by foot pedal class when left button is clicked.
    int  m_LeftButtonClicked;

    /// Flag representing HIGH or LOW state of watchdog.
    bool m_WatchdogState;

    /// Flag representing close blade function call required or not.
    int m_CloseBlade;

    /// Flag representing if user pressed analog pedal or not.
    int m_LeftPedalDown;

    /// Value representing analog pedal value.
    int m_LeftPedalDepth;

private:

    /**
     * @brief Notifies GUI in case of change in pedal buttons. Uptades all flags
     * related to pedal buttons.
     */
    void ProcessPedalButtons();

    /**
     * @brief Pulses watchdog output with 5ms period.
     */
    void PulseWatchDog();

    /**
     * @brief Calculates desired velocity based on target velocity and analog pedal value.
     * @param desired_vel output value.
     * @param pedal_depth analog pedal value.
     * @param target_vel target velocity set by user.
     */
    void CalculateDesiredVelocity(int& desired_vel, int pedal_depth,int target_vel);

private slots:

    /// - Callback from EPOS thread. Called when initialization is complete.
    void on_InitComplete(int state);

    /// - Callback from EPOS thread. Called when oscillation mode is complete.
    void on_TransToOscComplete(int errcode);

    /// - Callback from EPOS thread. Called when blade close function is complete.
    void on_BladeClosed(int errcode);

    /// - Callback from foot pedal. Called when left button is clicked in pedal.
    void on_FootPedalLButton();

    /// - Callback from foot pedal. Called when right button is clicked in pedal.
    void on_FootPedalRButton();

    /// - Callback from foot pedal. Called when right pedal is clicked in pedal.
    void on_RightFootPedal();

    /// - Callback from foot pedal. Called when left pedal is pressed in pedal.
    void on_LeftFootPedal();

    /// - Callback from foot pedal. Called when SPI communication state changed.
    void on_SPIStateChanged(int state);

signals:
    /// - This signal is emitted to notify window for GUI update.
    void UpdateGUI(int state);
};

#endif // MOTORTHREAD_H
