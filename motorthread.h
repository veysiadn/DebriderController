// --------------------------------------------------------------- //
// CKim - class "motorthread" encapsulating main thread of the
// debrider controller. Uses 'QThread'
// Last Updated : 2020.10.19 CKim & VysADN
// --------------------------------------------------------------- //
#ifndef MOTORTHREAD_H
#define MOTORTHREAD_H

#include <QThread>
#include "epos4_can.h"
#include <QElapsedTimer>
#include <m_defines.h>
#include "wiringPi.h"
#include <footpedal.h>
#include "eposthread.h"

class motorThread : public QThread
{
    Q_OBJECT

public:

    motorThread();


    // CKim - Control loop is implemented in run() function
    virtual void run();

    void ReInitialize();

    // CKim - Variables
public:
    int m_DebriderInstantSpeed;
    int m_DebriderTargetSpeed;
    int m_Oscillate;


    int m_TargetPos;            
    int m_emergency;

    int guiBtnCloseBlade;       // CKim - Close button pressed
    int guiBtnMaxRPM;           // CKim - Left Foot pedal button clicked
    int guiBtnChangeDirection;  // CKim - Right Foot pedal button clicked
    int guiEmergencyMode;       // CKim - Flag indicating notification of the emrgency to GUI. 1 if notified

private:
    MaxonMotor m_Motor;                 // CKim - Motor class
    FootPedal m_FootPedal;              // CKim - Serial Communication class for foot pedal
    QElapsedTimer watchDogTimer;        // CKim - Timer class for watchdog??
    EposThread m_eposThread;             // CKim - Thread class for doing time consuming jobs

    int m_currState, m_prevState;
    int m_RightPedalClicked;
    int m_RightButtonClicked;
    int m_LeftButtonClicked;
    bool watchDogState;

    int m_CloseBlade;            // VysADN CloseBlade function parameters
    int m_LeftPedalDown;
    int m_LeftPedalDepth;

private:
    void ProcessPedalButtons();
    void PulseWatchDog();

private slots:
    // CKim - Callbacks from EPOS thread
    void OnInitComplete(int errcode);
    void OnTransToOscComplete(int errcode);
    void OnBladeClosed(int errcode);

    // CKim - Callbacks from foot pedal
    void OnFootPedalLButton();
    void OnFootPedalRButton();
    void OnRightFootPedal();

signals:
    void UpdateGUI(int state);  // CKim - This signal is emitted to notify window for GUI update
};




#endif // MOTORTHREAD_H
