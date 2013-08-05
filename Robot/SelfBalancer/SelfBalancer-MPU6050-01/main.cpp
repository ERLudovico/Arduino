#include <Arduino.h>
#include "Wire.h"
#include "I2Cdev.h"
#include "MPU6050.h"

/**************
 * DEFINICOES
 **************/
// DEBUG
#define debugSerial                true
#define debugMPU6050               true
#define debugLoopTime              false
#define debugLed                   false
#define debugPID                   true
#define debugMotores               true
// Definicao de Hardware
#define useMPU6050                 true
#define GUARD_GAIN               20.0  // PID

MPU6050 accelgyro(0x69);

/**************
 * VARIAVEIS
 **************/
/* Serial*/
extern HardwareSerial Serial ;

/*
 * DATASHEET MPU6050
 *
 * 2g  = 0.06 mg/LSB
 * 4g  = 0.12
 * 8g  = 0.24
 * 16G = 0.49
 * Leitura em 16Bits
 */

int offSet = 0 ;
int16_t ax, ay, az;
int16_t gx, gy, gz;

double accXangle; // Angle calculate using the accelerometer
double accYangle;
double gyroXangle = 180; // Angle calculate using the gyro
double gyroYangle = 180;
double compAngleX = 180; // Calculate the angle using a Kalman filter
double compAngleY = 180;

/* PID */

float K = 1.4;
int   Kp = 3;                      
int   Ki = 1;                   
int   Kd = 6; // 6 
int error = 0;
int last_error = 0;
int integrated_error = 0;
int pTerm = 0, iTerm = 0, dTerm = 0;
double Input, Output , setPoint;

/* Motor */

int drive = 0;
int motorSpeed = 0 ;
const int enMAPin = 11; // H-Bridge enable pin
const int in1Pin = 10; // H-Bridge input pins
const int in2Pin = 9;

const int enMBPin = 3; // H-Bridge enable pin
const int in3Pin = 5; // H-Bridge input pins
const int in4Pin = 6;

const int margemErro = 1 ;


/* deltaT */
int STD_LOOP_TIME           = 9;  
int lastLoopTime            = STD_LOOP_TIME;
int lastLoopUsefulTime      = STD_LOOP_TIME;
unsigned long loopStartTime = 0;
int deltaT                  = 0;

/**************
 * FUNCOES
 **************/

void pare();
void re();
void frente();
void acelera(int torque);
void motorDriver(int torque);
int calcDeltaT();
int updatePid(int targetPosition, int currentPosition) ;
double atualizaSensors(int deltaT);


void setup() {
  Wire.begin();  

  if (debugSerial){
    Serial.begin(115200);
    Serial.println(F("===================  SETUP ================="));
  } 

  // initialize device
  if (debugSerial && debugMPU6050) Serial.println(F("Initializing I2C devices..."));
  accelgyro.initialize();

  // verify connection
  if (debugSerial && debugMPU6050) {
    Serial.println("Testing device connections...");
    boolean OK = accelgyro.testConnection() ;
    ( OK )? 
    Serial.println(F("MPU6050 connection successful")): 
    Serial.println(F("MPU6050 connection failed"));
  }
  if ( debugLed ){
    pinMode(13,OUTPUT);
    digitalWrite(13, HIGH); 
  }
  /* PID */
  setPoint = 4 ;
  /********/


  /* Motores */
  pinMode(in1Pin, OUTPUT);
  pinMode(in2Pin, OUTPUT);
  pinMode(in3Pin, OUTPUT);
  pinMode(in4Pin, OUTPUT);
  pinMode(enMAPin, OUTPUT);
  pinMode(enMBPin, OUTPUT);
  /***********/

  if (debugSerial){
    Serial.println(F("=============== FIM  SETUP ================="));
  } 

}

void loop() {
      if (debugSerial && debugLoopTime) Serial.println(lastLoopTime);
  
  Input = atualizaSensors(deltaT);
  motorSpeed = updatePid(setPoint, floor(Input));  
  //motorSpeed = map(floor(Input),-90,90, 
  //motorDriver(motorSpeed);
  ( Input>(-40) && Input<(40) ) ? motorDriver(motorSpeed) : motorDriver(0); 
  // *********************** loop timing control **************************
  deltaT = calcDeltaT();
}


