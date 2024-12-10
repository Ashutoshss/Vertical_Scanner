#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <SPI.h>
#include <SD.h>

//Variable
float D_fixed = 150;
volatile bool limitSwitchTriggered = false;  // Flag to indicate the interrupt
int xystep_per_rotation_count = 0;
int zstep_per_rotation_count = 0;

const int stepsPerRevolution = 200; // Number of steps per revolution for the motor
const float anglePerStep = 360.0 / stepsPerRevolution; // Angle moved per step
float currentAngle = 0.0;
float x = 0.0;
float y = 0.0;
float z = 0.0;

//Pins
#define XYSTEP_PIN 2     // Pin connected to the STEP pin of the driver
#define XYDIR_PIN 3      // Pin connected to the DIR pin of the driver
#define ZSTEP_PIN 6     // Pin connected to the STEP pin of the driver
#define ZDIR_PIN 7      // Pin connected to the DIR pin of the driver

const int buzzerPin = 8;
const int chipSelect = 4;
const int limitSwitchPin = 2;    // Pin where the limit switch is connected

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

void setup() {
  Serial.begin(115200);
  
  if (!lox.begin()) {
    Serial.println("Failed to initialize VL53L0X. Check wiring!");
    while (1);
  }
  Serial.println("VL53L0X initialized successfully.");
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    while (1);
  }
  Serial.println("card initialized.");
  pinMode(buzzerPin, OUTPUT);
  pinMode(XYSTEP_PIN, OUTPUT);
  pinMode(XYDIR_PIN, OUTPUT); 
  pinMode(ZSTEP_PIN, OUTPUT);
  pinMode(ZDIR_PIN, OUTPUT);

  pinMode(limitSwitchPin, INPUT_PULLUP); 
  attachInterrupt(digitalPinToInterrupt(limitSwitchPin), limitSwitchISR, FALLING);

  DataLogger(x,y,z);    // initialize the data with cordinate 0.0 ,0.0, 0.0  
}

void loop() {
  if (!limitSwitchTriggered){

    if(xystep_per_rotation_count <= stepsPerRevolution){
      VL53L0X_RangingMeasurementData_t measure;
      lox.rangingTest(&measure, false);
      
      if (measure.RangeStatus != 4) {
        float D_measured = measure.RangeMilliMeter;
        float Radius = D_fixed - D_measured;
        x = Radius * cos(radians(currentAngle));
        y = Radius * sin(radians(currentAngle));
        DataLogger(x,y,z);
      } else {
        Serial.println("Out of range.");
      }
      digitalWrite(XYSTEP_PIN, HIGH); // Send a pulse to step the motor
      delayMicroseconds(1000);      // Wait for 1 millisecond (adjust for speed)
      digitalWrite(XYSTEP_PIN, LOW);  // Complete the step
      delayMicroseconds(1000);      // Wait for 1 millisecond
      xystep_per_rotation_count = xystep_per_rotation_count +1;
      currentAngle = currentAngle+anglePerStep;
      delay(10);
      }
    else if (zstep_per_rotation_count == stepsPerRevolution){
      // rotate the actuator one full rotation with a pitch of 8mm and count it in z axis
      for (int i = 0; i < stepsPerRevolution; i++) {
        digitalWrite(ZSTEP_PIN, HIGH);
        delayMicroseconds(1000);
        digitalWrite(ZSTEP_PIN, LOW);
        delayMicroseconds(1000);
      }
      z = z+2;  // in mm(Thread 2mm Pitch Lead Screw with Copper Nut)
      xystep_per_rotation_count = 0;
      zstep_per_rotation_count = 0;
    }
    
    }
  else{
    Serial.println("limitSwitchTriggered");
    digitalWrite(buzzerPin, HIGH);  // Turn on the buzzer
    delay(500);                     // Keep the buzzer on for 500 ms
    digitalWrite(buzzerPin, LOW);
    delay(500);
    }
}

void limitSwitchISR() {
  limitSwitchTriggered = true;  
}
void DataLogger(float x, float y, float z){
  File myFile = SD.open("datalog.txt", FILE_WRITE);

  if (myFile) {
    myFile.print(x);
    myFile.print(",");
    myFile.print(y);
    myFile.print(",");
    myFile.println(z);
    myFile.close();
  }
  else {
    Serial.println("error opening datalog.txt");
  }
}
