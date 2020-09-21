#include "epos4_can.h"


MaxonMotor::MaxonMotor()
{
    m_errorFlag = 0;

    PortName_MCP = "CAN0";

    m_Node_ID_MCP = 1;

    ErrorCode = 0x00;

    m_Mode = 0;

    std::cout<< "Instance initialized...\n ";
}


void MaxonMotor::CloseDevice(void *keyHandle_)
{
    unsigned int error_code = 0;

    std::cout<<"Closing Device!"<<std::endl;

    if(keyHandle_ != 0)
        VCS_CloseDevice(keyHandle_, &error_code);

    VCS_CloseAllDevices(&error_code);
}


void MaxonMotor::EnableDevice(void *keyHandle_, unsigned short Node_ID)
{
    unsigned int error_code = 0;

    int IsFault = FALSE;

    // 현재 State을 불러온다.
    if( VCS_GetFaultState(keyHandle_, Node_ID, &IsFault, &error_code) )
    {
        if( IsFault && !VCS_ClearFault(keyHandle_, Node_ID, &error_code) )
        {
            std::cout << "Clear Fault Failed!, error_code = " << error_code << std::endl;
            m_errorFlag = 1;
            return;
        }

        int IsEnabled = FALSE;

        // Device가 Enabled 되어 있는지 Check
        if( VCS_GetEnableState(keyHandle_, Node_ID, &IsEnabled, &error_code) )
        {
            if (!IsEnabled && !VCS_SetEnableState(keyHandle_, Node_ID, &error_code)) 
            {
                std::cout << "Set Enable State Failed!, error_code = " << error_code << std::endl;
                m_errorFlag = 1;
            }
            else
            {
                std::cout << "Set Enable State Succeeded!" << std::endl;
                m_errorFlag = 0;
            }
        }
    }
    else
    {
        std::cout << "Get fault state failed!, error_code =" << error_code << std::endl;
        m_errorFlag = 1;
    }
}


void MaxonMotor::EnableVelocityMode()
{
    //if(m_Mode == OMD_PROFILE_VELOCITY_MODE)     return;

    unsigned int error_code = 0;

    if (!VCS_ActivateProfileVelocityMode(m_keyHandle_MCP, m_Node_ID_MCP, &error_code))
    {
        std::cout << "Activate Profile Velocity Mode Failed!" << std::endl;
        m_errorFlag = 1;
        return;
    }

    m_Mode = OMD_PROFILE_VELOCITY_MODE;

    unsigned int ProfileAcceleration = 50000;
    unsigned int ProfileDeceleration = 50000;

    if (!VCS_SetVelocityProfile(m_keyHandle_MCP, m_Node_ID_MCP, ProfileAcceleration, ProfileDeceleration, &error_code))
    {
        std::cout << "VCS_SetVelocityProfile Failed!, error_code = " << error_code << std::endl;
        m_errorFlag = 1;
    }
}

void MaxonMotor::EnablePositionMode()
{
    if(m_Mode == OMD_PROFILE_POSITION_MODE)     return;

    unsigned int error_code = 0;

    // Profile Position Mode 활성화
    if( !VCS_ActivateProfilePositionMode(m_keyHandle_MCP, m_Node_ID_MCP, &error_code) )
    {
        std::cout << "Activate Profile Position Mode Failed!" << std::endl;
        m_errorFlag = 1;
        return;
    }

    m_Mode = OMD_PROFILE_POSITION_MODE;

    // profile 값 설정 : 속도, 가감속 값
    unsigned int ProfileVelocity = 15000;
    unsigned int ProfileAcceleration = 50000;
    unsigned int ProfileDeceleration = 50000;

    // 설정한 위치값으로 이동시켜주는 VCS 함수
    if (!VCS_SetPositionProfile(m_keyHandle_MCP, m_Node_ID_MCP, ProfileVelocity,
        ProfileAcceleration, ProfileDeceleration, &error_code)) {
        std::cout << "VCS_SetPositionProfile Failed!, error_code = " << error_code << std::endl;
        m_errorFlag = 1;
    }
}

void MaxonMotor::SetOscMode(int Amp)
{
    m_OscAmp = Amp;

    unsigned int error_code = 0;

    if (!VCS_GetPositionIs(m_keyHandle_MCP, m_Node_ID_MCP, &m_OscOffset, &error_code))
    {
        std::cout << " Error while getting current position , error_code = " << error_code << std::endl;
        m_errorFlag = 1;
    }
}

