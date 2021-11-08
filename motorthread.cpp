#include <motorthread.h>

motorThread::motorThread()
{
    m_currState = DEBRIDER_STATE_UNINIT;
    m_prevState = DEBRIDER_STATE_UNINIT;
    m_DebriderTargetSpeed = 0;          // CKim - Debrider speed at maximum pedal press
    m_DebriderInstantSpeed=0;           // CKim - Current debrider speed.
    m_Oscillate  = 0;
    m_TargetPos  = 0;
    m_CloseBlade = 0;
    m_emergency  = 0;

    m_LeftButtonClicked =   m_RightButtonClicked = m_RightPedalClicked = 0;

    guiBtnCloseBlade=0;
    guiChangePresetRPM=0;
    guiBtnChangeDirection=0;
    guiEmergencyMode=0;
    watchDogState=true;

    m_eposThread.SetMotor(&m_Motor);
    m_eposThread.SetPedal(&m_FootPedal);
    connect(&m_eposThread,&EposThread::InitializationComplete,this,&motorThread::on_InitComplete);
    connect(&m_eposThread,&EposThread::OscillationComplete,this,&motorThread::on_TransToOscComplete);
    connect(&m_eposThread,&EposThread::CloseBladeComplete,this,&motorThread::on_BladeClosed);

    connect(&m_FootPedal,&FootPedal::LButtonClicked,this,&motorThread::on_FootPedalLButton);
    connect(&m_FootPedal,&FootPedal::RButtonClicked,this,&motorThread::on_FootPedalRButton);
    connect(&m_FootPedal,&FootPedal::RPedalClicked,this,&motorThread::on_RightFootPedal);

}


