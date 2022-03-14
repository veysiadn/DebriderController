/*****************************************************************************
 * \file  motorthread.h
 * \brief Header file includes motorThread class blueprint which is
 * encapsulation of main thread of debrider controller, using QThread.
 *
 * Last Updated : 2021.10.18 Chunwoo Kim (CKim) & Veysi ADIN (VysAdn)
 * Contact Info : cwkim@kist.re.kr & veysi.adin@kist.re.kr
 *******************************************************************************/


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


    /// CKim - Control loop is implemented in run() function
    virtual void run();

    void ReInitialize();

    /// CKim - Variables
public:
    int m_DebriderDesiredSpeed;
    int m_DebriderInstantSpeed;
    int m_DebriderTargetSpeed;
    int m_Oscillate;


    int m_TargetPos;            
    int m_Emergency;

    int m_GuiBtnCloseBlade;       /// CKim - Close button pressed
    int m_GuiChangePresetRPM;     /// CKim - Left Foot pedal button clicked
    int m_GuiBtnChangeDirection;  /// CKim - Right Foot pedal button clicked
    int m_GuiEmergencyMode;       /// CKim - Flag indicating notification of the emergency to GUI. 1 if notified

private:
    MaxonMotor m_Motor;                  // CKim - Motor class
    FootPedal m_FootPedal;               // CKim - SPI Communication class for foot pedal
    QElapsedTimer m_WatchdogTimer;       // CKim - Timer class for watchdog
    EposThread m_EposThread;             // CKim - Thread class for doing time consuming jobs

    int  m_CurrState, m_PrevState;
    int  m_RightPedalClicked;
    int  m_LeftPedalClicked;
    int  m_RightButtonClicked;
    int  m_LeftButtonClicked;
    bool m_WatchdogState;

    int m_CloseBlade;            // VysADN CloseBlade function parameters
    int m_LeftPedalDown;
    int m_LeftPedalDepth;
private:
    void ProcessPedalButtons();
    void PulseWatchDog();
    void CalculateDesiredVelocity(int& desired_vel, int pedal_depth,int target_vel);

private slots:
    // CKim - Callbacks from EPOS thread
    void on_InitComplete(int state);
    void on_TransToOscComplete(int errcode);
    void on_BladeClosed(int errcode);

    // CKim - Callbacks from foot pedal
    void on_FootPedalLButton();
    void on_FootPedalRButton();
    void on_RightFootPedal();
    void on_LeftFootPedal();
    void on_SPIStateChanged(int state);
signals:
    void UpdateGUI(int state);  // CKim - This signal is emitted to notify window for GUI update
};

#endif // MOTORTHREAD_H
