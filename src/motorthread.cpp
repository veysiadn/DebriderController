#include <motorthread.h>

MotorThread::MotorThread()
{
    m_CurrState = DEBRIDER_STATE_UNINIT;
    m_PrevState = DEBRIDER_STATE_UNINIT;
    m_DebriderTargetSpeed = 0;          // CKim - Debrider speed at maximum pedal press
    m_DebriderInstantSpeed=0;           // CKim - Current debrider speed.
    m_Oscillate  = 0;
    m_TargetPos  = 0;
    m_CloseBlade = 0;
    m_Emergency  = 0;

    m_LeftButtonClicked =   m_RightButtonClicked = m_RightPedalClicked = m_LeftPedalClicked = 0;

    m_GuiBtnCloseBlade=0;
    m_GuiChangePresetRPM=0;
    m_GuiBtnChangeDirection=0;
    m_GuiEmergencyMode=0;
    m_WatchdogState=true;

    m_EposThread.SetMotor(&m_Motor);
    m_EposThread.SetPedal(&m_FootPedal);
    connect(&m_EposThread,&EposThread::InitializationComplete,this,&MotorThread::on_InitComplete);
    connect(&m_EposThread,&EposThread::OscillationComplete,this,&MotorThread::on_TransToOscComplete);
    connect(&m_EposThread,&EposThread::CloseBladeComplete,this,&MotorThread::on_BladeClosed);

    connect(&m_FootPedal,&FootPedal::LButtonClicked,this,&MotorThread::on_FootPedalLButton);
    connect(&m_FootPedal,&FootPedal::RButtonClicked,this,&MotorThread::on_FootPedalRButton);
    connect(&m_FootPedal,&FootPedal::LPedalClicked,this,&MotorThread::on_LeftFootPedal);
    connect(&m_FootPedal,&FootPedal::RPedalClicked,this,&MotorThread::on_RightFootPedal);

}


