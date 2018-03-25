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
#include <Wire.h>
// Data variables
float pastAltitudes[40];

// Flight status variables
int launch = 0;
int apogee = 0;
int landed = 0;

// Time variables
unsigned long currentMillis, previousMillis = millis();
const unsigned long _INTERVAL = 5;

// SD Variables
File file;

// Pins
const int led1 = 0;
const int led2 = 1;
const int ss = 10;
const int mosi = 11;
const int miso = 12;
const int sck = 13;
const int pr1in = 14;
const int pr1out = 15;
const int pr2in = 16;
const int pr2out = 17;
const int sda = 18;
const int scl = 19;

// Function Prototypes
void setup(void);
void loop(void);
void checkLaunch(void);
void checkApogee(void);
void checkLanded(void);
int getAltitude(void);
int getMagnet(void);
float getAcceleration(void);
String poll(void);
void analyseData(void);

/**
 * Sets up pins and starts interrupt timer
 */
void setup(void) {
  // Set input and output pins
  Serial.begin(500000);
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
  pinMode(10, OUTPUT); // Needed for SD library

  digitalWrite(led1, HIGH);
  digitalWrite(led2, HIGH);

  // Setup I2C
  Wire.begin();
  
  /*
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
  */ // Commented for function testing
}

/**
 * Implements main data collection algorithm
 */
void loop() {
  currentMillis = millis();
  if (currentMillis - previousMillis >= _INTERVAL) {
    // Set time
    previousMillis = currentMillis;
    // Poll data
    String string = poll();
    // Write data
    Serial.print(string);
    /*file.write(string);*/ // Commented for function testing
    
    // Check flight status
    if (!launch) {
      checkLaunch();
    } else if (!apogee) {
      checkApogee();
    } else if (!landed) {
      checkLanded();
    } else if (landed) {
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
  for (int i = 0; i < 39; i++) {
    if ((pastAltitudes[39] - pastAltitudes[i]) > 100) {
      // Rocket has launched
      launch = 1;
      /*file.write("LAUNCH");*/ // Commented for unit testing
    }
  }
}

/**
 * Checks if the rocket has reached apogee
 */
void checkApogee() {
  for (int i = 0; i < 39; i++) {
    if ((pastAltitudes[39] - pastAltitudes[i]) < -5) {
      // Rocket has reached apogee
      apogee = 1;
      /*file.write("APOGEE");*/
    }
  }
}

/**
 * Checks if the rocket has landed
 */
void checkLanded() {
  if ((pastAltitudes[39] - pastAltitudes[0]) == 0) {
    // Rocket has landed
    landed = 1;
    /*file.write("LANDING");*/ // Commented for unit testing
  }
}

/**
 * Returns the most recent altitude and temperature
 */
int getAltitude() {
  Wire.requestFrom(0x68, 5); //0x60
  int temp = 0;
  while(Wire.available()) { // TODO fix blocking
    temp = (temp << 4) + Wire.read();
  }
  return temp;
}

/**
 * Returns the most recent change in magnetic field
 */
int getMagnet() {
  /*Wire.requestFrom(0x0E, 1);
  while(!Wire.available()) {
    continue(); // TODO fix blocking
  }
  return Wire.read();*/ // Commented for unit testing
  return 10;
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
String poll() {
  // Retain only 40 past altitudes
  for (int i = 0; i < 39; i++) {
    pastAltitudes[i] = pastAltitudes[i+1];
  }
  
  // Update altitude and temperature
  int temp = getAltitude();
  pastAltitudes[39] = temp >> 8;
  int temperature = temp & 0xFF;
  
  // Update acceleration
  float acceleration = getAcceleration();
  
  // Update magnetometer
  int magnet = getMagnet();
  
  // Get string to write to SD card
  String string;
  string.concat(minute());
  string.concat(":");
  string.concat(second());
  string.concat(":");
  string.concat(millis() % 1000);
  string.concat(",");
  
  String temperatureString = String(temperature);
  string.concat(temperatureString);
  string.concat(",");
  
  String altitudeString = String(pastAltitudes[39]);
  string.concat(altitudeString);
  string.concat(",");
  
  String accelerationString = String(acceleration);
  string.concat(accelerationString);
  string.concat(",");
  
  String gforceString = String(acceleration/9.80665);
  string.concat(gforceString);
  string.concat(",");

  String magnetString = String(magnet);
  string.concat(magnetString);
  string.concat(",");
  
  string.concat(analogRead(pr1in));
  string.concat(",");
  string.concat(analogRead(pr2in));
  string.concat("\n");
  
  return string;
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

