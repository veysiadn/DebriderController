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
        m_TargetVel = 0;
        m_Oscillate = 0;
        m_TargetPos = 0;        
        m_CloseBlade=0;
        m_emergency=0;
        m_NewTargetVel=0;
        m_Osc_State=false;
        btn_MAXRPM_LastState=0;
        btn_ChangeDirLastState=0;
        m_closeBladeWindow=0;
    }
    virtual void run();
    long m_NewTargetVel;
    int m_TargetVel;
    int m_Oscillate;
    bool m_Osc_State;
    bool m_running;
    int m_CloseBlade;            // VysADN CloseBlade function parameters
    int m_TargetPos;            
    int m_emergency;
    int m_closeBladeWindow;
    QElapsedTimer timer;
    bool WD_State=true;
    serial_com arduino;
private:
    // CKim - Motor class
    int m_currState, m_prevState;
    MaxonMotor m_Motor;
    int m_RightPedalDown;
    int m_LeftPedalDown;
    int btn_ChangeDirLastState;
    int btn_MAXRPM_LastState;
signals:
    void UpdateGUI(int state);
};




#endif // MOTORTHREAD_H
