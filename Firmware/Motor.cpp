#include "Motor.h"

Motor::Motor() {
  rotateAccel = 50000;
  rotatedirPin = 2;
  rotatestepPin = 0;
  RMS3 = 27;
  RMS2 = 14;
  RMS1 = 12;
  REN = 13;
  
  rotateEnable = 1;
  R_STEPS_PER_REV = 200;
  rotateDelay = 1000;
  rotateAngle = R_STEPS_PER_REV / 360.0 * 60;
  microstep = 1;
  
  rotatetimer = NULL;
  rotatetimerMux = portMUX_INITIALIZER_UNLOCKED;
  rotateconttimer = NULL;
  rotateconttimerMux = portMUX_INITIALIZER_UNLOCKED;
  d = 100000;
  dir = 0; //direction
  rotateSpd = 6000;
  n = 0;
  stepCount = 0;
  rampUpStepCount = 0;
  totalSteps = 0;
  stepPosition = 0;
  movementDone = false;
}

Motor::~Motor() {}

void Motor::motorInit() {
  pinMode(rotatestepPin, OUTPUT);
  pinMode(rotatedirPin, OUTPUT);
  pinMode(REN, OUTPUT);
  pinMode(RMS1, OUTPUT);
  pinMode(RMS2, OUTPUT);
  pinMode(RMS3, OUTPUT);

  digitalWrite(REN, LOW); //turn on
  set_sixteenthstep();

  Serial.println("motor initialized");
}

void Motor::set_fullstep() {
  if (microstep == 1) {
    return;
  }
  R_STEPS_PER_REV = 200;
  rotateAccel /= pow(0.7071, sqrt(microstep));
  rotateSpd *= microstep;
  rotateAngle /= microstep;
  digitalWrite(RMS1, LOW);
  digitalWrite(RMS2, LOW);
  digitalWrite(RMS3, LOW);
  microstep = 1;
}

void Motor::set_quarterstep() {
  if (microstep == 4) {
    return;
  }
  R_STEPS_PER_REV = 800;
  if (microstep < 4) {
    rotateAccel *= pow(0.7071, (2 / microstep));
    rotateSpd /= 4 / microstep;
    rotateAngle *= 4 / microstep;
  } else {
    rotateAccel /= pow(0.7071, (microstep / 8));
    rotateSpd *= microstep / 4;
    rotateAngle /= microstep / 4;
  }
  digitalWrite(RMS1, LOW);
  digitalWrite(RMS2, HIGH);
  digitalWrite(RMS3, LOW);
  microstep = 4;
}

void Motor::set_sixteenthstep() {
  if (microstep == 16) {
    return;
  }
  R_STEPS_PER_REV = 3200;
  rotateAccel *= pow(0.7071, 4 / sqrt(microstep));
  rotateSpd /= 16 / microstep;
  rotateAngle *= 16 / microstep;
  digitalWrite(RMS1, HIGH);
  digitalWrite(RMS2, HIGH);
  digitalWrite(RMS3, HIGH);
  microstep = 16;
}

void Motor::rotateNSteps(int steps) {
  digitalWrite(rotatedirPin, steps < 0 ? HIGH : LOW);
  totalSteps = abs(steps);
  d = rotateAccel;
  timerAlarmWrite(rotatetimer, d, true);
  stepCount = 0;
  n = 0;
  rampUpStepCount = 0;
  movementDone = false;
  timerAlarmEnable(rotatetimer);
}
