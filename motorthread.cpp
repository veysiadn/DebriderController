#include <motorthread.h>
#include <epos4_can.h>
void motorThread::run()
{
    pinMode(WATCHDOG_PIN,OUTPUT);
    pinMode(EMERGENCY_BUTTON,INPUT);
    int dir = 1;
while(true){
    if(digitalRead(EMERGENCY_BUTTON)==LOW){
    m_Motor.CloseAllDevice();
    m_Motor.ActiviateAllDevice();

    if(!arduino.isRunning()) arduino.start();  // If serial thread isn't running start thread.

    if (!m_Motor.EPOSGetError())
    {
        std::cout << "EPOS CONNECTION SUCCESFULL" << std::endl;
    }
    else
    {
        std::cout << "EPOS CONNECTION ERROR " << std::endl;
        m_currState = DEBRIDER_STATE_EPOS_ERROR;
        emit UpdateGUI(DEBRIDER_STATE_EPOS_ERROR);
        m_running=false;
    }

    if (!arduino.SerialGetError())
    {
        std::cout << "Serial Connection Initialized..." << std::endl;
        arduino.SerialSetError(false);
    }
    else
    {
        std::cout << "Serial CONNECTION ERROR : Serial Connection couldn't initialized... " << std::endl;
        m_currState = DEBRIDER_STATE_SERIAL_ERROR;
        m_running=false;
        arduino.SerialSetError(true);
        emit UpdateGUI(m_currState);
    }

    if(!arduino.SerialGetError() && !m_Motor.EPOSGetError())
    {
        if(m_currState!=DEBRIDER_STATE_EMERGENCY || !m_emergencyMainWindow)
        {
            m_currState = DEBRIDER_STATE_ENABLED;
            m_running=true;
            std::cout<< " DEBRIDER STATE ENABLED " << std::endl;
            timer.start();
        }
    }
    while(m_running)
    {
        if(arduino.SerialGetError() || !arduino.isRunning())
        {
            m_currState=DEBRIDER_STATE_SERIAL_ERROR;
            arduino.SerialSetError(true);
            std::cout << "Serial CONNECTION ERROR " << std::endl;
            break;
        }
        if(m_Motor.EPOSGetError())
        {
            m_currState=DEBRIDER_STATE_EPOS_ERROR;
            emit UpdateGUI(m_currState);
            std::cout << "EPOS CONNECTION ERROR " << std::endl;
            break;
        }

        // VYSADN EmergencyButton Start
        if(digitalRead(EMERGENCY_BUTTON)==HIGH || m_emergencyMainWindow)
                                                { m_emergency=1; }
        else                                    { m_emergency=0; }

        if((arduino.btn_CloseBlade && arduino.Get_Analog_Pedal_Val < 10) || m_closeBladeWindow)
                                               {  m_CloseBlade = 1; }
        else                                   {  m_CloseBlade = 0; }

        if(arduino.Get_Analog_Pedal_Val > 23)
                                               {   m_LeftPedalDown = 1; }
        else                                   {   m_LeftPedalDown = 0; }

        // ######### HARDWARE MAXRPM BUTTON CLICKED SETTINGS START  ###########
        if(arduino.btn_MAXRPM != btn_MAXRPM_LastState)
        {
           // std::cout << "last state : "<< btn_MAXRPM_LastState <<
             //            " frst state : " << arduino.btn_MAXRPM << std::endl;
            if(m_currState!=DEBRIDER_STATE_ENABLED)
            {
                btn_MAXRPM_GUI=0;
            }
            else
            {
                btn_MAXRPM_GUI=1;
            }
            btn_MAXRPM_LastState = arduino.btn_MAXRPM;
            emit UpdateGUI(m_currState);
        }
        // ######### HARDWARE MAXRPM BUTTON CLICKED SETTINGS FINISHED  ###########

        // ######### HARDWARE CHANGE DIRECTION BUTTON CLICKED SETTINGS START  ###########

        if(arduino.btn_ChangeDirection!=btn_ChangeDirLastState)
        {
            if(m_currState!=DEBRIDER_STATE_ENABLED)
            {
                btn_ChangeDir_GUI=0;
            }
            else
            {
                btn_ChangeDir_GUI=1;
            }
            btn_ChangeDirLastState=arduino.btn_ChangeDirection;
            emit UpdateGUI(m_currState);
        }
        // ######### HARDWARE CHANGE DIRECTION BUTTON CLICKED SETTINGS FINISH  ###########


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
                && m_CloseBlade && m_prevState!=DEBRIDER_STATE_CLOSE_BLADES)
        {
            m_currState = DEBRIDER_STATE_CLOSE_BLADES; // VysADN CloseBlade function parameters
            m_Motor.EnablePositionMode();
            m_TargetPos = m_Motor.CloseBlade();
            m_Motor.MovePosition(m_TargetPos);
            m_CloseBlade=0;
            m_closeBladeWindow=0;
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
                m_NewTargetVel=(m_TargetVel/1000)*(arduino.Get_Analog_Pedal_Val-23);

//                std::cout << "Target velocity is : " << m_TargetVel << std::endl;
//                std::cout << "New Target Velocity is : "<< m_NewTargetVel << std::endl;
//                std::cout << "Analog Value is : " << m_Motor.Get_Analog_Pedal_Val << std::endl;
                m_Motor.MoveVelocity(m_NewTargetVel);
                emit UpdateGUI(m_currState);
            }
        }


        // CKim - Pedal released
        if(m_prevState > DEBRIDER_STATE_ENABLED && m_LeftPedalDown == 0)
        {
            if(m_prevState == DEBRIDER_STATE_CLOSE_BLADES && !m_CloseBlade )
            {
                m_currState = DEBRIDER_STATE_ENABLED;
                emit UpdateGUI(m_currState);
                m_Motor.MoveVelocity(0);
            }
            //TransitToEnabled();
            if(m_prevState == DEBRIDER_STATE_RUNNING )
            {
                m_NewTargetVel=0;
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

        if((m_currState == DEBRIDER_STATE_OSC) && m_Oscillate && arduino.Get_Analog_Pedal_Val > 23)
        {

            m_Motor.WaitForMotion();
            m_Motor.RunOscMode(dir);
            dir *= -1;
        }

        m_prevState = m_currState;
        if(timer.nsecsElapsed()>=50000000){
            if(WD_State){
            digitalWrite(WATCHDOG_PIN,LOW);
            timer.restart();
            WD_State=false;
                        }else   {
                                digitalWrite(WATCHDOG_PIN,HIGH);
                                timer.restart();
                                WD_State=true;
                                    }
                                 }
                }

   m_Motor.DisableAllDevice();
        }
    else if(m_currState!=DEBRIDER_STATE_EMERGENCY)
    {
        m_currState=DEBRIDER_STATE_EMERGENCY;
        emit UpdateGUI(m_currState);
    }
    }
}
