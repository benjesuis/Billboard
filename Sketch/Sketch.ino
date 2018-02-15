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
const int led1 = 0;
const int led2 = 1;
const int ss = 10; // Apparently needs to be pin 4?
const int mosi = 11;
const int miso = 12;
const int sck = 13;
const int pr1in = 14;
const int pr1out = 15;
const int pr2in = 16;
const int pr2out = 17;
const int sda = 18;
const int scl = 19;

/**
 * Sets up pins and starts interrupt timer
 */
void setup(void) {
  // Set input and output pins
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(ss, OUTPUT);
  pinMode(mosi,OUTPUT);
  pinMode(miso,INPUT);
  pinMode(sck, OUTPUT);
  pinMode(pr1in, INPUT);
  pinMode(pr1out, OUTPUT);
  pinMode(pr2in, INPUT);
  pinMode(pr2out, OUTPUT);
  pinMode(sda, OUTPUT);
  pinMode(scl, OUTPUT);

  digitalWrite(led, HIGH);
  digitalWrite(led, HIGH);

  // Setup SD
  while(!SD.begin(ss)) {
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
  
  currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // Set time
    previousMillis = currentMillis;
    // Poll data
    char* string = poll();
    // Write data
    file.write(string);
    
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
  strcat(string, minute());
  strcat(string, ":");
  strcat(string, second());
  strcat(string, ":");
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

  // TODO add data analysis formulae
  
  // Save and close data.txt file
  file.close();
}

