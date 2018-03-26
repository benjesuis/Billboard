#include <SD.h>
#include <SPI.h>

File file;
const int ss = 10;
const int mosi = 11;
const int miso = 12;
const int sck = 13;

void setup() {
  // put your setup code here, to run once
  Serial.begin(9600);
  Serial.println("test");
  pinMode(ss, OUTPUT);
  pinMode(mosi,OUTPUT);
  pinMode(miso,INPUT);
  pinMode(sck, OUTPUT);
  while(!SD.begin(ss)) {
    // Waits for SD card to be inserted
    delay(100);
    Serial.println("Waiting...");
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
  Serial.println(filename);

  file = SD.open(filename, FILE_WRITE);
  file.write("Testing...");
  //file.write(filename);
  file.close();
  Serial.println("done");
}

void loop() {
  // put your main code here, to run repeatedly:
  
}
