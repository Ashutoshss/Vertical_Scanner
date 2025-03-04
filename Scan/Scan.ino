#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <SPI.h>
#include <SD.h>

#define DIR_PIN  2 
#define STEP_PIN 3
#define ZDIR_PIN  6 
#define ZSTEP_PIN 7

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

const int stepsPerRevolution = 200; // Number of steps per revolution for the motor
const float anglePerStep = 360.0 / stepsPerRevolution; // Angle moved per step
const int height_per_rotation = 8; // mm

const int chipSelect = 4;

float angle = 0.0;
int z = 0.0;

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
  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  digitalWrite(DIR_PIN, HIGH); 
    
  pinMode(ZDIR_PIN, OUTPUT);
  pinMode(ZSTEP_PIN, OUTPUT);
  digitalWrite(ZDIR_PIN, HIGH); 
}
int MoveUP(int curr_h){
  for (int i = 0; i < stepsPerRevolution; i++) {
        digitalWrite(ZSTEP_PIN, HIGH);
        delayMicroseconds(1000);
        digitalWrite(ZSTEP_PIN, LOW);
        delayMicroseconds(1000);
    }
  return curr_h + height_per_rotation;
}
void loop() {
  for (int i = 0; i < stepsPerRevolution; i++) {
    VL53L0X_RangingMeasurementData_t measure;
    lox.rangingTest(&measure, false);
    if (measure.RangeStatus != 4) {
      int d = measure.RangeMilliMeter;
      DataLogger(d, angle, z);
    } else {
      Serial.println(" out of range ");
    }
      digitalWrite(STEP_PIN, HIGH);
      delayMicroseconds(1000);
      digitalWrite(STEP_PIN, LOW);
      delayMicroseconds(1000);   
      angle += anglePerStep;
  }
  z = MoveUP(z);
  angle = 0.0;
  delay(100);
}

void DataLogger(int distance, float angle, int z){
  File myFile = SD.open("datalog.txt", FILE_WRITE);

  if (myFile) {
    myFile.print(distance);
    myFile.print(",");
    myFile.print(angle);
    myFile.print(",");
    myFile.println(z);
    myFile.close();
  }
  else {
    Serial.println("error opening datalog.txt");
  }
}
