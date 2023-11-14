#include "communication_manager.hpp"

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <SPI.h>

#define I2C_SDA 12 //22 P20
#define I2C_SCL 14 //21 P19

CommunicationManager::CommunicationManager(TwoWire &a_wires)
: m_wires(a_wires)
, m_pwm(Adafruit_PWMServoDriver(0x47, a_wires))
{}

void CommunicationManager::begin()
{
    m_wires.begin(I2C_SDA, I2C_SCL, 400000);
    m_pwm.begin();
}

Adafruit_PWMServoDriver &CommunicationManager::pwm()
{
    return m_pwm;
}
