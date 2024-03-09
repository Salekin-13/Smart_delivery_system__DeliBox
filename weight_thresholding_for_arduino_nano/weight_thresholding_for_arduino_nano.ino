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
  // Serial.println("HX711 Demo");
  // Serial.println("Initializing the scale");

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  attachInterrupt(digitalPinToInterrupt(tarePin), tare, FALLING);
  // Serial.println("Before setting up the scale:");
  // Serial.print("read: \t\t");
  // Serial.println(scale.read());      // print a raw reading from the ADC

  // Serial.print("read average: \t\t");
  // Serial.println(scale.read_average(20));   // print the average of 20 readings from the ADC

  // Serial.print("get value: \t\t");
  // Serial.println(scale.get_value(5));   // print the average of 5 readings from the ADC minus the tare weight (not set yet)

  // Serial.print("get units: \t\t");
  // Serial.println(scale.get_units(5), 1);  // print the average of 5 readings from the ADC minus tare weight (not set) divided
  //           // by the SCALE parameter (not set yet)
            
  scale.set_scale(349.3);
  //scale.set_scale(-471.497);                      // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();               // reset the scale to 0

  // Serial.println("After setting up the scale:");

  // Serial.print("read: \t\t");
  // Serial.println(scale.read());                 // print a raw reading from the ADC

  // Serial.print("read average: \t\t");
  // Serial.println(scale.read_average(20));       // print the average of 20 readings from the ADC

  // Serial.print("get value: \t\t");
  // Serial.println(scale.get_value(5));   // print the average of 5 readings from the ADC minus the tare weight, set with tare()

  // Serial.print("get units: \t\t");
  // Serial.println(scale.get_units(5), 1);        // print the average of 5 readings from the ADC minus tare weight, divided
  //           // by the SCALE parameter set with set_scale

  // Serial.println("Readings:");
}

void loop() {
  // Serial.print("one reading:\t");
  // Serial.print(scale.get_units(), 1);
  // Serial.print("\t| average:\t");
  reading = scale.get_units(20);
  reading = abs(reading);
  if(reading > 100)
    {
    digitalWrite(outputPin, LOW);
    weightDetect = 1;
    }
  else 
  {
    weightDetect = 0;
   digitalWrite(outputPin, HIGH);
  }
  // if(reading < lightWeight)
  //   voltage = 100;
  // else if(reading > lightWeight && reading < heavyWeight)
  //   voltage = 300;
  // else
  //   voltage = 650;

  // voltage = map(reading, 0, 5000, 0,650);
  //voltage = constrain(voltage, 0,650);
  //analogWrite(outputPin, voltage);

  if(weightDetect)
  {
    Serial.print("Weight: ");
    Serial.print(reading);
    Serial.println("\t Parcel Paisi!");
  // Serial.println(voltage);
  }
  else {
      Serial.print("Weight: ");
    Serial.println(reading);
  }


  delay(100);
}

void tare()
{
  scale.tare();
  Serial.println("Interrupt Tare");
}

