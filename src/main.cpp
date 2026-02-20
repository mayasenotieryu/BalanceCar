#include <Arduino.h>
#include <PWM.h>

PWM motor;

void setup()
{
    Serial.begin(115200);
    motor.begin();
}

void loop()
{
    static int speed = 0;
    static int step = 5;

    motor.setSpeed(speed);

    speed += step;
    delay(100);
    if (speed > 200)
    {
        delay(3000);
        speed = -200;
    }

    delay(100);
}