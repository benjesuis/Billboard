/*
 * Boston University Rocket Propulsion Group HPR Team 3
 * To be used for data collection on Project Platypus
 *
 * Created: Jan 10, 2018
 * Author: Glenn Xavier
 */

#include <TimerOne.h>
#include <SD.h>
#include <TimeLib.h>
#include <Streaming.h>

// Data variables
float pastAltitudes[40];

// Flight status variables
int launch = 0;
int apogee = 0;
int landed = 0;

// Time variables
unsigned long currentMillis, previousMillis = millis();
unsigned long interval = 20000;

// SD Variables
File file;

// Pins
const int led = 1;
const int altimeter = 2;
const int magnetometer = 3;
const int photoresistor1 = 5;
const int photoresistor2 = 6;
const int slaveSelect = 4; // Must be 4
const int errorLed = 7;

/**
 * Sets up pins and starts interrupt timer
 */
void setup(void) {
  // Set input and output pins
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);
  pinMode(altimeter, INPUT);
  pinMode(magnetometer, INPUT);
  pinMode(photoresistor1, INPUT);
  pinMode(photoresistor2, INPUT);

  // Keep error LED on until algorithm runs
  pinMode(errorLed, OUTPUT);
  digitalWrite(errorLed, HIGH);
  
  // Setup SD
  pinMode(SS, OUTPUT); // Pin 10 must be MOSI pin
  while(!SD.begin(slaveSelect)) {
    // Waits for SD card to be inserted
    delay(100);
  }
  
  // Increments filename if filename has been taken
  int count = 0;
  char filename[10];
  do {
    strcpy(filename, "");
    strcat(filename, "log");
    strcat(filename, count);
    strcat(filename, ".txt");
    count++;
  } while (SD.exists(filename));
  
  // Create new log.txt file
  file = SD.open(filename, FILE_WRITE);
}

/**
 * Implements main data collection algorithm
 */
void loop() {
  //Turn on error LED
  digitalWrite(errorLed, HIGH);
  
  currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // Set time
    previousMillis = currentMillis;
    // Poll data
    char* string = poll();
    // Write data
    file.write(string);
    // Turn off error LED
    digitalWrite(errorLed, LOW);
    
    // Check flight status
    if (!launch) {
      checkLaunch();
    } else if (!apogee) {
      checkApogee();
    } else if (!landed) {
      checkLanded();
    } else {
      // Save and close log.txt file
      file.close();
      // Do data analysis
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
  return (((pastAltitudes[37] - pastAltitudes[36])/0.05) - ((pastAltitudes[39] - pastAltitudes[38])/0.05)) / 0.05;
}

/**
 * Polls data from sensors, then generates string to write to SD card
 */
char* poll() {
  // Retain only 40 past altitudes
  for (int i = 0; i < 39; i++) {
    pastAltitudes[i] = pastAltitudes[i+1];
  }
  
  // Update altitude
  pastAltitudes[39] = digitalRead(altimeter); // TODO Correct implementation of altimeter
  float acceleration = getAcceleration();
  
  // Get string to write to SD card
  char string[100];
  strcat(string, hour());
  strcat(string, ",");
  strcat(string, minute());
  strcat(string, ",");
  strcat(string, second());
  strcat(string, ",");
  strcat(string, millis() % 1000);
  strcat(string, ",");
  
  char altitudeString[10];
  dtostrf(pastAltitudes[39], 10, 3, altitudeString);
  strcat(string, altitudeString);
  strcat(string, ",");
  
  char accelerationString[10];
  dtostrf(acceleration, 10, 3, accelerationString);
  strcat(string, accelerationString);
  strcat(string, ",");
  
  char gforceString[10];
  dtostrf(acceleration/9.80665, 10, 3, gforceString);
  strcat(string, gforceString);
  strcat(string, ",");
  
  strcat(string, digitalRead(magnetometer));
  strcat(string, ",");
  strcat(string, analogRead(photoresistor1));
  strcat(string, ",");
  strcat(string, analogRead(photoresistor2));
  strcat(string, "\n");
  return string;
}

/**
 * Writes string to SD card
 */
void writeToSD(char* string) {
  file.println(string);
  file.flush();
}

/**
 * Analyses flight data and writes results to SD card
 */
void analyseData() {
  // Turn on error LED
  digitalWrite(errorLed, LOW);
  
  // Increments filename if filename has been taken
  int count = 0;
  char filename[10];
  do {
    strcpy(filename, "");
    strcat(filename, "data");
    strcat(filename, count);
    strcat(filename, ".txt");
    count++;
  } while (SD.exists(filename));
  
  // Create new data.txt file
  file = SD.open(filename, FILE_WRITE);

  // Turn off error LED
  digitalWrite(errorLed, LOW);

  // TODO add data analysis formulae
  
  // Save and close data.txt file
  file.close();
}

