#ifndef SERVO_HANDLER_HPP
#define SERVO_HANDLER_HPP

#include <ESP32Servo.h>

class ServoHandler{
public:
    ServoHandler(int const &a_pin);
    void begin();
    void write(int a_angle);
    int get_angle();

private:
    int m_pin;
    int m_pos;
    Servo m_servo;
};


#endif //SERVO_HANDLER_HPP