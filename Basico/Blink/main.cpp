//#define __AVR_ATmega328P__
#include <Arduino.h>
#include <PString.h>
//#include <SoftwareSerial.h>
//#include <Serial.java>
//#include <CommPort.java>
//#include <RXTXcomm.jar>
//#include <RXTXCommDriver.java>
#define SECS 86400

extern HardwareSerial Serial;

void setup() {
  
    
  char buffer[40];
  //Serial.begin(115200);
  delay(2000); 
  Serial.begin(115200); 
  // There are two main ways to use a PString.
  // First, the "quickie" way, simply renders a single value into a buffer
  PString(buffer, sizeof(buffer), PI); // print the value of PI into the buffer
  Serial.println(buffer); // do whatever you want with "buffer" here.
// Other "quickie" examples:
  PString(buffer, sizeof(buffer), "Printing strings");
  PString(buffer, sizeof(buffer), SECS);
  PString(buffer, sizeof(buffer), SECS, HEX);
  Serial.println(buffer);
  // You can also created a (named) PString object and operate
  // on it exactly as you would with Serial or LiquidCrystal
  PString str(buffer, sizeof(buffer));
  str.print("The value of PI is ");
  str.print(PI);
  // At this point, buffer contains "The value of PI is 3.14.."
  // You can get to the data directly...
  Serial.println(buffer);
  // ... or indirectly
  Serial.println(str);
  
  // There are a couple of useful member functions:
  Serial.print("The string's length is ");
  Serial.println(str.length());
  Serial.print("Its capacity is ");
  Serial.println(str.capacity());
  
  // You can reuse a PString with the begin() function
  str.begin();
  str.print("Hello, world!");
  Serial.println(str);
  
  // Or accomplish the same thing with the assignment operator:
  str = "Goodbye, cruel";
  
  // PStrings support the concatenation operator 
  str += " world!";
  Serial.println(str);
  
  // And you can also check for equivalance
  if (str == "Goodbye, cruel world!")
    Serial.println("Yes, alas, goodbye indeed");
}

void loop() {
    
}

// stty -F /dev/ttyS0 9600 <-- Configura
// cmd /c mode com11       <-- Verifica
// cat /dev/ttyS10 (com11) <-- Observa