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
#define debugLoopTime              true

// Definicao de Hardware
#define useMPU6050                 true

/**************
 * VARIAVEIS
 **************/
/* MPU6050 */
double accXangle; // Angle calculate using the accelerometer
double accYangle;
double gyroXangle = 180; // Angle calculate using the gyro
double gyroYangle = 180;
double compAngleX = 180; // Calculate the angle using a Kalman filter
double compAngleY = 180;


MPU6050 accelgyro(0x69);
/*
 * DATASHEET
 * 2g  = 0.06 mg/LSB
 * 4g  = 0.12
 * 8g  = 0.24
 * 16G = 0.49
 * Leitura em 16Bits
 */

// deltaT
int STD_LOOP_TIME           = 9;  
int lastLoopTime            = STD_LOOP_TIME;
int lastLoopUsefulTime      = STD_LOOP_TIME;
unsigned long loopStartTime = 0;
int deltaT                  = 0;
// Acel Gyro

int16_t ax, ay, az;
int16_t gx, gy, gz;

extern HardwareSerial Serial ;

/**************
 * FUNCOES
 **************/

int calcDeltaT();
void atualizaSensors(int deltaT);

/**************
 * EXECUCAO
 **************/

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

  if (debugSerial){
    Serial.println(F("=============== FIM  SETUP ================="));
  } 

}

void loop() {
    if (debugSerial && debugLoopTime) Serial.println(lastLoopTime);

  atualizaSensors(deltaT);
  // *********************** loop timing control **************************
  deltaT = calcDeltaT(); 
}

/* MPU6050 */

void atualizaSensors(int deltaT){

  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  /* Calcula os angulos (em graus) de X e Y  */
  accYangle = (atan2(ax,az)+PI)*RAD_TO_DEG;
  accXangle = (atan2(ay,az)+PI)*RAD_TO_DEG; 
  
  double gyroXrate =  (double)gx/131.0;
  double gyroYrate = -((double)gy/131.0);
  
  /* Calcula os angulos X e Y do Giroscopio*/  
  gyroXangle += gyroXrate*((double)deltaT/1000);  
  gyroYangle += gyroYrate*((double)deltaT/1000);
  
  /* Aplica o Filtro Complementar*/
  compAngleX = (0.93*(compAngleX+(gyroXrate*(double)deltaT/1000)))+(0.07*accXangle); 
  compAngleY = (0.93*(compAngleY+(gyroYrate*(double)deltaT/1000)))+(0.07*accYangle);  

  // DEBUG //
  if (debugSerial && debugMPU6050){
    Serial.print(accXangle);
    Serial.print("\t");
    Serial.print(accYangle);
    Serial.print("\t"); 

    Serial.print(gyroXangle);
    Serial.print("\t");
    Serial.print(gyroYangle);
    Serial.print("\t");

    Serial.print(compAngleX);
    Serial.print("\t");
    Serial.print(compAngleY); 
    Serial.print("\t");
  }
  delay(1);
}

/* UTEIS */

int calcDeltaT(){
  lastLoopUsefulTime = millis()-loopStartTime;
  if(lastLoopUsefulTime<STD_LOOP_TIME) { 
    delay(STD_LOOP_TIME-lastLoopUsefulTime);
    if (debugSerial && debugLoopTime){
      Serial.print("  Esperei: ");
      Serial.println(STD_LOOP_TIME-lastLoopUsefulTime);
    }
  }
  lastLoopTime = millis() - loopStartTime;
  loopStartTime = millis(); 
  return lastLoopTime;
}

