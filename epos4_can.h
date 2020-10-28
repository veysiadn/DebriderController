// --------------------------------------------------------------- //
// CKim - class "MaxonMotor" encapsulating functions of the
// EPOS Command library. Modified to 'QThread' to provide
// execution of initialization and oscillation mode in separate thread
// Last Updated : 2020.10.20 CKim
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

    // CKim - CAN port parameters
    char* m_PortName_MCP;           // CKim - Port name
    void* m_keyHandle_MCP;          // CKim - Handle to the CAN port
    unsigned short m_Node_ID_MCP;   // CKim - Node ID
    unsigned int ErrorCode;         // CKim - Error code returned from VCS functions

    // CKim - Operating modes and parameters
    char m_Mode;
    int m_OscOffset;
    int m_OscAmp;
    long ReferencePosition;
    int m_errorFlag;

    // CKim - Enable / Disable specific node in specific port
    void EnableDevice(void* keyHandle_, unsigned short Node_ID);
    void DisableDevice(void* keyHandle_, unsigned short Node_ID);

    void* ActivateDevice(char* PortName, unsigned short Node_ID);
    void CloseDevice(void* keyHandle_);

    // CKim - Motion command for specific node in specific port
    void Move(void* keyHandle_, long target_velocity, unsigned short Node_ID);
    void MoveToPosition(void* keyHandle_, long target_position, unsigned short Node_ID);
    void GetCurrentPosition(void *keyHandle_, int& current_position, unsigned short Node_ID);

public:
    MaxonMotor();

    // CKim - Open CAN port and configures communication
    // CAN port name, node id and baudrate is hard coded for now.
    int OpenCANCommunication();

    // CKim - Enable all devices
    int EnableMotorController();

    void ActiviateAllDevice();
    void DisableAllDevice();
    void CloseAllDevice();
    int EPOSGetError();
    void EnablePositionModeWithSpeed(unsigned int speed);
    void GetCurrentPositionAllDevice(int* current_position);
    void MoveAllDevice(long* target_velocity);
    int getCloseBladePosition();

    // -----------------------------------------------
    void EnableVelocityMode();
    void EnablePositionMode();

    void MoveVelocity(const long& target_velocity);
    void MovePosition(const long& target_pos);

    int GetCurrentVelocity();
    void SetOscMode(int Amp);
    void RunOscMode(int dir);
    void StopMotion();
    void WaitForMotion();
};


#endif // EPOS4_CAN_H
