// --------------------------------------------------------------- //
// CKim - class "MaxonMotor" encapsulating functions of the
// EPOS Command library
// Last Updated : 2019.08.21 CKim
// --------------------------------------------------------------- //

#ifndef EPOS4_CAN_H
#define EPOS4_CAN_H

#include <EposCmdLib_6.5.1.0/include/Definitions.h>
#include <iostream>
#include <wiringPi.h>
#include <m_defines.h>

class MaxonMotor
{
private:
    char* PortName_MCP;             // Port Name
    unsigned int ErrorCode;         // VCS 함수에서 error 발생시 return 되는 값
    unsigned short m_Node_ID_MCP;   // Node ID 번호

    void* m_keyHandle_MCP;          // Handle to the CAN port. Open Device를 하기 위한
    char m_Mode;

    int m_OscOffset;
    int m_OscAmp;
    long ReferencePosition;
    int m_errorFlag;

    void EnableDevice(void* keyHandle_, unsigned short Node_ID);
    void DisableDevice(void* keyHandle_, unsigned short Node_ID);

    void* ActivateDevice(char* PortName, unsigned short Node_ID);

    void CloseDevice(void* keyHandle_);

    void Move(void* keyHandle_, long target_velocity, unsigned short Node_ID);
    void MoveToPosition(void* keyHandle_, long target_position, unsigned short Node_ID);

public:
    MaxonMotor();

    void CloseAllDevice();

    void ActiviateAllDevice();

    void DisableAllDevice();

    void GetCurrentPositionAllDevice(int* current_position);

    void MoveAllDevice(long* target_velocity);

    int CloseBlade();

    int EPOSGetError();
    // -----------------------------------------------
    void EnableVelocityMode();
    void EnablePositionMode();

    void MoveVelocity(const long& target_velocity);
    void MovePosition(const long& target_pos);

    void GetCurrentVelocity();
    void GetCurrentPosition(void *keyHandle_, int& current_position, unsigned short Node_ID);

    void SetOscMode(int Amp);
    void RunOscMode(int dir);
    void StopMotion();
    void WaitForMotion();
};


#endif // EPOS4_CAN_H