void MotorThread::run()
{
    m_WatchdogTimer.start();
//    int dir = 1;

    // CKim -  Thread starts when GUI is created.
    // This thread generates 5 ms watchdog pulse that keeps the power to the EPOS
    // and Pump alive. Therefore, any action that takes longer than 5 ms for execution
    // (EPOS initialization, Oscillation mode)should be executed from the separate thread.
    // Otherwise, to the motor will cut
    while(true)
    {
        // ------------------------------------------------------------------ //
        // CKim - If the state is in bad state (communication error or emergency) (-5, -4, -3)
        // Pulse the watchdog and return
        if(m_CurrState < DEBRIDER_STATE_UNINIT)   // UNINIT = -2 state
        {
            // CKim - Except for when emergency occured and Emergency Window needs to be raised
            if(m_CurrState <= DEBRIDER_STATE_EPOS_ERROR  && m_GuiEmergencyMode == 0)
            {
                m_EposThread.Abort();
                m_GuiEmergencyMode = 1;
                // CKim - Close EPOS
                m_Motor.CloseAllDevice();
                m_Motor.DisableAllDevice();
                emit UpdateGUI(m_CurrState);
                std::cout<<"Something is wrong.\n";
                digitalWrite(VDD_RESET,LOW);
                delay(10);
                digitalWrite(VDD_RESET,HIGH);
                delay(10);
            }
            m_PrevState = m_CurrState;
            PulseWatchDog();
            continue;
        }
        // ------------------------------------------------------------------ //

        // ------------------------------------------------------------------ //
        // CKim - If the state is in normal state, but not initialized pulse watch dog and
        // return until both communication has been established
        if(m_CurrState < DEBRIDER_STATE_INIT)   // INIT = 0 state
        {
            m_PrevState = m_CurrState;
            // CKim - If not initialized start initializing in separate thread
            // If the initializing thread exits without error, slot will update the state
            if(m_CurrState == DEBRIDER_STATE_UNINIT)   // UNINIT = -2 state
            {
                m_CurrState = DEBRIDER_STATE_INITIALIZING;  // INITAILIZING = -1 state
                std::cout<<"Initializing Connections\n";
                m_FootPedal.start();
                m_EposThread.SetTransition(kInit);
                m_EposThread.start();

            }
            PulseWatchDog();
            continue;
        }
        // ------------------------------------------------------------------ //

        // ------------------------------------------------------------------ //
        // CKim - Code beyond here is executed only if the state is equal or greater than DEBRIDER_STATE_INIT
        // First check if no emrgency and communication is OK
        // 1. Read relay state
        if(digitalRead(EMERGENCY_RELAY_CONTROL)==LOW)
        {
            std::cout << "Relay disconnected " << std::endl;
            // ###EMERGENCY_MODE ACTIONS###
            m_Emergency = 1;
            m_CurrState = DEBRIDER_STATE_EMERGENCY;

            // CKim - Stop Pump Motors
            pwmWrite(SUCTION_MOTOR_PWM,0);
            m_EposThread.Abort();
            // CKim - Close EPOS
            m_Motor.CloseAllDevice();
            m_Motor.DisableAllDevice();
            m_GuiEmergencyMode = 1;
            emit UpdateGUI(m_CurrState);
            continue;
        }

        // 2. Check EPOS and SPI communication state
        if(m_FootPedal.GetSPIError() || !m_FootPedal.isRunning())
        {
            // ###SPI_ERROR ACTIONS###
            m_CurrState=DEBRIDER_STATE_SPI_ERROR;
            m_FootPedal.SetSPIError(true);
            emit UpdateGUI(m_CurrState);
            std::cout << "SPI CONNECTION ERROR " << std::endl;
            continue;
        }
        // ###EPOS_ERROR ACTIONS###
        if(m_Motor.EPOSGetError())
        {
            m_CurrState=DEBRIDER_STATE_EPOS_ERROR;
            emit UpdateGUI(m_CurrState);
            std::cout << "EPOS CONNECTION ERROR " << std::endl;
            continue;   //break;
        }
        // ------------------------------------------------------------------ //

        // ------------------------------------------------------------------ //
        // CKim - Process pedal inputs (R/L Button and R Pedal) that were updated from slots
        // This updates GUI which in turn change motion paramteters
        ProcessPedalButtons();

        // CKim - Read and process left pedal input.
        m_LeftPedalDepth = m_FootPedal.GetLeftPedalValue();

        // CKim - Based on left and right pedal inputs, system will be in
        // two different operating state (close blade or spinning blade)
        if(m_LeftPedalDepth < ADC_VALUE_THRESHOLD && m_DebriderTargetSpeed){
            m_LeftPedalDown = 1;
        }else{
            m_LeftPedalDown = 0;
        }

        if(m_GuiBtnCloseBlade)
        {
            m_GuiBtnCloseBlade = 0;
            if(!m_LeftPedalDown)    {   m_CloseBlade = 1;       }
            else                    {   m_CloseBlade = 0;       }
        }
        // ---------------------------------------------------------------------------------

        // --------------------------------------------------------------------------------- //
        // CKim - Left Pedal Pressed, state transition from ENABLED to RUNNING or OSC
        if(m_PrevState == DEBRIDER_STATE_ENABLED && m_LeftPedalDown == 1)
        {
           // std::cout<<"State transition from enabled to run/osc\n";
            if(m_Oscillate)
            {
                m_CurrState = DEBRIDER_STATE_OSC;
                m_EposThread.SetTransition(kRunOscillation);
                m_EposThread.SetOscillationVelocity(m_DebriderTargetSpeed*GEAR_RATIO);
                m_EposThread.start();
            }
            else
            {
                m_CurrState = DEBRIDER_STATE_RUNNING;
                m_Motor.EnableVelocityMode();
            }
            emit UpdateGUI(m_CurrState);
            m_PrevState = m_CurrState;
            PulseWatchDog();
            continue;
        }

        // CKim - Right Pedal Pressed, state transition from ENABLED to CLOSE
        // Will return to ENABLED through callback upon completion of closing
        if (m_PrevState == DEBRIDER_STATE_ENABLED && m_CloseBlade)
        {
            //std::cout<<"State transition from enabled to close\n";
            m_CurrState = DEBRIDER_STATE_CLOSE_BLADE;
            m_CloseBlade = 0;
            m_EposThread.SetTransition(kCloseBlade);
            m_EposThread.start();

            emit UpdateGUI(m_CurrState);
            m_PrevState = m_CurrState;
            PulseWatchDog();
            continue;
        }
        // --------------------------------------------------------------------------------- //

        // ---------------------------------------------------------------------------------
        // CKim - In Running mode
        if(m_CurrState == DEBRIDER_STATE_RUNNING)
        {
            // CKim - Change speed
            m_DebriderInstantSpeed=int(float(m_Motor.GetCurrentVelocity())/float(GEAR_RATIO));
            PulseWatchDog();
            CalculateDesiredVelocity(m_DebriderDesiredSpeed,m_LeftPedalDepth,m_DebriderTargetSpeed);
            //            std::cout << "Target velocity is : " << m_DebriderTargetSpeed << std::endl;
//            std::cout << "New Target Velocity is : "<< m_DebriderInstantSpeed << std::endl;
//            std::cout << "Analog Value is : " << m_FootPedal.GetLeftPedalValue() << std::endl;
            // Analog pedal value varies from 0-1023, we discard 23 and map analog value to from 0-1000.
            // Based on received analog value from pedal we adjust speed. 200 is when we take gear ratio in to account.
            m_Motor.MoveVelocity(m_DebriderDesiredSpeed);
            PulseWatchDog();
            emit UpdateGUI(m_CurrState);
        }
        // ---------------------------------------------------------------------------------

        // ---------------------------------------------------------------------------------
        // CKim - Left Pedal released
        // ###FROM RUNNING OR OSC MODE TO ENABLED TRANSITION###
        if(m_PrevState > DEBRIDER_STATE_ENABLED && m_LeftPedalDown == 0)
        {
            if(m_PrevState == DEBRIDER_STATE_RUNNING )
            {
                m_DebriderInstantSpeed=0;
                m_Motor.MoveVelocity(0);
                m_CurrState = DEBRIDER_STATE_ENABLED;
               // std::cout << "Returning to Enabled from running\n";
                emit UpdateGUI(m_CurrState);
            }

            if(m_PrevState == DEBRIDER_STATE_OSC)
            {
                m_EposThread.Abort();
                m_CurrState = DEBRIDER_STATE_ENABLED;
               // std::cout << "Returning to Enabled from oscillation\n";
                emit UpdateGUI(m_CurrState);
            }
        }

        // CKim - Blade closed
        if(m_PrevState == DEBRIDER_STATE_BLADE_CLOSED)
        {
            //std::cout << "Returning to Enabled from closed\n";
            m_CurrState = DEBRIDER_STATE_ENABLED;
            emit UpdateGUI(m_CurrState);
        }


        // ---------------------------------------------------------------------------------

        m_PrevState = m_CurrState;
        PulseWatchDog();

    }
}

