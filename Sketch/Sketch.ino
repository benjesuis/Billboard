/*
 * Boston University Rocket Propulsion Group HPR Team 3
 * To be used for data collection on Project Platypus
 *
 * Created: Jan 10, 2018
 * Author: Glenn Xavier
 */

#include <TimerOne.h>
#include <SD.h>
#include <Time.h>
#include <TimeLib.h>

// Data variables
float pastAltitudes[40];

// Flight status variables
int launch = 0;
int apogee = 0;
int landed = 0;

// Time variables
unsigned long currentMillis = previousMillis = millis();
unsigned long interval = 20000;

// SD Variables
File file = false;

// Pins
const int led = ;
const int altimeter = ;
const int magnetometer = ;
const int photoresistor1 = ;
const int photoresistor2 = ;
const int slaveSelect = 4;
const int errorLed = ;

/**
 * Sets up pins and starts interrupt timer
 */
int setup (void) {
  // Set input and output pins
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);
  pinMode(errorLed, OUTPUT);
  pinMode(altimeter, INPUT);
  pinMode(magnetometer, INPUT);
  pinMode(photoresistor1, INPUT);
  pinMode(photoresistor2, INPUT);
  
  // Setup SD
  pinMode(SS, OUTPUT); // Pin 10 must be MOSI pin
  while(!SD.begin(slaveSelect)) {
    // Waits for SD card to be inserted
    delay(100);
  }
  
  // Increments filename if filename has been taken
  int count = 0;
  while (exists("log" + String(count) + ".txt")) {
    count++;
  }
  // Create new log.txt file
  file = SD.open("log" + count + ".txt", FILE_WRITE); 
}

/**
 * Implements main data collection algorithm
 */
void loop() {
  currentMillis = millis();
  if (currentMillis - previousMillis() >= interval) {
    // Set time
    previousMillis = currentMillis;
    // Poll data
    String str = poll()
    // Write data
    write(str);
    // Check flight status
    if (!launch) {
      checkLaunch();
    } else if (!apogee) {
      checkApogee();
    } else if (!landed) {
      checkLanded();
    } else {
      // Rocket has landed. Do data analysis
      analyseData();
    }
  }
}

/**
 * Checks if the rocket has launched
 */
void checkLaunch() {
  int flag = 0;
  for (int i = 0; i < 39; i++) {
    if ((pastAltitudes[39] - pastAltitudes[i]) > 100) {
      // Rocket has launched
      flag = 1;
      writeToSD("LAUNCH");
    }
  }
  launch = flag;
}

/**
 * Checks if the rocket has reached apogee
 */
void checkApogee() {
  int flag = 0;
  for (int i = 0; i < 39; i++) {
    if ((pastAltitudes[39] - pastAltitudes[i]) < -5) {
      // Rocket has reached apogee
      flag = 1;
      writeToSD("APOGEE");
    }
  }
  apogee = flag;
}

/**
 * Checks if the rocket has landed
 */
void checkLanded() {
  int flag = 0;
  if ((pastAltitudes[39] - pastAltitudes[0]) == 0) {
    // Rocket has landed
    flag = 1;
    writeToSD("LANDING");
  }
  landed = flag;
}

/**
 * Returns the acceleration for the 4 most recent altitudes
 */
float getAcceleration() {
  return ((pastAltitudes[37] - pastAltitudes[36])/0.05) - (pastAltitudes[39] - pastAltitudes[38])/0.05)) / 0.05;
}

/**
 * Polls data from sensors, then generates String to write to SD card
 */
String poll(float data[]) {
  // Retain only 40 past altitudes
  for (int i = 0; i < 39; i++) {
    pastAltitudes[i] = pastAltitudes[i+1];
  }
  
  // Update altitude
  pastAltitudes[39] = digitalRead(altimeter);
  float acceleration = getAcceleration();
  
  // Get String to write to SD card
  String str = String(hour()) + ":" + String(minute()) + ":" + String(second()) + ":" + String(millis()) + "," + String(altitude)
    + "," + String(acceleration)+ "," + String(acceleration/9.80665) + "," + String(digitalRead(magnetometer)) + "," + String(analogRead(photoresistor1))
    + "," + String(analogRead(photoresistor2));
  return str;
}

/**
 * Writes string to SD card
 */
void writeToSD(String string) {
  file.println(string);
  flush();
}

/**
 * Analyses flight data and writes results to SD card
 */
void analyseData() {
  // TODO implement analyseData();
}

