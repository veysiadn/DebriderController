#ifndef MOTORTHREAD_H
#define MOTORTHREAD_H

#include <QThread>
#include "epos4_can.h"
#include <QElapsedTimer>
#include <m_defines.h>
#include <wiringPi.h>
#include <serial_com.h>
class motorThread : public QThread
{
    Q_OBJECT

public:

    motorThread() {
        m_currState = DEBRIDER_STATE_INIT;
        m_prevState = DEBRIDER_STATE_INIT;
        m_DebriderTargetSpeed = 0;
        m_Oscillate = 0;
        m_TargetPos = 0;        
        m_CloseBlade=0;
        m_emergency=0;
        m_DebriderInstantSpeed=0;
        pedalBtnMaxRPM_LastState=0;
        pedalBtnChangeDir_LastState=0;
        guiBtnCloseBlade=0;
        guiBtnMaxRPM=0;
        guiBtnChangeDirection=0;
        guiEmergencyMode=0;
    }
    virtual void run();
    int m_DebriderInstantSpeed;
    int m_DebriderTargetSpeed;
    int m_Oscillate;
    bool m_running;
    int m_CloseBlade;            // VysADN CloseBlade function parameters
    int m_TargetPos;            
    int m_emergency;
    int guiBtnCloseBlade;
    int guiBtnMaxRPM;
    int guiBtnChangeDirection;
    int guiEmergencyMode;
private:
    // CKim - Motor class
    int m_currState, m_prevState;
    MaxonMotor m_Motor;
    int m_RightPedalDown;
    int m_LeftPedalDown;
    int pedalBtnChangeDir_LastState;
    int pedalBtnMaxRPM_LastState;
    serial_com serialArduino;
    QElapsedTimer watchDogTimer;
    bool watchDogState=true;
private:
    void setBtnMaxRPMGUI();
    void setBtnChangeDirectionGUI();
    void initWatchDog();
signals:
    void UpdateGUI(int state);
};




#endif // MOTORTHREAD_H
