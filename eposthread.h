// --------------------------------------------------------------- //
// CKim - class "eposthread" for executing time consuming processes
// related to EPOS controller, such as initialization process and
// position command and wait, in separate thread Uses 'QThread'
// Last Updated : 2020.10.22 CKim & VysADN
// --------------------------------------------------------------- //

#ifndef EPOSTHREAD_H
#define EPOSTHREAD_H

#include <QThread>
#include "epos4_can.h"
#include "footpedal.h"

class EposThread : public QThread
{
    Q_OBJECT

public:

    EposThread();

    // CKim - Variables
    void SetMotor(MaxonMotor* p)    {   m_pMotor = p;           }
    void SetPedal(FootPedal* p)     {   m_pPedal = p;           }
    void SetTransition(int type)    {   m_Transition = type;    }
    void Abort()                    {   m_Abort = 1;            }
    void setOscillationRPM(int s)   {   m_OscSpeed=s;           }

private:
    MaxonMotor* m_pMotor;           // CKim - Motor class
    FootPedal* m_pPedal;            // CKim - Foot pedal class
    int m_Transition;               // 1: from uninit to init / 2: to close blade / 3: to osc
    int m_Abort;
    int m_OscSpeed;


private:
    // CKim - Control loop is implemented in run() function
    virtual void run();

    void RunInitialization();
    void RunCloseBlade();
    void RunOscillation();

signals:
    // CKim - This signal is emitted to notify the result of the initialization
    void InitializationComplete(int errcode);
    void CloseBladeComplete(int errcode);
    void OscillationComplete(int errcode);
};

#endif // EPOSTHREAD_H