// CKim - Initialization thread completed
void MotorThread::on_InitComplete(int state)
{
    if(state == DEBRIDER_STATE_ENABLED)
    {
        std::cout << "Initialization Complete\n";
        //m_currState = DEBRIDER_STATE_INIT;
        m_CurrState = DEBRIDER_STATE_ENABLED;
        emit UpdateGUI(m_CurrState);
    }else if (state==DEBRIDER_STATE_READY) {
        std::cout << "Waiting for Initialization Switch\n";
        m_CurrState = DEBRIDER_STATE_READY;
        emit UpdateGUI(m_CurrState);
    }

    else if (state==DEBRIDER_STATE_EPOS_ERROR)
    {
        std::cout << "Initialization error\n";
        m_CurrState = DEBRIDER_STATE_EPOS_ERROR;
        emit UpdateGUI(m_CurrState);
    }
    else if (state==DEBRIDER_STATE_EMERGENCY) {
        std::cout << "Initialization error\n";
        m_CurrState = DEBRIDER_STATE_EMERGENCY;
        emit UpdateGUI(m_CurrState);
    }else if (state==DEBRIDER_STATE_SPI_ERROR) {
        std::cout << "Initialization error\n";
        m_CurrState = DEBRIDER_STATE_EMERGENCY;
        emit UpdateGUI(m_CurrState);
    }else if (state==DEBRIDER_STATE_INITIALIZING) {
        std::cout << "Initialization error\n";
        m_CurrState = DEBRIDER_STATE_INITIALIZING;
        emit UpdateGUI(m_CurrState);
    }
}

void MotorThread::on_TransToOscComplete(int errcode)
{
    if(errcode == 0)
    {
        std::cout << "Oscillation mode ended\n";
        //m_currState = DEBRIDER_STATE_INIT;
        m_CurrState = DEBRIDER_STATE_ENABLED;
        emit UpdateGUI(m_CurrState);
    }
    else
    {
        std::cout << "Failed to start oscillation mode\n";
        m_CurrState = DEBRIDER_STATE_EMERGENCY;
        emit UpdateGUI(m_CurrState);
    }
}


