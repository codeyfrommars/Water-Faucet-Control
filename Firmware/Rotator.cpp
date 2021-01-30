#include "Rotator.h"


Rotator::Rotator() {
  Motor();
  rotateCont = 0;
  rotateRand = 0;
  randdelay = 5;
  currentCoord = 0;
}

Rotator::~Rotator() {}

void Rotator::rotate_continuous_stop() {
  rotateCont = 0;
  while (n > 0);
  timerAlarmDisable(rotateconttimer);
}

void Rotator::rotate_continuous(int dir) {
  digitalWrite(rotatedirPin, dir < 0 ? HIGH : LOW);
  d = rotateAccel;
  timerAlarmWrite(rotateconttimer, d, true);
  n = 0;
  timerAlarmEnable(rotateconttimer);
}

void Rotator::rotate_random() {
  rotateSpd = ((rand() % 12)+4) * 1000 / microstep;
  rotateNSteps(R_STEPS_PER_REV / 360.0 * (rand() % 721));
  while (!movementDone);
  if (randdelay != 0)
    delay(rand() % randdelay);
  rotateSpd = ((rand() % 12)+4) * 1000 / microstep;
  rotateNSteps(-(R_STEPS_PER_REV / 360.0 * (rand() % 721)));
  while (!movementDone);
  if (randdelay != 0)
    delay(rand() % randdelay);
}

void Rotator::backandforth() {
  rotateNSteps(rotateAngle);
  while (!movementDone);
  delay(100);
  rotateNSteps(-rotateAngle);
  while (!movementDone);
  delay(100);
}

void Rotator::moveSteps(int coord) {
  rotateNSteps(coord - currentCoord);
  currentCoord = coord;
  while(!movementDone);
}
