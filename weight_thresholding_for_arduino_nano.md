#include <Arduino.h>
#include "HX711.h"

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = A4;
const int LOADCELL_SCK_PIN = A5;

#define lightWeight 500
#define moderateWeight 1500
#define heavyWeight 3000

#define lightWeightVoltage 500
#define moderateWeight 1500
#define heavyWeight 3000

#define weightThreshold 100
int weightDetect = 0;

long reading = 0;
int weight = 0;
int voltage = 0;
int tarePin = 3;
#define outputPin 10
HX711 scale;

void setup() {
  Serial.begin(57600);
  pinMode(outputPin, OUTPUT);
  pinMode(tarePin, INPUT_PULLUP);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  attachInterrupt(digitalPinToInterrupt(tarePin), tare, FALLING);
  scale.set_scale(349.3);
  scale.tare();
}

void loop() {
  reading = scale.get_units(20);
  reading = abs(reading);
  if (reading > 100) {
    digitalWrite(outputPin, LOW);
    weightDetect = 1;
  } else {
    weightDetect = 0;
    digitalWrite(outputPin, HIGH);
  }
  if (weightDetect) {
    Serial.print("Weight: ");
    Serial.print(reading);
    Serial.println("\t Parcel Paisi!");
  } else {
    Serial.print("Weight: ");
    Serial.println(reading);
  }
  delay(100);
}

void tare() {
  scale.tare();
}
