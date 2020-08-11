#include <motorthread.h>
#include <epos4_can.h>
void motorThread::run()
{
    pinMode(WATCHDOG_PIN,OUTPUT);
    pinMode(EMERGENCY_BUTTON,INPUT);
    int dir = 1;
    while(true)
    {
        if(digitalRead(EMERGENCY_BUTTON)==LOW && !guiEmergencyMode){
        arduinoNano.serialArduinoRunning=1  ;
        if(!arduinoNano.isRunning())
            arduinoNano.start();  // If serial thread isn't running, start thread.
        //motorThread::msleep(3000);
        m_Motor.CloseAllDevice();
        m_Motor.ActiviateAllDevice();
        if(!arduinoNano.getSerialError() && !m_Motor.EPOSGetError())
        {
                m_currState = DEBRIDER_STATE_ENABLED;
                m_running=true;
                std::cout<< " DEBRIDER STATE ENABLED " << std::endl;
                std::cout << "EPOS & SERIAL CONNECTION INITIALIZED..." << std::endl;
                emit UpdateGUI(m_currState);
                watchDogTimer.start();
        }
        else if (m_Motor.EPOSGetError())
        {
            std::cout << "EPOS CONNECTION ERROR " << std::endl;
            m_currState = DEBRIDER_STATE_EPOS_ERROR;
            emit UpdateGUI(DEBRIDER_STATE_EPOS_ERROR);
            m_running=false;
        }
        else if (arduinoNano.getSerialError())
        {
            std::cout << "Serial CONNECTION ERROR : Serial Connection couldn't initialized... " << std::endl;
            m_currState = DEBRIDER_STATE_SERIAL_ERROR;
            arduinoNano.serialArduinoRunning=0;
            m_running=false;
            emit UpdateGUI(m_currState);
        }
        else
        {
            std::cout << "Critical ERROR : Serial & EPOS Couldn't be initialized... " << std::endl;
            m_currState = DEBRIDER_STATE_EMERGENCY;
            m_running=false;
            emit UpdateGUI(m_currState);
        }

        while(m_running)
        {
            if(arduinoNano.getSerialError() || !arduinoNano.isRunning())
            {
                m_currState=DEBRIDER_STATE_SERIAL_ERROR;
                arduinoNano.setSerialError(true);
                emit UpdateGUI(m_currState);
                std::cout << "Serial CONNECTION ERROR " << std::endl;
                m_running=false;
                break;
            }
            if(m_Motor.EPOSGetError())
            {
                m_currState=DEBRIDER_STATE_EPOS_ERROR;
                emit UpdateGUI(m_currState);
                std::cout << "EPOS CONNECTION ERROR " << std::endl;
                m_running=false;
                break;
            }

            if(digitalRead(EMERGENCY_BUTTON)==HIGH)
                                                    { m_emergency=1; }
            else                                    { m_emergency=0; }

            if((arduinoNano.pedalBtnCloseBlade && arduinoNano.pedalAnalogBLDCval < 23)
                    || (guiBtnCloseBlade && arduinoNano.pedalAnalogBLDCval < 23))
                                                   {  m_CloseBlade = 1; }
            else                                   {  m_CloseBlade = 0; }

            if(arduinoNano.pedalAnalogBLDCval > 23)
                                                   {   m_LeftPedalDown = 1; }
            else                                   {   m_LeftPedalDown = 0; }

            setBtnMaxRPM();
            setBtnChangeDirection();
            if(m_emergency && m_prevState != DEBRIDER_STATE_EMERGENCY)
            {
                m_currState = DEBRIDER_STATE_EMERGENCY;         // VysADN Emergency_Mode function parameters
                m_running=false;
                digitalWrite(PUMP_ENABLE,0);
                pwmWrite(PUMP_HARDPWM,0);
                m_Motor.CloseAllDevice();
                m_Motor.DisableAllDevice();
                emit UpdateGUI(m_currState);
                break;
            }
            //VYSADN EmergencyMode Finish

            if (m_currState == DEBRIDER_STATE_ENABLED
                    && m_CloseBlade && m_prevState!=DEBRIDER_STATE_CLOSE_BLADE)
            {
                m_currState = DEBRIDER_STATE_CLOSE_BLADE; // VysADN CloseBlade function parameters
                m_Motor.EnablePositionMode();
                m_TargetPos = m_Motor.CloseBlade();
                m_Motor.MovePosition(m_TargetPos);
                m_CloseBlade=0;
                guiBtnCloseBlade=0;
                emit UpdateGUI(m_currState);
            }
            //VYSADN DEBRIDER CLOSE BLADE FINISH

            // CKim - Pedal pushed
            if((m_prevState == DEBRIDER_STATE_ENABLED || m_prevState==DEBRIDER_STATE_RUNNING )
                    && m_LeftPedalDown == 1)
            {
                //TransitToRunning();
                if(m_Oscillate)
                {
                    m_currState = DEBRIDER_STATE_OSC;
                    emit UpdateGUI(m_currState);
                    // Start oscillation.
                    m_Motor.EnablePositionMode();
                    m_Motor.SetOscMode(15000);
                    dir = 1;
                }

                else
                {
                    m_currState = DEBRIDER_STATE_RUNNING;
                    m_Motor.EnableVelocityMode();
                    m_instantSpeed=(m_DebriderTargetSpeed/1000)*(arduinoNano.pedalAnalogBLDCval-23);

    //                std::cout << "Target velocity is : " << m_TargetVel << std::endl;
    //                std::cout << "New Target Velocity is : "<< m_NewTargetVel << std::endl;
    //                std::cout << "Analog Value is : " << m_Motor.Get_Analog_Pedal_Val << std::endl;
                    m_Motor.MoveVelocity(m_instantSpeed);
                    emit UpdateGUI(m_currState);
                }
            }


            // CKim - Pedal released
            if(m_prevState > DEBRIDER_STATE_ENABLED && m_LeftPedalDown == 0)
            {
                if(m_prevState == DEBRIDER_STATE_CLOSE_BLADE && !m_CloseBlade )
                {
                    m_currState = DEBRIDER_STATE_ENABLED;
                    emit UpdateGUI(m_currState);
                    m_Motor.MoveVelocity(0);
                }
                //TransitToEnabled();
                if(m_prevState == DEBRIDER_STATE_RUNNING )
                {
                    m_instantSpeed=0;
                    m_currState = DEBRIDER_STATE_ENABLED;
                    emit UpdateGUI(m_currState);
                    m_Motor.MoveVelocity(0);
                }

                if(m_prevState == DEBRIDER_STATE_OSC)
                {
                    m_currState = DEBRIDER_STATE_ENABLED;
                    m_Motor.StopMotion();   // Stop oscillate
                    emit UpdateGUI(m_currState);
                }
            }

            if((m_currState == DEBRIDER_STATE_OSC) && m_Oscillate && arduinoNano.pedalAnalogBLDCval > 23)
            {

                m_Motor.WaitForMotion();
                m_Motor.RunOscMode(dir);
                dir *= -1;
            }

            m_prevState = m_currState;
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
                m_Motor.DisableAllDevice();
                arduinoNano.closeSerialPort();
        } //if(EMERGENCY_BUTTON==LOW || !guiEmergencyMode)
      } // while(true)
} // void run()
void motorThread::setBtnMaxRPM()
{
    // ######### HARDWARE MAXRPM BUTTON CLICKED SETTINGS START  ###########
    if(arduinoNano.pedalBtnMaxRPM != pedalBtnMaxRPM_LastState)
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
        pedalBtnMaxRPM_LastState = arduinoNano.pedalBtnMaxRPM;
        emit UpdateGUI(m_currState);
    }
    // ######### HARDWARE MAXRPM BUTTON CLICKED SETTINGS FINISHED  ###########
}

void motorThread::setBtnChangeDirection()
{
    // ######### HARDWARE CHANGE DIRECTION BUTTON CLICKED SETTINGS START  ###########

    if(arduinoNano.pedalBtnChangeDirection!=pedalBtnChangeDir_LastState)
    {
        if(m_currState!=DEBRIDER_STATE_ENABLED)
        {
            guiBtnChangeDirection=0;
        }
        else
        {
            guiBtnChangeDirection=1;
        }
        pedalBtnChangeDir_LastState=arduinoNano.pedalBtnChangeDirection;
        emit UpdateGUI(m_currState);
    }
    // ######### HARDWARE CHANGE DIRECTION BUTTON CLICKED SETTINGS FINISH  ###########

}
