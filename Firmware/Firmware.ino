
#include <WiFi.h>
#include <WiFiClient.h>
#include "Rotator.h"
#include <Wire.h>
#include <VL53L1X.h>


Rotator* cMotor;
//motor objects
Rotator rot1;
VL53L1X sensor;

int maxAngle = 90; //this is the range of motion of your sink

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(400000); // use 400 kHz I2C

  //set up sensor
  sensor.setTimeout(500);
  if (!sensor.init())
  {
    Serial.println("Failed to detect and initialize sensor!");
    while (1);
  }
  sensor.setDistanceMode(VL53L1X::Short);
  sensor.setMeasurementTimingBudget(20000);
  sensor.startContinuous(50);

  //set up motor
  rot1.motorInit();
  cMotor = &rot1;
  rot1.rotatetimer = timerBegin(0, 80, true);
  timerAttachInterrupt(rot1.rotatetimer, &rotateTimerEvent, true);
}

void loop() {
  int distance = sensor.read();
  if (distance < 300) {
    Serial.print(distance);
    int coord = convertDistance(distance);
    cMotor->moveSteps(coord);
    Serial.print(" current coord: ");
    Serial.println(cMotor->currentCoord);
  }
}

int convertDistance (int distance) { // convert distance to motor steps
  //300 -> 90 degrees (800 steps)
  return distance * (3200 * maxAngle / 360) / 300;
}







void IRAM_ATTR rotateTimerEvent()
{
  //portENTER_CRITICAL_ISR(&rotatetimerMux);
  if ( cMotor->stepCount < cMotor->totalSteps ) {
    REG_WRITE(GPIO_OUT_W1TS_REG, BIT0); //STEP_HIGH reg write
    REG_WRITE(GPIO_OUT_W1TC_REG, BIT0); //STEP_LOW reg write
    cMotor->stepCount++;
  }
  else {
    cMotor->d = cMotor->rotateAccel;
    cMotor->movementDone = true;
    //portEXIT_CRITICAL_ISR(&rotatetimerMux);
    timerAlarmDisable(cMotor->rotatetimer);
  }
  if ( cMotor->rampUpStepCount == 0 ) { // ramp up phase
    cMotor->n++;
    cMotor->d = abs(cMotor->d - (2 * cMotor->d) / (4 * cMotor->n + 1));
    if ( cMotor->d <= cMotor->rotateSpd ) { // reached max speed
      cMotor->d = cMotor->rotateSpd;
      cMotor->rampUpStepCount = cMotor->stepCount;
    }
    if ( cMotor->stepCount >= cMotor->totalSteps / 2 ) { // reached halfway point
      cMotor->rampUpStepCount = cMotor->stepCount;
    }
  }
  else if ( cMotor->stepCount >= cMotor->totalSteps - cMotor->rampUpStepCount ) { // ramp down phase
    cMotor->d = abs((cMotor->d * (4 * cMotor->n + 1)) / (4 * cMotor->n + 1 - 2));
    cMotor->n--;
  }
  timerAlarmWrite(cMotor->rotatetimer, cMotor->d, true);
}

void IRAM_ATTR rotateTimerEvent_continuous()
{
  //portENTER_CRITICAL_ISR(&rotateconttimerMux);
  REG_WRITE(GPIO_OUT_W1TS_REG, BIT0); //STEP_HIGH reg write
  REG_WRITE(GPIO_OUT_W1TC_REG, BIT0); //STEP_LOW reg write
  if (cMotor->rotateCont == 0) { //ramp down phase
    cMotor->d = abs((cMotor->d * (4 * cMotor->n + 1)) / (4 * cMotor->n + 1 - 2));
    cMotor->n--;
  } else if ( cMotor->d > cMotor->rotateSpd ) { // ramp up phase
    cMotor->n++;
    cMotor->d = abs(cMotor->d - (2 * cMotor->d) / (4 * cMotor->n + 1));
  } else if (cMotor->d < cMotor->rotateSpd) {
    cMotor->d = abs((cMotor->d * (4 * cMotor->n + 1)) / (4 * cMotor->n + 1 - 2));
    cMotor->n--;
  } else {
    cMotor->d = cMotor->rotateSpd;
  }
  timerAlarmWrite(cMotor->rotateconttimer, cMotor->d, true);
}
