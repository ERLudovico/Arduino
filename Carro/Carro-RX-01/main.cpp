#include <Arduino.h>
#include "Wire.h"
#include <ServoIn.h>
#include <Timer1.h>

/**************
 * DEFINICOES
 **************/
// DEBUG
#define debugSerial                true
#define debugMotores               true
#define debugLed                   false
// Definicao de Hardware

// RC
#define SERVOS 2

/**************
 * VARIAVEIS
 **************/
/* Serial*/
extern HardwareSerial Serial;

/* RC */
int throttleValue = 0;
int steeringValue = 0;
int offsetTorqueLeft = 15;
int offsetTorqueRight = -15;
uint16_t g_values[SERVOS]; // output buffer for ServoIn
uint8_t g_workIn[SERVOIN_WORK_SIZE(SERVOS)]; // we need to have a work buffer for the ServoIn class

const int steeringPin = 8;
const int throttlePin = 9;

//const int throttlePin = 12;
//const int steeringPin = 13;
static uint8_t lastB = 0;

rc::ServoIn g_ServoIn(g_values, g_workIn, SERVOS);

/* Motor */

int drive = 0;
int motorSpeed = 0;
int torqueLeft = 0;
int torqueRight = 0;

const int enMAPin = 6; //11; // H-Bridge enable pin
const int in1Pin = 7; //10; // H-Bridge input pins
const int in2Pin = 5; //9;

const int enMBPin = 3; // H-Bridge enable pin
const int in3Pin = 2; // 5; // H-Bridge input pins
const int in4Pin = 4; //6;

const int margemErro = 10;


/* deltaT */

/**************
 * FUNCOES
 **************/

void stop();
void toBack();
void toAhead();
void setThrottle(int torqueLeft, int torqueRight);
void motorDriver(int torqueLeft, int torqueRight);
void driveACar();

void setup() {
    Wire.begin();
    rc::Timer1::init(); // RC

    if (debugSerial) {
        Serial.begin(115200);
        Serial.println(F("===================  SETUP ================="));
    }

    if (debugLed) {
        pinMode(13, OUTPUT);
        digitalWrite(13, HIGH);
    }
    /* Motores */
    pinMode(in1Pin, OUTPUT);
    pinMode(in2Pin, OUTPUT);
    pinMode(in3Pin, OUTPUT);
    pinMode(in4Pin, OUTPUT);
    pinMode(enMAPin, OUTPUT);
    pinMode(enMBPin, OUTPUT);
    /***********/

    /* RC */
    pinMode(throttlePin, INPUT);
    pinMode(steeringPin, INPUT);
    //    pinMode(throttlePin, INPUT);
    //    pinMode(steeringPin, INPUT);

    // only allow pin change interrupts for PB0-3 (digital pins 8-11)
    PCMSK0 = (1 << PCINT0) | (1 << PCINT1) | (0 << PCINT2) | (0 << PCINT3);


    // enable pin change interrupt 0
    PCICR = (1 << PCIE0);

    // start listening
    g_ServoIn.start();


    /***********/
    if (debugSerial) {
        Serial.println(F("=============== FIM  SETUP ================="));
    }

}

void loop() {

    driveACar();
//    toAhead();
//    for (int i = 0; i < 150; i = i + 10) {
//        
//        setThrottle(i, i);
//        delay(1000);
//    }
    
    //        motorDriver(100);
    //        delay(1000);
    //    
    // update incoming values
    //    	g_ServoIn.update();
    //	Serial.print(g_values[0]);
    //        Serial.print(" - ");
    //	Serial.println(g_values[1]);
    //	Serial.print(" - ");
    //        Serial.print(g_values[2]);
    //        Serial.print(" - ");	
    //        Serial.println(g_values[3]);
    delay(100);

}