void MaxonMotor::RunOscMode(int dir)
{
    if (m_Mode != OMD_PROFILE_POSITION_MODE)     return;

    unsigned int error_code = 0;
    int Absolute = TRUE;
    int Immediately = TRUE;

    long tgtPos;
    if (dir > 0) { tgtPos = m_OscOffset + m_OscAmp; }
    if (dir < 0) { tgtPos = m_OscOffset - m_OscAmp; }
    if (dir == 0) { tgtPos = m_OscOffset; }

    if (!VCS_MoveToPosition(m_keyHandle_MCP, m_Node_ID_MCP, tgtPos, Absolute,Immediately, &error_code))
    {
        std::cout << "Move To Position Failed!, error_code = " << error_code << std::endl;
        m_errorFlag = 1;
    }
}

void MaxonMotor::DisableDevice(void *keyHandle_, unsigned short Node_ID)
{
    unsigned int error_code = 0;
    int IsFault = FALSE;

    if( VCS_GetFaultState(keyHandle_, Node_ID, &IsFault, &error_code) )
    {
        if( IsFault && !VCS_ClearFault(keyHandle_, Node_ID, &error_code) )
        {
            std::cout << "Clear Fault Failed!, error_code = "<< error_code << std::endl;
            m_errorFlag = 1;
            return;
        }

        int IsEnabled = FALSE;

        if( VCS_GetEnableState(keyHandle_, Node_ID, &IsEnabled, &error_code) )
        {
            if (IsEnabled && !VCS_SetDisableState(keyHandle_, Node_ID, &error_code)) 
            {
                std::cout << "Set Disable state failed!, error_code = " << error_code << std::endl;
                m_errorFlag = 1;
            }

            else
            {
               std::cout << "Set Disable State Succeeded!" << std::endl;
                 digitalWrite(PUMP_ENABLE,0);
                 pwmWrite(PUMP_HARDPWM,0);
                 m_errorFlag = 0;
            }
        }
    }
    else
    {
        std::cout<<"Get Fault State Failed!, error_code=" << error_code << std::endl;
        m_errorFlag = 1;
    }

}

void MaxonMotor::Move(void *keyHandle_, long target_velocity, unsigned short Node_ID)
{
    unsigned int error_code = 0;

    if(!VCS_MoveWithVelocity(keyHandle_, Node_ID, target_velocity, &error_code))
    {
        std::cout << "Move With Velocity failed!, error_code = " << error_code << std::endl;
        m_errorFlag = 1;
    }
}

void MaxonMotor::MoveVelocity(const long& target_velocity)
{
    if(m_Mode != OMD_PROFILE_VELOCITY_MODE)     return;

    unsigned int error_code = 0;
    if (!VCS_MoveWithVelocity(m_keyHandle_MCP, m_Node_ID_MCP, target_velocity, &error_code)) 
    {
        std::cout << "Move With Velocity failed!, error_code = " << error_code << std::endl;
        m_errorFlag = 1;
    }
}

void MaxonMotor::MovePosition(const long& target_pos)
{
    if (m_Mode != OMD_PROFILE_POSITION_MODE)     return;

    unsigned int error_code = 0;
    int Absolute = TRUE;
    int Immediately = TRUE;

    if (!VCS_MoveToPosition(m_keyHandle_MCP, m_Node_ID_MCP, target_pos, Absolute, Immediately, &error_code))
    {
        std::cout << "Move To Position Failed!, error_code = " << error_code << std::endl;
        m_errorFlag = 1;
    }
}

void MaxonMotor::StopMotion()
{
    unsigned int error_code = 0;
    if(!VCS_HaltPositionMovement(m_keyHandle_MCP, m_Node_ID_MCP, &error_code))
    {
        std::cout << "StopMotion Failed!, error_code = " << error_code << std::endl;
        m_errorFlag = 1;
    }
}

void MaxonMotor::WaitForMotion()
{
    unsigned int error_code = 0;

    // in ms
    if(!VCS_WaitForTargetReached(m_keyHandle_MCP, m_Node_ID_MCP,1000,&error_code))
    {

        std::cout << "Waiting Failed!, error_code = " << error_code << std::endl;
        m_errorFlag = 1;
    }

}

