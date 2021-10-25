/*****************************************************************************
 * \file  epos4can.h
 * \brief Header file includes MaxonMotor class, which encapsulates functions
 * of the EPOS command library. Modified to 'Qthread' to provide execution of
 * initialization and oscillation mode in seperate thread.
 *
 * Last Updated : 2021.10.18 Chunwoo Kim (CKim) & Veysi ADIN (VysAdn)
 * Contact Info : cwkim@kist.re.kr & veysi.adin@kist.re.kr
 *******************************************************************************/

#ifndef EPOS4_CAN_H
#define EPOS4_CAN_H

#include <EposCmdLib_6.5.1.0/include/Definitions.h>
#include <iostream>
#include <wiringPi.h>
#include "m_defines.h"

class MaxonMotor
{
public:
    MaxonMotor();

    /**
     * @brief OpenCANCommunication Opens CAN port and configures communication.
     * @note CAN port name, node id and baudrate is hard coded for now.
     * @return 1 if succesfull otherwise 0.
     */
    int OpenCANCommunication();

    /**
     * @brief EnableMotorController Enables motor controller and clears faults.
     * @return 1 if succesfull, otherwise 0.
     */
    int EnableMotorController();
    /**
     * @brief ActiviateAllDevice Activates connected EPOS driver, with hard coded
     * communication information.
     */
    void ActiviateAllDevice();
    /**
     * @brief DisableAllDevice Puts motor driver in disabled state,
     */
    void DisableAllDevice();
    /**
     * @brief CloseAllDevice Closes connected controller.
     */
    void CloseAllDevice();
    /**
     * @brief EPOSGetError Returns error flag from command library.
     * @return Error flag value.
     */
    int  EPOSGetError();
    /**
     * @brief EnablePositionModeWithSpeed Enables position mode with specific
     * speed value
     * @param speed Velocity value which will be used in position mode.
     */
    void EnablePositionModeWithSpeed(int speed);
    /**
     * @brief GetCurrentPositionAllDevice Gets current position values of connected motors
     * in terms of encoder increment value.
     * @param current_position Value to be written the returned position value.s
     */
    void GetCurrentPositionAllDevice(int& current_position);
    /**
     * @brief GetCloseBladePosition Finds position of motor which will correspond to closed
     * blade position in the debrider.
     * @return Close blade position in terms of encoder increment values.
     */
    int  GetCloseBladePosition();

    /**
     * @brief EnableVelocityMode Enables motor with velocity mode.
     */
    void EnableVelocityMode();
    /**
     * @brief EnablePositionMode Enables motor with position mode.
     */
    void EnablePositionMode();
    /**
     * @brief MoveVelocity Sends target velocity command to motor,motor moves according to
     * target velocity.
     * @param target_velocity Velocity value for motor to move.
     */
    void MoveVelocity(const long& target_velocity);
    /**
     * @brief MovePosition Sends target position value to motor, motor moves
     * to target position.
     * @param target_pos Desired position.
     */
    void MovePosition(const long& target_pos);
    /**
     * @brief GetCurrentVelocity Gets current velocity of motor.
     * @return Current velocity of motor.
     */
    int GetCurrentVelocity();
    /**
     * @brief SetOscMode Put motor in oscillation mode.
     * @param Amp
     */
    void SetOscMode(int Amp);
    /**
     * @brief RunOscMode Runs motor in oscillation mode.
     * @param dir direction of movement. @note direction will be changing periodically.
     */
    void RunOscMode(int dir);
    /**
     * @brief StopMotion Stops the motion of motor.
     */
    void StopMotion();
    /**
     * @brief WaitForMotion Waits for motor to reach target position.
     */
    void WaitForMotion();
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
};


#endif // EPOS4_CAN_H
