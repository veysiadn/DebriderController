#include "eposthread.h"


EposThread::EposThread() :
    QThread ()
{
    m_Transition = 1;       // 1: from uninit to init / 2: to close blade / 3: to osc
}

void EposThread::run()
{
    if(m_Transition == 1)   {   RunInitialization();    }
    if(m_Transition == 2)   {   RunCloseBlade();        }
    if(m_Transition == 3)   {   RunOscillation();       }
}

void EposThread::RunInitialization()
{
    int waitCnt = 0;
    int errcode = 0;

    // CKim - First check the relay status, wait for 1 s until watchdog
    // starts pulsing and relay is closed to provide
    // power EPOS and establish CAN connection. If not return with error
    if(digitalRead(EMERGENCY_RELAY_CONTROL)==LOW)
    {
        std::cout<<"Waiting for relay\n";
        waitCnt++;
        msleep(1000);   // CKim - wait for 1 sec
    }

    // CKim - If relay is still low after 1 sec. Throw error
    if(digitalRead(EMERGENCY_RELAY_CONTROL)==LOW)
    {
        // ERRR : Relay is not closed
        std::cout<<"ERRR : Relay is not closing\n";
        errcode = -1;
        emit InitializationComplete(errcode);
        return;
    }
    std::cout<<"Relay is closed, starting to establish CAN and SPI\n";

    // ------------------------------------------------------------ //
    // CKim - CAN Initialization
    // ------------------------------------------------------------ //
    // CKim - Open and configure CAN Communication
    //m_pMotor->CloseAllDevice();
    if(!m_pMotor->OpenCANCommunication())
    {
        // ERRR : CAN Port was not opened or not properly configured
        errcode = -2;
        emit InitializationComplete(errcode);
        return;
    }
    //msleep(200);

    // CKim - Enable the motor. For some reason we need multiple tries here
    for(int i=0; i<3; i++)
    {
        if(!m_pMotor->EnableMotorController())
        {
            // ERRR : Clear fault and enabling motor failed
            errcode = -2;
//            emit InitializationComplete(errcode);
//            return;
        }
        else
        {
            errcode = 0;
        }
        msleep(200);
    }
    // ------------------------------------------------------------ //
    //m_pMotor->EnableVelocityMode();
    //msleep(200);
    if(errcode == 0)    std::cout<<"Initialization thread no error!\n";
    emit InitializationComplete(errcode);
    return;
}


void EposThread::RunCloseBlade()
{
    int tgtPos;
    m_pMotor->EnablePositionMode();
    m_pMotor->GetCurrentPositionAllDevice(tgtPos);
    tgtPos = tgtPos + INC_PER_ROTATION/12;
    m_pMotor->MovePosition(tgtPos);
    m_pMotor->WaitForMotion();
    emit CloseBladeComplete(m_pMotor->EPOSGetError());
}

//void EposThread::RunCloseBlade()
//{
//    m_pMotor->MoveInCloseBladeMode();
//    emit CloseBladeComplete(m_pMotor->EPOSGetError());
//}

void EposThread::RunOscillation()
{
    m_Abort = 0;
    int dir = 1;
    // Set Oscillation Motion
    if(m_OscSpeed < 0 ) m_OscSpeed*=-1;
    m_pMotor->EnablePositionModeWithSpeed(m_OscSpeed);
    m_pMotor->SetOscMode(15000);
    // Start motion and continue until m_Abort is set to true from outside
    while(!m_Abort)
    {
        m_pMotor->RunOscMode(dir);
        m_pMotor->WaitForMotion();
        dir *= -1;
    }
    m_pMotor->StopMotion();   // Stop oscillate
    emit OscillationComplete(m_pMotor->EPOSGetError());
}