void driveACar() {
    g_ServoIn.update();
    steeringValue = map(g_values[0], 1000, 2000, -230, 230);
    throttleValue = map(g_values[1], 1000, 2000, -230, 230);
    if (debugMotores && debugSerial) {
        Serial.print(F("steeringValue: "));
        Serial.print(steeringValue);
        Serial.print(F(" throttleValue: "));
        Serial.println(throttleValue);
    }
    if (abs(throttleValue) < margemErro) {
        stop();
    } else {
        if (throttleValue > 0) {
            toAhead();
        } else {
            toBack();
        }
        if (abs(steeringValue) < margemErro) {
            // Rodas alinhada            
            setThrottle(throttleValue, throttleValue);
        } else {
            if (steeringValue > 0) { // Direita                                                                
                int compThrottleValue = map (steeringValue,0,230,0,50);
                
                setThrottle(throttleValue, throttleValue - compThrottleValue);
            } else { // Esquerda                
                int compThrottleValue = map (steeringValue,-230,0,0,50);
                
                setThrottle(throttleValue - compThrottleValue, throttleValue);
            }
        }
    }
}

/* MOTOR */

//void motorDriver(int torqueLeft, int torqueRight) {
//    int torqueMedia = (torqueLeft + torqueRight ) / 2 ;
//    if (debugMotores && debugSerial) {
//        Serial.print("Torques: ")
//        Serial.print(abs(torqueLeft));
//        Serial.print(" - ")
//        Serial.print(abs(torqueRight));
//    }
//    if (torqueMedia > -margemErro && torqueMedia < margemErro) {
//        stop();
//    } else {
//        (torqueMedia >= 0) ? toAhead() : toBack(); // Definir direcao
//        torque = abs(torque); // Mapeio o torque
//        torque = map(torque, margemErro, 255, 150, 255);
//        
//        setThrottle(torqueLeft, torqueRight);
//    }
//
//}

void setThrottle(int torqueLeft, int torqueRight) {

    int absTorqueLeft = abs(torqueLeft) + offsetTorqueLeft;
    int absTorqueRight = abs(torqueRight) + offsetTorqueRight;

    if (debugMotores && debugSerial) {
        Serial.print(F("absTorqueLeft: "));
        Serial.print(absTorqueLeft);
        Serial.print(F(" absTorqueRight: "));
        Serial.println(absTorqueRight);
    }
    analogWrite(enMAPin, absTorqueLeft);
    analogWrite(enMBPin, absTorqueRight);
}

void toAhead() {
    if (debugMotores && debugSerial) Serial.println(F("__FRENTE__"));
    // Ajusta motores a FRENTE  
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, HIGH);
    digitalWrite(in3Pin, LOW);
    digitalWrite(in4Pin, HIGH);
}

void toBack() {
    if (debugMotores && debugSerial) Serial.println(F("__RE__"));
    // Ajusta motores a RE
    digitalWrite(in1Pin, HIGH);
    digitalWrite(in2Pin, LOW);
    digitalWrite(in3Pin, HIGH);
    digitalWrite(in4Pin, LOW);
}

void stop() {
    if (debugMotores && debugSerial) Serial.println(F("__PARADO__"));
    // Para moters
    analogWrite(enMAPin, 0);
    analogWrite(enMBPin, 0);
}

/* Receptor */
ISR(PCINT0_vect) {
    // we need to call the ServoIn ISR here, keep code in the ISR to a minimum!
    uint8_t newB = PINB;
    uint8_t chgB = newB ^ lastB; // bitwise XOR will set all bits that have changed
    lastB = newB;

    // has any of the pins changed?
    if (chgB) {
        // find out which pin has changed
        if (chgB & _BV(0)) {
            g_ServoIn.pinChanged(0, newB & _BV(0));
        }
        if (chgB & _BV(1)) {
            g_ServoIn.pinChanged(1, newB & _BV(1));
        }
        //        if (chgB & _BV(2)) {
        //            g_ServoIn.pinChanged(2, newB & _BV(2));
        //        }
        //        if (chgB & _BV(3)) {
        //            g_ServoIn.pinChanged(3, newB & _BV(3));
        //        }
    }
}