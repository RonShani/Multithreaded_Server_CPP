#include "motor_driver.hpp"

MotorDriver::MotorDriver(uint8_t a_a, uint8_t a_b, uint8_t a_speed, Adafruit_PWMServoDriver &a_pwm)
: m_a(a_a), m_b(a_b), m_speed(a_speed), m_pwm(a_pwm)
{
}

void MotorDriver::forth(uint16_t a_speed)
{
    m_pwm.setPWM(m_a, 0, 0);
    m_pwm.setPWM(m_b, 4096, 0);
    m_pwm.setPWM(m_speed, 0, a_speed);
}

void MotorDriver::back(uint16_t a_speed)
{
    m_pwm.setPWM(m_a, 4096, 0);
    m_pwm.setPWM(m_b, 0, 0);
    m_pwm.setPWM(m_speed, 0, a_speed);
}

void MotorDriver::stop()
{
    m_pwm.setPWM(m_a, 0, 0);
    m_pwm.setPWM(m_b, 0, 0);
    m_pwm.setPWM(m_speed, 0, 0);
}
