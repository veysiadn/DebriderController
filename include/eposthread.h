/**
 ***************************************************************************
 * \file  eposthread.h
 * \brief Header file includes EposThread class, which implements time
 * consuming processes related to EPOS controller, such as initialization
 * process and waiting for position command to complete, in seperate thread
 * using QThread.
 *
 * Last Updated : 2022.03.16 Chunwoo Kim (CKim) & Veysi ADIN (VysAdn)
 * Contact Info : cwkim@kist.re.kr & veysi.adin@kist.re.kr
 ***************************************************************************
**/


#ifndef EPOSTHREAD_H
#define EPOSTHREAD_H

#include <QThread>

#include "epos4_can.h"
#include "footpedal.h"
#include "m_defines.h"
#include "wiringPi.h"

class EposThread : public QThread
{
    Q_OBJECT

public:

    EposThread();

    /**
     * @brief Sets motor to access created variables and
     * member functions of MaxonMotor class instance.
     * @param p MaxonMotor instance
     */
    void SetMotor(MaxonMotor* p)    {   m_pMotor = p;           }

    /**
     * @brief Sets foot pedal to access created variable and member
     * functions of the FootPedal class instance.
     * @param p FootPedal class instance.
     */
    void SetPedal(FootPedal* p)     {   m_pPedal = p;           }

    /**
     * @brief Sets the transition type
     * \see TransitionTypes enum ;   type 1 : init , type 2 : close blade , type 3 : oscillation.
     * @param type selected type.
     */
    void SetTransition(int type)    {   m_Transition = type;    }

    /**
     * @brief Aborts the oscillation mode operation.
     */
    void Abort()                    {   m_Abort = 1;            }

    /**
     * @brief Sets oscillation mode velocity.
     * @param s velocity value to use in oscillation mode.
     */
    void SetOscillationVelocity(int s)   {   m_OscSpeed=s;      }
    /**
     * @brief Gets waiting for motion flag.
     * @return flag describing waiting for motion action.
     */
    bool GetWaitingForMotionInfo() {return waiting_for_motion_; }
private:
    /// MaxonMotor class instance to access created variables and member functions in Motor thread.
    MaxonMotor* m_pMotor;
    /// FootPedal class instance to access created variables and member functions in Footpedal thread.
    FootPedal* m_pPedal;
    /// Transition flag. \see TransitionTypes enum.
    int m_Transition;               // 1: from uninit to init / 2: to close blade / 3: to osc
    /// Abort flag to abort oscillation mode operation.
    int m_Abort;
    /// Oscillation speed value.
    int m_OscSpeed;
    bool waiting_for_motion_ = false;


private:
    /**
     * @brief Control loop implemented here.Thread entrance function.
     */
    virtual void run();
    /**
     * @brief Runs initialization procedure includes ;
     * Opening CAN port, enabling drives and waiting for operation switch.
     */
    void RunInitialization();
    /**
     * @brief Runs close blade mode, currently close blade mode
     * moves motor 30 degre per click of pedal.
     */
    void RunCloseBlade();
    /**
     * @brief Runs motor in oscillation mode, with desired velocity.
     */
    void RunOscillation();

signals:
    /**
     * @brief This signal is emitted to notify the result of the initialization
     * @param errcode Error parameter to share in case of error in initialization.
     */
    void InitializationComplete(int errcode);
    /**
     * @brief This signal is emitted to notify the result of close blade mode.
     * @param errcode Error parameter to share in case of error in initialization.
     */
    void CloseBladeComplete(int errcode);
    /**
     * @brief This signal is emitted to notify the result of oscillation mode.
     * @param errcode Error parameter to share in case of error in initialization.
     */
    void OscillationComplete(int errcode);
};

#endif // EPOSTHREAD_H
