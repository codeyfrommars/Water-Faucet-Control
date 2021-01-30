#pragma once
#include "Arduino.h"


class Motor {
public:
  unsigned int rotateAccel;
  int rotatedirPin;
  int rotatestepPin;
  int RMS3;
  int RMS2;
  int RMS1;
  int REN; //LOW = on, HIGH = off

  boolean rotateEnable;
  int R_STEPS_PER_REV;
  int rotateDelay;
  int rotateAngle;
  int microstep;
  
  hw_timer_t * rotatetimer;
  portMUX_TYPE rotatetimerMux;
  hw_timer_t * rotateconttimer;
  portMUX_TYPE rotateconttimerMux;
  volatile double d;
  volatile int dir; //direction
  volatile unsigned int rotateSpd;
  volatile unsigned long n;
  volatile unsigned long stepCount;
  volatile unsigned long rampUpStepCount;
  volatile unsigned long totalSteps;
  volatile int stepPosition;
  volatile bool movementDone;


  Motor();
  ~Motor();
  //set microstep size
  void set_fullstep();
  void set_quarterstep();
  void set_sixteenthstep();
  //initialize motor
  void motorInit();
  void startTimer();
  void rotateNSteps(int steps);

  
};

void IRAM_ATTR rotateTimerEvent();
