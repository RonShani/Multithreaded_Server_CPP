#include "vehicle_driver.hpp"

#include <Adafruit_PWMServoDriver.h>

Vehicle::Vehicle(CommunicationManager &a_cmng)
: m_pwm(a_cmng.pwm())
, m_front_left(4,3,5, m_pwm)
, m_front_right(2,1,0, m_pwm)
, m_rear_left(10,9,11,m_pwm)
, m_rear_right(8,7,6, m_pwm)
, m_blink_left(12, m_pwm)
, m_blink_right(13, m_pwm)
{

}

void Vehicle::forward(uint16_t const &a_speed)
{
    m_front_left.forth(a_speed);
    m_front_right.forth(a_speed);
    m_rear_left.forth(a_speed);
    m_rear_right.forth(a_speed);
}

void Vehicle::backward(uint16_t const &a_speed)
{
    m_front_left.back(a_speed);
    m_front_right.back(a_speed);
    m_rear_left.back(a_speed);
    m_rear_right.back(a_speed);
}

void Vehicle::left_turn(uint16_t const &a_duration, uint16_t const &a_speed)
{
    m_front_left.back(a_speed);
    m_rear_left.back(a_speed);
    m_front_right.forth(a_speed);
    m_rear_right.forth(a_speed);
    delay(a_duration);
    stop();
}

void Vehicle::right_turn(uint16_t const &a_duration, uint16_t const &a_speed)
{
    m_front_left.forth(a_speed);
    m_rear_left.forth(a_speed);
    m_front_right.back(a_speed);
    m_rear_right.back(a_speed);
    delay(a_duration);
    stop();
}

void Vehicle::stop()
{
    m_front_left.stop();
    m_front_right.stop();
    m_rear_left.stop();
    m_rear_right.stop();
}

void Vehicle::blink_left()
{
    m_blink_left.on(4095);
}

void Vehicle::blink_right()
{
    m_blink_right.on(4095);
}

void Vehicle::blink_both()
{
    m_blink_left.on(4095);
    m_blink_right.on(4095);
}

void Vehicle::blink_stop()
{
    m_blink_left.off();
    m_blink_right.off();
}


void Vehicle::blink_ok(Vehicle &a_car)
{
    a_car.blink_both();
    delay(20);
    a_car.blink_stop();
}

void Vehicle::go_forward(Vehicle &a_car)
{
  a_car.forward(2048);
  delay(1000);
  a_car.stop();
}

void Vehicle::go_backward(Vehicle &a_car)
{
  a_car.backward(2048);
  delay(1000);
  a_car.stop();
}

void Vehicle::turn_left(Vehicle &a_car)
{
  a_car.left_turn(400, 2048);
}