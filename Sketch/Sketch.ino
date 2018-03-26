/*
 * Boston University Rocket Propulsion Group HPR Team 3
 * To be used for data collection on Project Platypus
 *
 * Created: Jan 10, 2018
 * Author: Glenn Xavier
 * Helper: Ben Livney
 */

#include <SD.h>
#include <TimeLib.h>
#include <Wire.h>

// Data variables
float pastAltitudes[40];
float magSum;
int magCount;

// Flight status variables
int launch = 0;
int apogee = 0;
int landed = 0;

// Time variables
unsigned long currentMillis, previousMillis = millis();
const unsigned long _INTERVAL = 20;

// SD Variables
File file;

// pins
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

//<<<<<<< HEAD
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
float chapmanFerraro(float);

//=======
//>>>>>>> parent of 51459cd... Added function prototypes
/**
   Sets up pins and starts interrupt timer
*/
void setup(void) {
  magSum = 0;
  magCount = 0;
  // Set input and output pins
  Serial.begin(9600);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(ss, OUTPUT);
  pinMode(mosi, OUTPUT);
  pinMode(miso, INPUT);
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


  // Setup SD
  while (!SD.begin(ss)) {
    // Waits for SD card to be inserted
    delay(100);
  }

  // Increments filename if filename has been taken
  int count = 0;
  String filename = "DATA0.txt";
  while (SD.exists(filename)) {
    filename = "DATA";
    filename.concat(String(count));
    filename.concat(".txt");
    count++;
  }

  // Create new log.txt file
  file = SD.open(filename, FILE_WRITE);
  // Commented for function testing
}

/**
   Implements main data collection algorithm
*/
void loop() {
  currentMillis = millis();
  if (currentMillis - previousMillis >= _INTERVAL) {
    // Set time
    previousMillis = currentMillis;
    // Poll data
    String string = poll();
    // Write data
    char line[50];
    string.toCharArray(line, 50);
    Serial.print(line);
    file.write(line); // Commented for function testing

    // Check flight status
    if (!launch) {
      checkLaunch();
    } else if (!apogee) {
      checkApogee();
    } else if (!landed) {
      checkLanded();
    } else if (landed) {
      // Do data analysis
      analyseData();
    }
  }
}

/**
   Checks if the rocket has launched
*/
void checkLaunch() {
  for (int i = 0; i < 39; i++) {
    if ((pastAltitudes[39] - pastAltitudes[i]) > 100 || second() > 0) {
      // Rocket has launched
      launch = 1;
      file.write("LAUNCH\r\n"); // Commented for unit testing
      Serial.println("LAUNCH");
      break; //delete this later
    }
  }
}

/**
   Checks if the rocket has reached apogee
*/
void checkApogee() {
  for (int i = 0; i < 39; i++) {
    if ((pastAltitudes[39] - pastAltitudes[i]) < -5 || second() > 1) {
      // Rocket has reached apogee
      apogee = 1;
      file.write("APOGEE\r\n");
      Serial.println("APOGEE");
      break; //delete this later
    }
  }
}

/**
   Checks if the rocket has landed
*/
void checkLanded() {
  if ((pastAltitudes[39] - pastAltitudes[0]) == 0) {
    // Rocket has landed
    landed = 1;
    file.write("LANDING\r\n"); // Commented for unit testing
    Serial.println("LANDING");
  }
}

/**
   Returns the most recent altitude and temperature
*/
int getAltitude() {
  Wire.requestFrom(0x68, 5); //0x60
  int temp = 0;
  while (Wire.available()) { // TODO fix blocking
    temp = (temp << 4) + Wire.read();
  }
  return temp;
}

/**
   Returns the most recent change in magnetic field
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
   Returns the acceleration for the 4 most recent altitudes
*/
float getAcceleration() {
  return (((pastAltitudes[37] - pastAltitudes[36]) / 0.05) - ((pastAltitudes[39] - pastAltitudes[38]) / 0.05)) / 0.05;
}

/**
   Polls data from sensors, then generates string to write to SD card
*/
String poll() {
  // Retain only 40 past altitudes
  for (int i = 0; i < 39; i++) {
    pastAltitudes[i] = pastAltitudes[i + 1];
  }

  // Update altitude and temperature
  int temp = getAltitude();
  pastAltitudes[39] = temp >> 8;
  int temperature = temp & 0xFF;

  // Update acceleration
  float acceleration = getAcceleration();

  // Update magnetometer
  int magnet = getMagnet();
  magSum = magSum + magnet;
  magCount++;

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

  String gforceString = String(acceleration / 9.80665);
  string.concat(gforceString);
  string.concat(",");

  String magnetString = String(magnet);
  string.concat(magnetString);
  string.concat(",");

  string.concat(analogRead(pr1in));
  string.concat(",");
  string.concat(analogRead(pr2in));
  string.concat("\r\n");

  return string;
}

/**
   Analyses flight data and writes results to SD card
*/
void analyseData() {
  // Data analysis formulae
  float aveMag = magSum / magCount;
  float magnetofloat = chapmanFerraro(aveMag);
  String magnetopause = String(magnetofloat);
  String header = "magnetopause altitude: ";
  header.concat(magnetopause);
  char string[50];
  header.toCharArray(string, 50);
  Serial.println(string);
  file.write(string);
  // Save and close data.txt file
  file.close();
  //Don't do anything else
  while (1);
}

float chapmanFerraro(float bField) {
  //bField should be in Tesla, not microTesla
  float R = 6.738 * pow(10, 6),
        mu = 4 * PI, // * pow(10, -7),
        rho = 1.12, // * pow(10, -20),
        v2 = 1.6; // * pow(10, 11);
  int power = (-6 - 6 + 7 + 20 - 11);
  float base = (2 * bField * bField) / (mu * rho * v2);
  float r = R * pow(base * pow(10, power), 1.0 / 6.0);
  return r;
}