void motorThread::run()
{
    watchDogTimer.start();
    int dir = 1;

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
        on_off_timer.start();
        if(m_currState < DEBRIDER_STATE_UNINIT)   // UNINIT = -2 state
        {
            // CKim - Except for when emergency occured and Emergency Window needs to be raised
            if(m_currState <= DEBRIDER_STATE_EPOS_ERROR  && guiEmergencyMode == 0)
            {
                m_eposThread.Abort();
                guiEmergencyMode = 1;
                // CKim - Close EPOS
                m_Motor.CloseAllDevice();
                m_Motor.DisableAllDevice();
                emit UpdateGUI(m_currState);
            }
            m_prevState = m_currState;
            PulseWatchDog();
            continue;
        }
        // ------------------------------------------------------------------ //

        // ------------------------------------------------------------------ //
        // CKim - If the state is in normal state, but not initialized pulse watch dog and
        // return until both communication has been established
        if(m_currState < DEBRIDER_STATE_INIT)   // INIT = 0 state
        {
            m_prevState = m_currState;
            // CKim - If not initialized start initializing in separate thread
            // If the initializing thread exits without error, slot will update the state
            if(m_currState == DEBRIDER_STATE_UNINIT)   // UNINIT = -2 state
            {
                m_currState = DEBRIDER_STATE_INITIALIZING;  // INITAILIZING = -1 state
                std::cout<<"Initializing Connections\n";
                m_eposThread.SetTransition(1);
                m_eposThread.start();

                // CKim - Open SPI port
                m_FootPedal.start();
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
            m_emergency = 1;
            m_currState = DEBRIDER_STATE_EMERGENCY;

            // CKim - Stop Pump Motors
            pwmWrite(PUMP_HARDPWM,0);
            m_eposThread.Abort();
            // CKim - Close EPOS
            m_Motor.CloseAllDevice();
            m_Motor.DisableAllDevice();
            guiEmergencyMode = 1;
            emit UpdateGUI(m_currState);
            continue;
        }

        // 2. Check EPOS and SPI communication state
        if(m_FootPedal.GetSPIError() || !m_FootPedal.isRunning())
        {
            // ###SPI_ERROR ACTIONS###
            m_currState=DEBRIDER_STATE_SPI_ERROR;
            m_FootPedal.SetSPIError(true);
            emit UpdateGUI(m_currState);
            std::cout << "SPI CONNECTION ERROR " << std::endl;
            continue;
        }
        // ###EPOS_ERROR ACTIONS###
        if(m_Motor.EPOSGetError())
        {
            m_currState=DEBRIDER_STATE_EPOS_ERROR;
            emit UpdateGUI(m_currState);
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
        if(m_LeftPedalDepth > 23 && m_DebriderTargetSpeed)   {   m_LeftPedalDown = 1; }
        else                        {   m_LeftPedalDown = 0; }

        if(guiBtnCloseBlade)
        {
            guiBtnCloseBlade = 0;
            if(!m_LeftPedalDown)    {   m_CloseBlade = 1;       }
            else                    {   m_CloseBlade = 0;       }
        }
        // ---------------------------------------------------------------------------------

        // --------------------------------------------------------------------------------- //
        // CKim - Left Pedal Pressed, state transition from ENABLED to RUNNING or OSC
        if(m_prevState == DEBRIDER_STATE_ENABLED && m_LeftPedalDown == 1)
        {
           // std::cout<<"State transition from enabled to run/osc\n";
            if(m_Oscillate)
            {
                m_currState = DEBRIDER_STATE_OSC;
                m_eposThread.SetTransition(3);
                m_eposThread.setOscillationRPM(m_DebriderTargetSpeed*5);
                m_eposThread.start();
            }
            else
            {
                m_currState = DEBRIDER_STATE_RUNNING;
                testTimer.start();
                m_Motor.EnableVelocityMode();
                if (counter > 100 ) {
                     std::cout << "Enabling Velocity Mode Timer : "<<testTimer.nsecsElapsed() <<  std::endl;
                    counter=0;
                    testTimer.restart();
                }else {
                    testTimer.restart();
                }
            }
            emit UpdateGUI(m_currState);
            m_prevState = m_currState;
            PulseWatchDog();
            continue;
        }

        // CKim - Right Pedal Pressed, state transition from ENABLED to CLOSE
        // Will return to ENABLED through callback upon completion of closing
        if (m_prevState == DEBRIDER_STATE_ENABLED && m_CloseBlade)
        {
            //std::cout<<"State transition from enabled to close\n";
            m_currState = DEBRIDER_STATE_CLOSE_BLADE;
            m_CloseBlade = 0;
            m_eposThread.SetTransition(2);
            m_eposThread.start();

            emit UpdateGUI(m_currState);
            m_prevState = m_currState;
            PulseWatchDog();
            continue;
        }
        // --------------------------------------------------------------------------------- //

        // ---------------------------------------------------------------------------------
        // CKim - In Running mode
        if(m_currState == DEBRIDER_STATE_RUNNING)
        {
            // CKim - Change speed
            m_DebriderInstantSpeed=int(m_Motor.GetCurrentVelocity()/5);
            PulseWatchDog();
//            std::cout << "Target velocity is : " << m_DebriderTargetSpeed << std::endl;
//            std::cout << "New Target Velocity is : "<< m_DebriderInstantSpeed << std::endl;
//            std::cout << "Analog Value is : " << m_FootPedal.GetLeftPedalValue() << std::endl;
            // Analog pedal value varies from 0-1023, we discard 23 and map analog value to from 0-1000.
            // Based on received analog value from pedal we adjust speed. 200 is when we take gear ratio in to account.
            testTimer.restart();
            m_Motor.MoveVelocity((m_DebriderTargetSpeed/200)*(m_LeftPedalDepth-23));
            PulseWatchDog();
            if (counter > 100 ) {
                 std::cout << "Sending Target Velocity took : "<<testTimer.elapsed() << "ms" << std::endl;
                counter=0;
                testTimer.restart();
            }else {
                counter++;
                testTimer.restart();
            }

            emit UpdateGUI(m_currState);
        }
        // ---------------------------------------------------------------------------------

        // ---------------------------------------------------------------------------------
        // CKim - Left Pedal released
        // ###FROM RUNNING OR OSC MODE TO ENABLED TRANSITION###
        if(m_prevState > DEBRIDER_STATE_ENABLED && m_LeftPedalDown == 0)
        {
            if(m_prevState == DEBRIDER_STATE_RUNNING )
            {
                m_DebriderInstantSpeed=0;
                m_Motor.MoveVelocity(0);
                m_currState = DEBRIDER_STATE_ENABLED;
               // std::cout << "Returning to Enabled from running\n";
                emit UpdateGUI(m_currState);
            }

            if(m_prevState == DEBRIDER_STATE_OSC)
            {
                m_eposThread.Abort();
                m_currState = DEBRIDER_STATE_ENABLED;
               // std::cout << "Returning to Enabled from oscillation\n";
                emit UpdateGUI(m_currState);
            }
        }

        // CKim - Blade closed
        if(m_prevState == DEBRIDER_STATE_BLADE_CLOSED)
        {
            //std::cout << "Returning to Enabled from closed\n";
            m_currState = DEBRIDER_STATE_ENABLED;
            emit UpdateGUI(m_currState);
        }


        // ---------------------------------------------------------------------------------

        m_prevState = m_currState;
        PulseWatchDog();

    }
}


// CKim - Initialization thread completed
void motorThread::on_InitComplete(int errcode)
{
    if(errcode == 0)
    {
        std::cout << "Initialization Complete\n";
        //m_currState = DEBRIDER_STATE_INIT;
        m_currState = DEBRIDER_STATE_ENABLED;
        emit UpdateGUI(m_currState);
    }
    else
    {
        std::cout << "Initialization error\n";
        m_currState = DEBRIDER_STATE_EMERGENCY;
        emit UpdateGUI(m_currState);
    }
}

void motorThread::on_TransToOscComplete(int errcode)
{
    if(errcode == 0)
    {
        //std::cout << "Oscillation mode ended\n";
        //m_currState = DEBRIDER_STATE_INIT;
        m_currState = DEBRIDER_STATE_ENABLED;
        emit UpdateGUI(m_currState);
    }
    else
    {
        std::cout << "Failed to start oscillation mode\n";
        m_currState = DEBRIDER_STATE_EMERGENCY;
        emit UpdateGUI(m_currState);
    }
}


void motorThread::on_BladeClosed(int errcode)
{
    if(errcode == 0)
    {
        std::cout << "Blade closed\n";
        m_currState = DEBRIDER_STATE_BLADE_CLOSED;
        emit UpdateGUI(m_currState);
    }
    else
    {
        std::cout << "Blade closing error\n";
        m_currState = DEBRIDER_STATE_EMERGENCY;
        emit UpdateGUI(m_currState);
    }
}

void motorThread::on_FootPedalLButton()
{
    // CKim - Left Button changes RPM to MAX
    m_LeftButtonClicked = 1;
    //std::cout<<"Left Click\n";
}

void motorThread::on_FootPedalRButton()
{
    // CKim - Right button changes spin direction
    m_RightButtonClicked = 1;
    //std::cout<<"Right Click\n";
}

void motorThread::on_RightFootPedal()
{
    //std::cout<<"Right Pedal Click\n";
    // CKim - Right Foot Pedal has effect of pressing "Close" button in GUI
    m_RightPedalClicked = 1;
}

void motorThread::ReInitialize()
{
    std::cout << "Reinitializing\n";
    m_currState = DEBRIDER_STATE_UNINIT;
    guiEmergencyMode = 0;
}

void motorThread::ProcessPedalButtons()
{
    if(m_LeftButtonClicked || m_RightButtonClicked || m_RightPedalClicked){

        if(m_LeftButtonClicked){
            m_LeftButtonClicked = 0;
            if(m_currState!=DEBRIDER_STATE_ENABLED){
                guiChangePresetRPM=0;
            }else{
                guiChangePresetRPM=1;
            }
        }

        if(m_RightButtonClicked)
        {
            m_RightButtonClicked = 0;
            if(m_currState!=DEBRIDER_STATE_ENABLED){
                guiBtnChangeDirection=0;
            }else{
                guiBtnChangeDirection=1;
            }
        }

        if(m_RightPedalClicked)
        {
            m_RightPedalClicked = 0;
            if(m_currState!=DEBRIDER_STATE_ENABLED){
                guiBtnCloseBlade=0;
            }
            else{
                guiBtnCloseBlade=1;
            }
        }
        emit UpdateGUI(m_currState);
    }
}

void motorThread::PulseWatchDog()
{

    if(watchDogTimer.nsecsElapsed()>=5000000)
    {
        if(watchDogState)
        {
            digitalWrite(WATCHDOG_PIN,LOW);
            watchDogTimer.restart();
            watchDogState=false;
        }
        else
        {
            digitalWrite(WATCHDOG_PIN,HIGH);
            watchDogTimer.restart();
            watchDogState=true;
        }
    } // if watchDogTimer.nsecsElapsed

}
