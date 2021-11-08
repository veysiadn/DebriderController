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
    int guiChangePresetRPM;           // CKim - Left Foot pedal button clicked
    int guiBtnChangeDirection;  // CKim - Right Foot pedal button clicked
    int guiEmergencyMode;       // CKim - Flag indicating notification of the emrgency to GUI. 1 if notified

private:
    MaxonMotor m_Motor;                  // CKim - Motor class
    FootPedal m_FootPedal;               // CKim - SPI Communication class for foot pedal
    QElapsedTimer watchDogTimer;         // CKim - Timer class for watchdog
    QElapsedTimer testTimer;             // VysADN - test timing information.
    QElapsedTimer on_off_timer;
    EposThread m_eposThread;             // CKim - Thread class for doing time consuming jobs
    int off_on_counter = 0;
    int counter=0 ;
    bool on_off_state = true;
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
    void on_InitComplete(int errcode);
    void on_TransToOscComplete(int errcode);
    void on_BladeClosed(int errcode);

    // CKim - Callbacks from foot pedal
    void on_FootPedalLButton();
    void on_FootPedalRButton();
    void on_RightFootPedal();

signals:
    void UpdateGUI(int state);  // CKim - This signal is emitted to notify window for GUI update
};




#endif // MOTORTHREAD_H
