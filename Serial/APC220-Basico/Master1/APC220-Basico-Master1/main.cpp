
#include <Arduino.h>

extern HardwareSerial Serial;

void setup() {
  
    
  Serial.begin(19200); 
  pinMode(13,OUTPUT);
}

void loop() {
  Serial.println("a");
  
  digitalWrite(13,HIGH);
  delay(500);
  digitalWrite(13,LOW);
  delay(500);
}

// stty -F /dev/ttyS0 9600 <-- Configura
// cmd /c mode com11       <-- Verifica
// cat /dev/ttyS10 (com11) <-- Observa