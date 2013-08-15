#include <Arduino.h>
/* Este código é do SLAVE (Receptor)
 * A ideia deste código é testar a comunicacao serial entre
 * dois dispositivos seriais: 
 * O primeiro vai conectado ao arduino, ao tera este código executando
 * O segundo vai diretamente conectado ao PC e irá printar a letra a
 */
extern HardwareSerial Serial;

int led = 13;
char inChar; // Where to store the character read

void setup() {
    Serial.begin(19200); // opens serial port, sets data rate to 9600 bps
    pinMode(led, OUTPUT);
}

void loop() {
    while (Serial.available() > 0) // Don't read unless
        // there you know there is data
    {
        inChar = Serial.read();

        switch (inChar) {
            case 'a':
                digitalWrite(led, HIGH); // turn the LED on (HIGH is the voltage level)
                delay(200); // wait for a second
                digitalWrite(led, LOW); // turn the LED off by making the voltage LOW
                Serial.print(inChar);
                break;
        }
    }
}

// stty -F /dev/ttyS0 9600 <-- Configura
// cmd /c mode com11       <-- Verifica
// cat /dev/ttyS10 (com11) <-- Observa