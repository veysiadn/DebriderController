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
     * @brief Opens CAN port and configures communication.
     * @note CAN port name, node id and baudrate is hard coded for now.
     * @return 1 if succesful otherwise 0.
     */
    int OpenCANCommunication();

    /**
     * @brief Enables motor controller and clears faults.
     * @return 1 if succesful, otherwise 0.
     */
    int EnableMotorController();

    /**
     * @brief Activates connected EPOS driver, with hard coded
     * communication parameters such as node id, baudrate and CAN port name.
     */
    void ActiviateAllDevice();

    /**
     * @brief Puts motor driver in disabled state.
     */
    void DisableAllDevice();

    /**
     * @brief Closes connected controller.
     */
    void CloseAllDevice();

    /**
     * @brief Returns error flag from command library.
     * @return Error flag value.
     */
    int  EPOSGetError();

    /**
     * @brief Enables position mode with specific
     * speed value
     * @param speed Velocity value which will be used in position mode.
     */
    void EnablePositionModeWithSpeed(int speed);

    /**
     * @brief Gets current position values of connected motors
     * in terms of encoder increment value.
     * @param current_position returned position value.
     */
    void GetCurrentPositionAllDevice(int& current_position);

    /**
     * @brief Finds position of motor which will correspond to closed
     * blade position in the debrider.
     * @return Close blade position in terms of encoder increment values.
     */
    int  GetCloseBladePosition();

    /**
     * @brief Enables motor with velocity mode.
     */
    void EnableVelocityMode();

    /**
     * @brief Enables motor with position mode.
     */
    void EnablePositionMode();

    /**
     * @brief Sends target velocity command to motor,motor moves according to
     * target velocity.
     * @param target_velocity Desired velocity.
     */
    void MoveVelocity(const long& target_velocity);

    /**
     * @brief Sends target position value to motor, motor moves
     * to target position.
     * @param target_pos Desired position.
     */
    void MovePosition(const long& target_pos);

    /**
     * @brief Gets current velocity of motor.
     * @return Current velocity of motor.
     */
    int GetCurrentVelocity();

    /**
     * @brief Put motor in oscillation mode.
     * @param Amp
     */
    void SetOscMode(int Amp);

    /**
     * @brief Runs motor in oscillation mode.
     * @param dir direction of movement. @note direction will be changing periodically.
     */
    void RunOscMode(int dir);

    /**
     * @brief Stops the motor motion in position mode.
     */
    void StopMotion();

    /**
     * @brief Waits for motor to reach target position.
     */
    void WaitForMotion();

    /**
     * @brief Checks whether target is reached or not.
     * @return 1 if drive reached the target, otherwise 0.
     */
    int GetMovementState();

    /**
     * @brief Stops the motor motion in velocity mode.
     */
    void StopVelocity();

private:
    ///   Port name
    char* m_PortName_MCP;

    ///   Handle to the CAN port
    void* m_keyHandle_MCP;

    ///   Node ID
    unsigned short m_Node_ID_MCP;

    ///   Error code returned from VCS functions
    unsigned int ErrorCode;

    ///  Operating modes and parameters
    char m_Mode;
    ///  Oscillation mode offset.
    int m_OscOffset;
    ///  Oscillation mode amplitude.
    int m_OscAmp;
    ///  EPOS error flag.
    int m_errorFlag;

    /**
     * @brief Enables specific node in specific port.
     * @param keyHandle_ opened port handle.
     * @param Node_ID node identifier.
     */
    void EnableDevice(void* keyHandle_, unsigned short Node_ID);

    /**
     * @brief Disable specific node in specific port.
     * @param keyHandle_ opened port handle.
     * @param Node_ID node identifier.
     */
    void DisableDevice(void* keyHandle_, unsigned short Node_ID);

    /**
     * @brief Opens the port with "CAN_mcp251x", "CANOpen" and "EPOS4" parameters.
     * @param PortName Name of the port (default "CAN0").
     * @param Node_ID Identifier of the specified node (default "1").
     * */
    void* ActivateDevice(char* PortName, unsigned short Node_ID);

    /**
     * @brief Closes opened ports.
     * @param keyHandle_ Opened port handle.
     */
    void CloseDevice(void* keyHandle_);

    /**
     * @brief Moves motor with specified target velocity command for specified node in specified port.
     * @param keyHandle_ opened port handle.
     * @param target_velocity desired velocity.
     * @param Node_ID Node identifier.
     */
    void Move(void* keyHandle_, long target_velocity, unsigned short Node_ID);

    /**
     * @brief Sends specific target position command for specified node in specified port.
     * @param keyHandle_ opened port handle.
     * @param target_position target position.
     * @param Node_ID Node identifier.
     */
    void MoveToPosition(void* keyHandle_, long target_position, unsigned short Node_ID);

    /**
     * @brief Requests current position from specified node in specified port.
     * @param keyHandle_ opened port handle.
     * @param current_position output parameter.
     * @param Node_ID Node identifier.
     */
    void GetCurrentPosition(void *keyHandle_, int& current_position, unsigned short Node_ID);

    /**
     * @brief Moves motor with CLOSE_BLADE_VELOCITY (20RPM),
     * @note This function is application specific. It is required to close the blade
     * of debrider with slow movement.
     */
    void MoveInCloseBladeMode();
};


#endif // EPOS4_CAN_H
