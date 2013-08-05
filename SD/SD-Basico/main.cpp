#include <Arduino.h>
#include <SoftwareSerial.h>
#include <SD.h>

#define CS      10

char* fileName = {"log.txt"};

File dataFile;
extern HardwareSerial Serial;

void setup() {

    Serial.begin(115200);

    pinMode(CS, OUTPUT);
    SD.begin(CS);
    Serial.println("Gravando Arquivo!");
    dataFile = SD.open(fileName, FILE_WRITE);
    dataFile.println("Ola!");
    dataFile.close();
    Serial.println("Arquivo Gravado!");
        
}

void loop() {
    
}
