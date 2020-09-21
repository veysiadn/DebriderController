#include <motorthread.h>
#include <epos4_can.h>
void motorThread::run()
{
    pinMode(WATCHDOG_PIN,OUTPUT);
    pinMode(EMERGENCY_RELAY_CONTROL,INPUT);
    delay(1000);
    initWatchDog();
    watchDogTimer.start();
    int dir = 1;
    while(true)
    {
        m_currState=DEBRIDER_STATE_INIT;
        if(digitalRead(EMERGENCY_RELAY_CONTROL)==LOW && !guiEmergencyMode)
        {
            serialArduino.arduinoSerialRunning=TRUE;
            if(!serialArduino.isRunning())
                serialArduino.start();
                m_Motor.ActiviateAllDevice();
                m_Motor.DisableAllDevice();
                m_Motor.ActiviateAllDevice();
            if(!serialArduino.getSerialError() && !m_Motor.EPOSGetError())
            {
                m_currState = DEBRIDER_STATE_ENABLED;
                m_running=true;
                std::cout<< " DEBRIDER STATE ENABLED " << std::endl;
                std::cout << "EPOS & SERIAL CONNECTION INITIALIZED..." << std::endl;
                emit UpdateGUI(m_currState);
               }
            else if (m_Motor.EPOSGetError())
            {
                std::cout << "EPOS CONNECTION ERROR " << std::endl;
                m_currState = DEBRIDER_STATE_EPOS_ERROR;
                emit UpdateGUI(DEBRIDER_STATE_EPOS_ERROR);
                m_running=false;
            }
            else if (serialArduino.getSerialError())
            {
                std::cout << "Serial CONNECTION ERROR : "
                             "Serial Connection couldn't initialized... " << std::endl;
                m_currState = DEBRIDER_STATE_SERIAL_ERROR;
                serialArduino.arduinoSerialRunning=0;
                m_running=false;
                emit UpdateGUI(m_currState);
            }
            else
            {
                std::cout << "Critical ERROR : Serial & "
                             "EPOS Couldn't be initialized... " << std::endl;
                m_currState = DEBRIDER_STATE_EMERGENCY;
                m_running=false;
                emit UpdateGUI(m_currState);
            }

            while(m_running)
            {
                // ###SERIAL_ERROR ACTIONS###
                if(serialArduino.getSerialError() || !serialArduino.isRunning())
                {
                    m_currState=DEBRIDER_STATE_SERIAL_ERROR;
                    serialArduino.setSerialError(true);
                    emit UpdateGUI(m_currState);
                    std::cout << "Serial CONNECTION ERROR " << std::endl;
                    m_running=false;
                    break;
                }
                // ###EPOS_ERROR ACTIONS###
                if(m_Motor.EPOSGetError())
                {
                    m_currState=DEBRIDER_STATE_EPOS_ERROR;
                    emit UpdateGUI(m_currState);
                    std::cout << "EPOS CONNECTION ERROR " << std::endl;
                    m_running=false;
                    break;
                }
                if(!firstSetup)
                {
                if(digitalRead(EMERGENCY_RELAY_CONTROL)==HIGH)
                                                        { m_emergency=1; }
                else                                    { m_emergency=0; }
                }
                else
                {
                    initWatchDog();
                }

                if((serialArduino.pedalBtnCloseBlade && serialArduino.pedalAnalogBLDCval < 23)
                        || (guiBtnCloseBlade && serialArduino.pedalAnalogBLDCval < 23))
                                                       {  m_CloseBlade = 1; }
                else
                                                       {  m_CloseBlade = 0; }

                if(serialArduino.pedalAnalogBLDCval > 23)
                                                       {   m_LeftPedalDown = 1; }
                else
                                                       {   m_LeftPedalDown = 0; }

                setBtnMaxRPMGUI();
                setBtnChangeDirectionGUI();

                // ###EMERGENCY_MODE ACTIONS###
                if(m_emergency)
                {
                    m_currState = DEBRIDER_STATE_EMERGENCY;
                    m_running=false;
                    digitalWrite(PUMP_ENABLE,0);
                    pwmWrite(PUMP_HARDPWM,0);
                    m_Motor.CloseAllDevice();
                    m_Motor.DisableAllDevice();
                    emit UpdateGUI(m_currState);
                    break;
                }

                // ###CLOSE_BLADE MODE ACTIONS###

                if (m_currState == DEBRIDER_STATE_ENABLED
                        && m_CloseBlade && m_prevState!=DEBRIDER_STATE_CLOSE_BLADE)
                {
                    m_currState = DEBRIDER_STATE_CLOSE_BLADE;
                    m_Motor.EnablePositionMode();
                    m_TargetPos = m_Motor.getCloseBladePosition();
                    m_Motor.MovePosition(m_TargetPos);
                    m_CloseBlade=0;
                    guiBtnCloseBlade=0;
                    emit UpdateGUI(m_currState);
                }

                // ###ENABLED AND RUNNING MODE  ACTIONS###
                if((m_prevState == DEBRIDER_STATE_ENABLED || m_prevState==DEBRIDER_STATE_RUNNING )
                        && m_LeftPedalDown == 1)
                {
                    // ###OSC_MODE ACTIONS###
                    if(m_Oscillate)
                    {
                        m_currState = DEBRIDER_STATE_OSC;
                        emit UpdateGUI(m_currState);
                        // Start oscillation.
                        m_Motor.EnablePositionMode();
                        m_Motor.SetOscMode(15000);
                        dir = 1;
                    }
                    // ###RUNNING MODE ACTIONS###
                    else
                    {
                        m_currState = DEBRIDER_STATE_RUNNING;
                        m_Motor.EnableVelocityMode();
                        m_DebriderInstantSpeed=(m_DebriderTargetSpeed/1000)*
                                (serialArduino.pedalAnalogBLDCval-23);

                        //  std::cout << "Target velocity is : " << m_DebriderTargetSpeed << std::endl;
                        //  std::cout << "New Target Velocity is : "<< m_DebriderInstantSpeed << std::endl;
                        //  std::cout << "Analog Value is : " << serialArduino.pedalAnalogBLDCval << std::endl;
                        m_Motor.MoveVelocity(m_DebriderInstantSpeed);
                        emit UpdateGUI(m_currState);
                    }
                }


                // CKim - Pedal released
                // ###FROM RUNNING OR OSC MODE TO ENABLED TRANSITION###
                if(m_prevState > DEBRIDER_STATE_ENABLED && m_LeftPedalDown == 0)
                {
                    if(m_prevState == DEBRIDER_STATE_CLOSE_BLADE && !m_CloseBlade )
                    {
                        m_Motor.MoveVelocity(0);
                    }
                    //TransitToEnabled();
                    if(m_prevState == DEBRIDER_STATE_RUNNING )
                    {
                        m_DebriderInstantSpeed=0;
                        m_Motor.MoveVelocity(0);
                    }

                    if(m_prevState == DEBRIDER_STATE_OSC)
                    {
                        m_Motor.StopMotion();   // Stop oscillate
                    }
                    m_currState = DEBRIDER_STATE_ENABLED;
                    emit UpdateGUI(m_currState);
                }

                if((m_currState == DEBRIDER_STATE_OSC) && m_Oscillate
                        && serialArduino.pedalAnalogBLDCval > 23)
                {

                    m_Motor.WaitForMotion();
                    m_Motor.RunOscMode(dir);
                    dir *= -1;
                }

                m_prevState = m_currState;
                if(firstSetup) { firstSetup=false; }
                if(watchDogTimer.nsecsElapsed()>=50000000)
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
              } // while(m_running)
                    m_Motor.CloseAllDevice();
                    m_Motor.DisableAllDevice();
                    serialArduino.closeSerialPort();
        } //if(EMERGENCY_BUTTON==LOW || !guiEmergencyMode)
      } // while(true)
} // void run()
void motorThread::setBtnMaxRPMGUI()
{
    // ######### HARDWARE MAXRPM BUTTON CLICKED SETTINGS START  ###########
    if(serialArduino.pedalBtnMaxRPM != pedalBtnMaxRPM_LastState)
    {
       // std::cout << "last state : "<< btn_MAXRPM_LastState <<
         //            " frst state : " << arduino.btn_MAXRPM << std::endl;
        if(m_currState!=DEBRIDER_STATE_ENABLED)
        {
            guiBtnMaxRPM=0;
        }
        else
        {
            guiBtnMaxRPM=1;
        }
        pedalBtnMaxRPM_LastState = serialArduino.pedalBtnMaxRPM;
        emit UpdateGUI(m_currState);
    }
    // ######### HARDWARE MAXRPM BUTTON CLICKED SETTINGS FINISHED  ###########
}

void motorThread::setBtnChangeDirectionGUI()
{
    // ######### HARDWARE CHANGE DIRECTION BUTTON CLICKED SETTINGS START  ###########

    if(serialArduino.pedalBtnChangeDirection!=pedalBtnChangeDir_LastState)
    {
        if(m_currState!=DEBRIDER_STATE_ENABLED)
        {
            guiBtnChangeDirection=0;
        }
        else
        {
            guiBtnChangeDirection=1;
        }
        pedalBtnChangeDir_LastState=serialArduino.pedalBtnChangeDirection;
        emit UpdateGUI(m_currState);
    }
    // ######### HARDWARE CHANGE DIRECTION BUTTON CLICKED SETTINGS FINISH  ###########

}
void motorThread::initWatchDog()
{
    for (int i = 0; i < 3 ;i++)
    {
        digitalWrite(WATCHDOG_PIN,LOW);
        delay(50);
        digitalWrite(WATCHDOG_PIN,HIGH);
        delay(50);
    }
}
