
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>

#include "Rotator.h"
#include <Wire.h>
#include <VL53L1X.h>

// insert WiFi network credentials here
const char* ssid = "username";
const char* password = "password";

// motor objects
Rotator* cMotor;
Rotator rot1;
VL53L1X sensor;

int maxAngle = 90; //this is the range of motion of your sink
int sensitivity = 200; // this is the sensitivity of the sensor
unsigned long onTimer = 0;
int timerDelay = 10000; // time until automatic shutoff
bool faucetStatus = false; //false = closed, true = open
unsigned long httpTimer = 0;
int httpDelay = 10000; // http post every 10 seconds

void setup() {
  Serial.begin(115200);
  
  WiFi.begin(ssid, password); // connect to wifi
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nConnected to the WiFi network");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  //set up sensor
  
  Wire.begin();
  Wire.setClock(400000); // use 400 kHz I2C
  
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

  // GET settings from app
  if ((WiFi.status() == WL_CONNECTED) && millis() - httpTimer > httpDelay) {
    getFaucetSettings();
    httpTimer = millis();
  } else if (WiFi.status() != WL_CONNECTED){
    Serial.println("Connection lost");
  }

  // read from sensor and move accordingly
  int distance = sensor.read();
  
  if (distance < sensitivity && distance > 0) {
    int coord = convertDistance(distance);
    cMotor->moveSteps(-coord);
    onTimer = millis();
    
    sendFaucetStatus(distance);
  
  } else {
    if (millis() - onTimer > timerDelay) { // after one minute, automatically shut off
      cMotor->moveSteps(0);

      sendFaucetStatus(0);
    }
  }
    
}

void getFaucetSettings() {
  HTTPClient http;
  http.begin("http://192.168.1.2:5000/api/get-settings");
  int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      JSONVar myObject = JSON.parse(payload);
      JSONVar keys = myObject.keys();

      maxAngle = keys[0];
      sensitivity = keys[1];
      timerDelay = keys[2];
      
    } else {
      Serial.println("Error on HTTP request");
    }
    
    http.end();

}

void sendFaucetStatus(int distance) {
  int currentTime = millis();
  int openAngle = distance * (maxAngle) / sensitivity;
  if (distance > 50 && faucetStatus == false) { // send status to app
    faucetStatus = true;
    Serial.print("faucet open @ "); Serial.println(currentTime);
  } else if (distance <= 50 && faucetStatus == true){
    faucetStatus = false;
    Serial.print("faucet close @ "); Serial.println(currentTime);
  }

  //http POST
  HTTPClient http;

  http.begin("http://192.168.1.2:5000/api/log");
  http.addHeader("Content-Type", "application/json");
  String httpRequestData = "{\"time\":\"" + String(currentTime) + "\",\"is_open\":\"" + String(faucetStatus) + "\",\"open_angle\":\"" + String(openAngle) + "\"}";
  int httpCode = http.POST(httpRequestData);

  if (httpCode > 0) {
    String response = http.getString();
    Serial.println(httpCode);
    Serial.println(response);
  } else {
    Serial.println("Error on HTTP POST");
  }
    
  http.end();
}

int convertDistance (int distance) { // convert distance to motor steps
  //300 -> 90 degrees (800 steps)
  return distance * (3200 * maxAngle / 360) / sensitivity;
}





// ISRs for motor rotation

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
