#ifndef BLINKER_DRIVER_HPP
#define BLINKER_DRIVER_HPP

#include <Adafruit_PWMServoDriver.h>


class Blinker{
public:
    Blinker(uint8_t a_led_channel, Adafruit_PWMServoDriver &a_pwm);
    void on(uint16_t a_speed);
    void off();

private:
    uint8_t m_led_channel;
    Adafruit_PWMServoDriver &m_pwm;
};

#endif // BLINKER_DRIVER_HPP