/**
 ***************************************************************************
 * \file  epos4can.h
 * \brief Header file includes MaxonMotor class, which encapsulates functions
 * of the EPOS command library. Modified to 'Qthread' to provide execution of
 * initialization and oscillation mode in seperate thread.
 *
 * Last Updated : 2022.03.16 Chunwoo Kim (CKim) & Veysi ADIN (VysAdn)
 * Contact Info : cwkim@kist.re.kr & veysi.adin@kist.re.kr
 ***************************************************************************
**/

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
     * @brief DisableAllDevice Puts motor driver in disabled state.
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
     * @param current_position Value to be written the returned position value.
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
    ///  - Port name
    char* m_PortName_MCP;

    ///  - Handle to the CAN port
    void* m_keyHandle_MCP;

    ///  - Node ID
    unsigned short m_Node_ID_MCP;

    ///  - Error code returned from VCS functions
    unsigned int ErrorCode;

    /// - Operating modes and parameters
    char m_Mode;
    /// - Oscillation mode offset.
    int m_OscOffset;
    /// - Oscillation mode amplitude.
    int m_OscAmp;
    /// - EPOS error flag.
    int m_errorFlag;

    /**
     * @brief EnableDevice Enables specific node in specific port.
     * @param keyHandle_ opened port handle.
     * @param Node_ID node identifier.
     */
    void EnableDevice(void* keyHandle_, unsigned short Node_ID);

    /**
     * @brief DisableDevice Disable specific node in specific port.
     * @param keyHandle_ opened port handle.
     * @param Node_ID node identifier.
     */
    void DisableDevice(void* keyHandle_, unsigned short Node_ID);

    /**
     * @brief ActivateDevice Opens the port with "CAN_mcp251x", "CANOpen" and "EPOS4" parameters.
     * @param PortName Name of the port (default "CAN0").
     * @param Node_ID Identifier of the specified node (default "1").
     * */
    void* ActivateDevice(char* PortName, unsigned short Node_ID);

    /**
     * @brief CloseDevice Closes opened ports.
     * @param keyHandle_ Opened port handle.
     */
    void CloseDevice(void* keyHandle_);

    /**
     * @brief Move Moves motor with specified target velocity command for specific node in specific port.
     * @param keyHandle_ opened port handle.
     * @param target_velocity target velocity.
     * @param Node_ID Node identifier.
     */
    void Move(void* keyHandle_, long target_velocity, unsigned short Node_ID);

    /**
     * @brief MoveToPosition Sends specific target position command for specific node in specific port.
     * @param keyHandle_ opened port handle.
     * @param target_position target position.
     * @param Node_ID Node identifier.
     */
    void MoveToPosition(void* keyHandle_, long target_position, unsigned short Node_ID);

    /**
     * @brief GetCurrentPosition Requests current position from specific node in specific port.
     * @param keyHandle_ opened port handle.
     * @param current_position output parameter.
     * @param Node_ID Node identifier.
     */
    void GetCurrentPosition(void *keyHandle_, int& current_position, unsigned short Node_ID);

    /**
     * @brief MoveInCloseBladeMode Moves motor with CLOSE_BLADE_VELOCITY (20RPM),
     * @note This function is application specific, It is required to close the blade
     * of debrider with slow movement.
     */
    void MoveInCloseBladeMode();
};


#endif // EPOS4_CAN_H
