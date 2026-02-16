#include "Encoder.h"

volatile long Encoder::countLeft = 0;
volatile long Encoder::countRight = 0;

Encoder::Encoder()
: lastL(0), lastR(0),
  rpmL(0), rpmR(0),
  speedL(0), speedR(0) {}

void IRAM_ATTR Encoder::isrLeft() {
    if (digitalRead(DIR_L))
        countLeft++;
    else
        countLeft--;
}

void IRAM_ATTR Encoder::isrRight() {
    if (digitalRead(DIR_R))
        countRight--;
    else
        countRight++;
}

void Encoder::begin() {

    pinMode(PULSE_L, INPUT);
    pinMode(DIR_L, INPUT);
    pinMode(PULSE_R, INPUT);
    pinMode(DIR_R, INPUT);

    attachInterrupt(digitalPinToInterrupt(PULSE_L), isrLeft, RISING);
    attachInterrupt(digitalPinToInterrupt(PULSE_R), isrRight, RISING);
}

void Encoder::update() {

    noInterrupts();
    long nowL = countLeft;
    long nowR = countRight;
    interrupts();

    long deltaL = nowL - lastL;
    long deltaR = nowR - lastR;

    lastL = nowL;
    lastR = nowR;

    rpmL = deltaL * 50.0;
    rpmR = deltaR * 50.0;

    float wheelCirc = 3.1416 * WHEEL_DIAMETER;
    speedL = (rpmL / 60.0) * wheelCirc;
    speedR = (rpmR / 60.0) * wheelCirc;
}

float Encoder::getRPM_L() { return rpmL; }
float Encoder::getRPM_R() { return rpmR; }
float Encoder::getSpeed_L() { return speedL; }
float Encoder::getSpeed_R() { return speedR; }
