#ifndef MOTOR_DRIVER_HPP
#define MOTOR_DRIVER_HPP

#include <Adafruit_PWMServoDriver.h>


class MotorDriver{
public:
    MotorDriver(uint8_t a_a, uint8_t a_b, uint8_t a_speed, Adafruit_PWMServoDriver &a_pwm);
    void forth(uint16_t a_speed);
    void back(uint16_t a_speed);
    void stop();

private:
    uint8_t m_a;
    uint8_t m_b;
    uint8_t m_speed;
    Adafruit_PWMServoDriver &m_pwm;
};

#endif // MOTOR_DRIVER_HPP