void MotorThread::on_BladeClosed(int errcode)
{
    if(errcode == 0)
    {
        std::cout << "Blade closed\n";
        m_CurrState = DEBRIDER_STATE_BLADE_CLOSED;
        emit UpdateGUI(m_CurrState);
    }
    else
    {
        std::cout << "Blade closing error\n";
        m_CurrState = DEBRIDER_STATE_EMERGENCY;
        emit UpdateGUI(m_CurrState);
    }
}

void MotorThread::on_FootPedalLButton()
{
    // CKim - Left Button changes RPM to MAX
    m_LeftButtonClicked = 1;
    //std::cout<<"Left Click\n";
}

void MotorThread::on_FootPedalRButton()
{
    // CKim - Right button changes spin direction
    m_RightButtonClicked = 1;
    //std::cout<<"Right Click\n";
}

void MotorThread::on_RightFootPedal()
{
    //std::cout<<"Right Pedal Click\n";
    // CKim - Right Foot Pedal has effect of pressing "Close" button in GUI
    m_RightPedalClicked = 1;
}

void MotorThread::ReInitialize()
{
    std::cout << "Reinitializing\n";
    m_CurrState = DEBRIDER_STATE_UNINIT;
    m_FootPedal.requestInterruption();
    if(m_FootPedal.isRunning()){
        m_FootPedal.SetSPIError(false);
    }
    m_GuiEmergencyMode = 0;
}

void MotorThread::on_SPIStateChanged(int state)
{
    m_CurrState=state;
    emit(UpdateGUI(m_CurrState));
}

void MotorThread::ProcessPedalButtons()
{
    if(m_LeftButtonClicked || m_RightButtonClicked || m_RightPedalClicked || m_LeftPedalClicked){

        if(m_LeftButtonClicked){
            m_LeftButtonClicked = 0;
            if(m_CurrState!=DEBRIDER_STATE_ENABLED){
                m_GuiChangePresetRPM=0;
            }else{
                m_GuiChangePresetRPM=1;
            }
        }

        if(m_RightButtonClicked)
        {
            m_RightButtonClicked = 0;
            if(m_CurrState!=DEBRIDER_STATE_ENABLED){
                m_GuiBtnChangeDirection=0;
            }else{
                m_GuiBtnChangeDirection=1;
            }
        }

        if(m_LeftPedalClicked)
        {
            m_LeftPedalClicked = 0;
            if(m_CurrState!=DEBRIDER_STATE_ENABLED){
                m_GuiBtnCloseBlade=0;
            }
            else{
                m_GuiBtnCloseBlade=1;
            }
        }
        emit UpdateGUI(m_CurrState);
    }
}

void MotorThread::PulseWatchDog()
{
    // CKim - Pulse watchdog
    if(m_WatchdogTimer.nsecsElapsed()>=2500000)
    {
        if(m_WatchdogState)
        {
            digitalWrite(WATCHDOG_PIN,LOW);
            m_WatchdogTimer.restart();
            m_WatchdogState=false;
        }
        else
        {
            digitalWrite(WATCHDOG_PIN,HIGH);
            m_WatchdogTimer.restart();
            m_WatchdogState=true;
        }
    } // if watchDogTimer.nsecsElapsed

}

void MotorThread::CalculateDesiredVelocity(int& desired_vel, int pedal_depth, int target_vel)
{
    int pedal_val = 0;
    pedal_val = (pedal_depth-ADC_MIN_PEDAL_OUTPUT)*(ADC_MIN_OUTPUT-ADC_MAX_OUTPUT)/(ADC_MAX_OUTPUT-ADC_MIN_PEDAL_OUTPUT)+ADC_MAX_OUTPUT;
    desired_vel = static_cast<int>((static_cast<float>(target_vel)/static_cast<float>(ADC_MAX_OUTPUT))*pedal_val*GEAR_RATIO);
}

void MotorThread::on_LeftFootPedal()
{
    //std::cout<<"Right Pedal Click\n";
    // CKim - Right Foot Pedal has effect of pressing "Close" button in GUI
    m_LeftPedalClicked = 1;
}
