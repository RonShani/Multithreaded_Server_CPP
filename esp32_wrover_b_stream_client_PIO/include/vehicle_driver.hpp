#ifndef VEHICLE_DRIVER_HPP
#define VEHICLE_DRIVER_HPP

#include <Adafruit_PWMServoDriver.h>
#include "communication_manager.hpp"
#include "motor_driver.hpp"
#include "blinker.hpp"

class Vehicle{
public:
    Vehicle(CommunicationManager &a_cmng);
    void forward(uint16_t const &a_speed);
    void backward(uint16_t const &a_speed);
    void stop();
    void left_turn(uint16_t const &a_duration, uint16_t const &a_speed);
    void right_turn(uint16_t const &a_duration, uint16_t const &a_speed);
    void blink_left();
    void blink_right();
    void blink_both();
    void blink_stop();

public:
    static void turn_left(Vehicle &a_car);
    static void go_backward(Vehicle &a_car);
    static void go_forward(Vehicle &a_car);
    static void blink_ok(Vehicle &a_car);

private:
    Adafruit_PWMServoDriver &m_pwm;
    MotorDriver m_front_left;
    MotorDriver m_front_right;
    MotorDriver m_rear_left;
    MotorDriver m_rear_right;
    Blinker m_blink_left;
    Blinker m_blink_right;
};

#endif // VEHICLE_DRIVER_HPP