#include "include/eposthread.h"


EposThread::EposThread() :
    QThread ()
{
    m_Transition = 1;       // 1: from uninit to init / 2: to close blade / 3: to osc
}

void EposThread::run()
{
    if(m_Transition == kInit)                {   RunInitialization();    }
    if(m_Transition == kCloseBlade)          {   RunCloseBlade();        }
    if(m_Transition == kRunOscillation)      {   RunOscillation();       }
}

void EposThread::RunInitialization()
{
    int state = DEBRIDER_STATE_INIT;
    // ------------------------------------------------------------ //
    // CKim - CAN Initialization
    // ------------------------------------------------------------ //
    // CKim - Open and configure CAN Communication
    //m_pMotor->CloseAllDevice();
    if(!m_pMotor->OpenCANCommunication())
    {
        // ERRR : CAN Port was not opened or not properly configured
        std::cout << "Open CAN communication failed.\n";
        state = DEBRIDER_STATE_EPOS_ERROR;
        emit InitializationComplete(state);
        return;
    }
    // CKim - Enable the motor. For some reason we need multiple tries here
    for(int i=0; i<3; i++)
    {
        if(!m_pMotor->EnableMotorController())
        {
            // ERRR : Clear fault and enabling motor failed
            state = DEBRIDER_STATE_EPOS_ERROR;
        }
        else
        {
            state = DEBRIDER_STATE_INIT;
            emit InitializationComplete(state);
            break;
        }
        msleep(200);
    }
    if(state==DEBRIDER_STATE_EPOS_ERROR)
    {
        emit InitializationComplete(state);
        std::cout << "Enabling motor failed " << std::endl;
        return ;
    }
    // ------------------------------------------------------------ //
    while(state!= DEBRIDER_STATE_ENABLED){
    //    int waitCnt = 0;
        // CKim - First check the relay status, wait for 1 s until watchdog
        // starts pulsing and relay is closed to provide
        // power EPOS and establish CAN connection. If not return with error
        if(digitalRead(EMERGENCY_RELAY_CONTROL)==LOW)
        {
            std::cout<<"Waiting for relay\n";
    //        waitCnt++;
            msleep(1000);   // CKim - wait for 1 sec
        }

        // CKim - If relay is still low after 1 sec. Throw error
        if(digitalRead(EMERGENCY_RELAY_CONTROL)==LOW)
        {
            // ERRR : Relay is not closed
            std::cout<<"ERRR : Relay is not closing\n";
            state = DEBRIDER_STATE_EMERGENCY;
            emit InitializationComplete(state);
            return;
        }
        std::cout<<"Relay is closed, checking init switch\n";
        state=DEBRIDER_STATE_INITIALIZING;
        while(state==DEBRIDER_STATE_INITIALIZING){
            if(digitalRead(INITIALIZATION_SWTICH)==HIGH){
                msleep(500);
                if(digitalRead(EMERGENCY_RELAY_CONTROL)==LOW){
                    state=DEBRIDER_STATE_EMERGENCY;
                    emit InitializationComplete(state);
                    break;
                }else {
                    state=DEBRIDER_STATE_INITIALIZING;
                }
                emit InitializationComplete(state);
            }else {
                state = DEBRIDER_STATE_READY;
            }
        }
        if(m_pMotor->EPOSGetError()){
            state = DEBRIDER_STATE_EPOS_ERROR;
            emit InitializationComplete(state);
            return;
        }

        if(digitalRead(FOOT_PEDAL_R_PEDAL_BUTTON)==LOW) {
            state = DEBRIDER_STATE_SPI_ERROR ;
            emit InitializationComplete(state);
            return;
        }

        while(digitalRead(INITIALIZATION_SWTICH)==LOW && state == DEBRIDER_STATE_READY){
            if(digitalRead(EMERGENCY_RELAY_CONTROL)==LOW){
                std::cout << "Relay opened" << std::endl;
                state=DEBRIDER_STATE_EMERGENCY;
                emit InitializationComplete(state);
                return;
            }
            if(digitalRead(FOOT_PEDAL_R_PEDAL_BUTTON)==LOW){
                std::cout << "Foot pedal Removed." << std::endl;
                state=DEBRIDER_STATE_SPI_ERROR;
                emit InitializationComplete(state);
                return;
            }
            if(m_pMotor->EPOSGetError()){
                std::cout << "Motor error occured." << std::endl;
                state=DEBRIDER_STATE_EPOS_ERROR;
                emit InitializationComplete(state);
                return;
            }
            msleep(100);
            state=DEBRIDER_STATE_READY;
            emit InitializationComplete(state);
        }
        state = DEBRIDER_STATE_ENABLED;
        emit InitializationComplete(state);
     }
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
    m_pMotor->SetOscMode(OSC_MODE_AMP);
    // Start motion and continue until m_Abort is set to true from outside
    while(!m_Abort)
    {
        m_pMotor->RunOscMode(dir);
        waiting_for_motion_ = true;
        m_pMotor->WaitForMotion();
        if(m_pMotor->EPOSGetError()) emit OscillationComplete(1);
        dir *= -1;
    }
    m_pMotor->StopMotion();   // Stop oscillate
    emit OscillationComplete(m_pMotor->EPOSGetError());
    waiting_for_motion_ = false;
}
