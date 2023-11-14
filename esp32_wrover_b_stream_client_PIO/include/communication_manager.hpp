#ifndef COMMUNICATION_MANAGER_HPP
#define COMMUNICATION_MANAGER_HPP

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <SPI.h>

class CommunicationManager{
public:
    CommunicationManager(TwoWire &a_wires);
    void begin();
    Adafruit_PWMServoDriver &pwm();


private:
    TwoWire &m_wires;
    Adafruit_PWMServoDriver m_pwm;
};

#endif // COMMUNICATION_MANAGER_HPP