int calcDeltaT(){
  lastLoopUsefulTime = millis()-loopStartTime;
  if(lastLoopUsefulTime<STD_LOOP_TIME) { 
    delay(STD_LOOP_TIME-lastLoopUsefulTime);
    if (debugSerial && debugLoopTime){
      if ( debugLed ){
        digitalWrite(13, HIGH); 
        digitalWrite(13, LOW); 
      } 
      else { 
        Serial.print("  Esperei: ");
        Serial.println(STD_LOOP_TIME-lastLoopUsefulTime);
      }
    }
  }
  lastLoopTime = millis() - loopStartTime;
  loopStartTime = millis(); 
  return lastLoopTime;
}

/* MOTOR */
void motorDriver(int torque){

  if (debugMotores && debugSerial) Serial.println(abs(torque));

  if ( torque > -margemErro && torque < margemErro ) {
    pare();
  } 
  else {
    ( torque >= 0) ? frente(): re(); // Definir direcao
    torque = abs(torque); // Mapeio o torque
    torque = map(torque,margemErro,255,150,255);
    acelera(torque);
  }

}

void acelera(int torque){
  analogWrite(enMAPin, torque + 25);
  analogWrite(enMBPin, torque);

}

void frente(){
  if (debugMotores && debugSerial) Serial.print(F(" : FRENTE : "));
  // Ajusta motores a FRENTE  
  digitalWrite(in1Pin,LOW);
  digitalWrite(in2Pin,HIGH);
  digitalWrite(in3Pin,LOW);
  digitalWrite(in4Pin,HIGH);
}

void re(){
  if (debugMotores && debugSerial) Serial.print(F(" : RE : "));    
  // Ajusta motores a RE
  digitalWrite(in1Pin,HIGH);
  digitalWrite(in2Pin,LOW);
  digitalWrite(in3Pin,HIGH);
  digitalWrite(in4Pin,LOW);
}

void pare(){
  if (debugMotores && debugSerial) Serial.print(F(" : PARADO!"));    
  // Para moters
  analogWrite(enMAPin, 0);
  analogWrite(enMBPin, 0);      
}


/* PID */

int updatePid(int targetPosition, int currentPosition)   {
  error = targetPosition - currentPosition; 
  //Serial.print("Error: ") ; Serial.println(error);
  pTerm = Kp * error;
  integrated_error += error;                                       
  iTerm = Ki * constrain(integrated_error, -GUARD_GAIN, GUARD_GAIN);
  dTerm = Kd * (error - last_error);                            
  last_error = error;
  return -constrain(K*(pTerm + iTerm + dTerm), -255, 255);
}

/* MPU6050 */

double atualizaSensors(int deltaT){

  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  /* Calcula os angulos (em graus) de X e Y  */
  accXangle = (atan2(ax,az)+PI)*RAD_TO_DEG; 
  accYangle = (atan2(ay,az)+PI)*RAD_TO_DEG;
  
  double gyroXrate =  (double)gx/131.0;
  //double gyroYrate = -((double)gy/131.0);
  
  /* Calcula os angulos X e Y do Giroscopio*/  
  gyroXangle += gyroXrate*((double)deltaT/1000);  
  //gyroYangle += gyroYrate*((double)deltaT/1000);
  
  /* Aplica o Filtro Complementar*/
  compAngleX = (0.93*(compAngleX+(gyroXrate*(double)deltaT/1000)))+(0.07*accXangle); 
  //compAngleY = (0.93*(compAngleY+(gyroYrate*(double)deltaT/1000)))+(0.07*accYangle);  

  // DEBUG //
  if (debugSerial && debugMPU6050){
//    Serial.print(accXangle-180);
//    Serial.print("\t");
//    Serial.print(accYangle-180);
//    Serial.print("\t"); 

//    Serial.print(gyroXangle);
//    Serial.print("\t");
//    Serial.print(gyroYangle);
//    Serial.print("\t");

    Serial.print(compAngleX-180+offSet);
    Serial.print("\t");
//    Serial.print(compAngleY-180); 
//    Serial.print("\t");
  }
  return (compAngleX-180+offSet) ;
  delay(1);
}