void* MaxonMotor::ActivateDevice(char *PortName, unsigned short Node_ID)
{
    char DeviceName[]           = "EPOS4";          // Device 이름
    char ProtocolStackName[]    = "CANopen";        // Protocol 이름
    char InterfaceName[]        = "CAN_mcp251x 0";  // Interface 이름 (PiCAN2 Board Chip 이름)

    unsigned int error_code = 0x00;
    unsigned long timeout_ = 10;                    // Port를 여는데 경과되는 시간
    unsigned long baudrate_ = 1000000;              // 통신 속도
    void *keyHandle_;

    // OpenDevice 함수 반환(HANDLE) 값
    keyHandle_ = VCS_OpenDevice(DeviceName, ProtocolStackName, InterfaceName, PortName, &error_code);

    // OpenDevice 실패
    if( keyHandle_ == 0 )
    {
        std::cout<<"Open Device Failure, error_code = "<< error_code << std::endl;
        m_errorFlag = 1;
        exit(0);
    }

    else
    {
        std::cout<<"Open Device Success!" << std::endl;
        m_errorFlag = 0;
    }

    // 지정한 Protocol 설정
    if(!VCS_SetProtocolStackSettings(keyHandle_, baudrate_, timeout_, &error_code) )
    {
        std::cout<<"Set Protocol Stack Settings Failed!, error_code = " << error_code << std::endl;
        m_errorFlag = 1;
        CloseDevice(keyHandle_);
        exit(0);
    }

    // Fault & State 함수 실행
    delay(100);
    EnableDevice(keyHandle_, Node_ID);

    return keyHandle_;
}

void MaxonMotor::ActiviateAllDevice()
{
    m_keyHandle_MCP = ActivateDevice(PortName_MCP, m_Node_ID_MCP);
}

void MaxonMotor::DisableAllDevice()
{
    DisableDevice(m_keyHandle_MCP, m_Node_ID_MCP);
}

void MaxonMotor::CloseAllDevice()
{
    CloseDevice(m_keyHandle_MCP);
}


void MaxonMotor::GetCurrentPosition(void *keyHandle_, int& current_position, unsigned short Node_ID)
{
    unsigned int error_code = 0;

    if(!VCS_GetPositionIs(keyHandle_, Node_ID, &current_position, &error_code))
    {
        std::cout << " Error while getting current position , error_code = "<< error_code << std::endl;
        m_errorFlag = 1;
    }
}

void MaxonMotor::GetCurrentPositionAllDevice(int* current_position)
{
    int pos;

    GetCurrentPosition(m_keyHandle_MCP, pos, m_Node_ID_MCP);
    current_position[0] = pos;
}

//  VysADN CloseBlade Function //
int MaxonMotor::getCloseBladePosition()
{
    int Current_Pos, new_Pos;

    unsigned int error_code = 0;
    
    if (!VCS_GetPositionIs(m_keyHandle_MCP, m_Node_ID_MCP, &Current_Pos, &error_code))
    {
        std::cout << " Error while getting current position , error_code = " << error_code << std::endl;
        m_errorFlag = 1;
    }
        new_Pos = ( Current_Pos % 20480);

    if(new_Pos  < 12240 && new_Pos > -12240 && new_Pos!=0 )
    {
            new_Pos=Current_Pos-new_Pos;
     //       std::cout << "First if .. : " << new_Pos << std::endl;
    }

    else if(new_Pos > 12240)
    {
        new_Pos = Current_Pos+(20480-new_Pos);
       // std::cout << "CCW else if .. : " << new_Pos << std::endl;
    }
    else if(new_Pos < -12240)
    {
        new_Pos = Current_Pos-(20480+new_Pos);
        //std::cout << "CCW else if .. : " << new_Pos << std::endl;
    }
    else
    {
        new_Pos=Current_Pos;
        //std::cout << "NO MOVE if .. : " << new_Pos << std::endl;
    }
    return new_Pos;
}

// VysADN CloseBlade Function ended. //

// VysADN Error Flag Function  
int MaxonMotor::EPOSGetError()
{
    return m_errorFlag ;
}
int MaxonMotor::GetCurrentVelocity()
{
    unsigned int error_code = 0;
    int currentVelocity=0;
    if(m_Mode == OMD_PROFILE_VELOCITY_MODE)
    {
        if(!VCS_GetVelocityIs(m_keyHandle_MCP, m_Node_ID_MCP, &currentVelocity, &error_code))
        {

        }
    }
    else
    {
     std::cout << "Error Velocity Mode is not active!!" << std::endl;
     return 0;
    }
    return currentVelocity;
}
