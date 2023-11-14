#include "servo_handler.hpp"
#include <ESP32Servo.h>

ServoHandler::ServoHandler(int const &a_pin)
: m_pin(a_pin)
, m_pos(0)
, m_servo{}
{
}

void ServoHandler::begin()
{
    //ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	//ESP32PWM::allocateTimer(2);
	//ESP32PWM::allocateTimer(3);
	m_servo.setPeriodHertz(50);
	m_servo.attach(m_pin, 500, 2500);
}

void ServoHandler::write(int a_angle)
{
    m_servo.write(a_angle);
    m_pos = a_angle;
}

int ServoHandler::get_angle()
{
    return m_pos;
}
