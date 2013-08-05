#include <Arduino.h>

/*
 * MMA7361
 */

/**************
 * DEFINICOES
 **************/

#define pinEixoX                        0
// DEBUG
#define debugSerial                     true
#define debugCalibraOffSetSensor        false
// Motores
#define   InA_R                         6     // INA right motor pin 
#define   InB_R                         7     // INB right motor pin
#define   PWM_R                         10    // PWM right motor pin
#define   InA_L                         8     // INA left motor pin
#define   InB_L                         9     // INB left motor pin
#define   PWM_L                         11    // PWM left motor pin

/**************
 * VARIAVEIS
 **************/

float motorOffsetL = 1; //The offset for left motor
float motorOffsetR = 1; //The offset for right motor

// Constante para ajuste do acelerometro 
const float vRef = 5.0; // Acelerometro vai conectado ao 5v 
// O pino GS da IMU ajusta a sensibilidade entre 1,5G ou 6G:
// Quando LOW  1,5G = 800 mV/g ou 0.800 V/g
// Quando HIGH 6,0G = 206 mV/g ou 0.206 V/g
const float vgSensiL = 0.800; // Sensibilidade do acelerometro em v/g 
const float vgSensiH = 0.206; // Sensibilidade do acelerometro em v/g 
const float vgSensi = vgSensiH;

// Constante em zero G ( -1g = 0.85 V | 1g = 2.45 V
float vZeroGX = 1.65; // 1.66 ; //1.71 ; //1.12 ; // 1.65

// Variaveis para uso dos calculos
const byte SAMPLES = 10;
int valGuidX = 0; // Valor do eixo X
int xOffset = 0;
int anguloX = 0;
float maxGuidX = 0.00;
float minGuidX = 0.00;

// PID
int error = 0;
int setPoint = 0;
int drive = 0;

// Controle do tempo!
// Dt
int STD_LOOP_TIME = 9;
int lastLoopTime = STD_LOOP_TIME;
int lastLoopUsefulTime = STD_LOOP_TIME;
unsigned long loopStartTime = 0;

/* SERIAL */

extern HardwareSerial Serial;

/* FUNCOES */

/*ACELEROMETRO*/
void calcAnguloX();
void calcAnguloXMedia();
void azimuteAcel();

/*KALMAN*/
float kalmanCalculate(float newAngle, float newRate, int looptime);
/*PID Controller*/
int updatePid(int targetPosition, int currentPosition);
/* MOTORES */
int Drive_Motor(int torque);
/* UTEIS */
void serialOut_timing();

void setup() {
    if (debugSerial) {
        Serial.begin(9600);
        Serial.println(F("===================  SETUP ================="));
        if (debugCalibraOffSetSensor) {
            Serial.print(F(" Valor de cada GUID: "));
            Serial.print(minGuidX);
            Serial.print(F(" : "));
            Serial.print(maxGuidX);
        }
    }
    azimuteAcel();
    //calibraOffSetSensor();

    if (debugSerial) {
        Serial.println(F("=============== FIM  SETUP ================="));
    }

}

void loop() {
    //serialOut_timing(); 


    // ********************* Sensor aquisition & filtering *******************
    calcAnguloXMedia();
    //anguloX = kalmanCalculate(anguloX, anguloX, lastLoopTime);            // calculate Absolute Angle

    // *********************** PID and motor drive *****************
    drive = updatePid(setPoint, anguloX); // PID algorithm
    if (anguloX > (setPoint - 40) && anguloX < (setPoint + 40)) {
        Serial.print("PID:");
        Serial.println(drive);
        Drive_Motor(drive);
    } else {
        Serial.println("Parado");
        Drive_Motor(0); // stop motors if situation is hopeless
    }
    // *********************** loop timing control **************************
    lastLoopUsefulTime = millis() - loopStartTime;
    if (lastLoopUsefulTime < STD_LOOP_TIME) delay(STD_LOOP_TIME - lastLoopUsefulTime);
    lastLoopTime = millis() - loopStartTime;
    loopStartTime = millis();
}

/* PID */
#define   GUARD_GAIN   20.0

float K = 1.4;
int Kp = 3;
int Ki = 1;
int Kd = 6;
int last_error = 0;
int integrated_error = 0;
int pTerm = 0, iTerm = 0, dTerm = 0;

int updatePid(int targetPosition, int currentPosition) {
    error = targetPosition - currentPosition;
    pTerm = Kp * error;
    integrated_error += error;
    iTerm = Ki * constrain(integrated_error, -GUARD_GAIN, GUARD_GAIN);
    dTerm = Kd * (error - last_error);
    last_error = error;
    return -constrain(K * (pTerm + iTerm + dTerm), -255, 255);
}

/* ACELEROMETRO*/

void azimuteAcel() { // make 30 iterations, average, and save offset
    //otherwise you get an overflow. But 60 iterations should be fine
    xOffset = 0;
    for (int i = 1; i <= 30; i++) {
        xOffset += analogRead(pinEixoX);
        delay(5); // need delay for 200Hz sample rate
    }
    xOffset /= 30; // get average
    xOffset -= (double) ((1024 / (vRef * 1000)) * 1650); // 0g = 338 Guids = 1.65 V ou 1650 mv 

    minGuidX = (double) ((1024 / (vRef * 1000)) * 850); //  1g =  850 mv
    maxGuidX = (double) ((1024 / (vRef * 1000)) * 2450); // -1g = 2450 mv

}

void calcAnguloX() {


    valGuidX = analogRead(pinEixoX); // Lendo o valor da porta ADC ( quids )
    anguloX = map((valGuidX - xOffset), minGuidX, maxGuidX, 90, -90); // Convert GUID em angulo

    Serial.print(anguloX);
    Serial.print(" : ");
    Serial.println(vZeroGX);

    delay(100);
}

void calcAnguloXMedia() {

    anguloX = 0;
    for (int i = 1; i <= SAMPLES; i++) {

        valGuidX = analogRead(pinEixoX); // Lendo o valor da porta ADC ( quids )
        anguloX += map((valGuidX - xOffset), minGuidX, maxGuidX, 90, -90); // Convert GUID em angulo    
        delay(1);

    }
    anguloX /= SAMPLES;

    Serial.println(anguloX);

    //delay(10);
}

/* KALMAN */
// KasBot V1  -  Kalman filter module

float Q_angle = 0.001; //0.001
float Q_gyro = 0.003; //0.003
float R_angle = 0.03; //0.03

float x_angle = 0;
float x_bias = 0;
float P_00 = 0, P_01 = 0, P_10 = 0, P_11 = 0;
float dt, y, S;
float K_0, K_1;

float kalmanCalculate(float newAngle, float newRate, int looptime) {
    dt = float(looptime) / 1000; // XXXXXXX arevoir
    x_angle += dt * (newRate - x_bias);
    P_00 += -dt * (P_10 + P_01) + Q_angle * dt;
    P_01 += -dt * P_11;
    P_10 += -dt * P_11;
    P_11 += +Q_gyro * dt;

    y = newAngle - x_angle;
    S = P_00 + R_angle;
    K_0 = P_00 / S;
    K_1 = P_10 / S;

    x_angle += K_0 * y;
    x_bias += K_1 * y;
    P_00 -= K_0 * P_00;
    P_01 -= K_0 * P_01;
    P_10 -= K_1 * P_00;
    P_11 -= K_1 * P_01;

    return x_angle;
}

/* UTEIS*/
void serialOut_timing() {
    static int skip = 0;
    if (skip++ == 5) { // display every 500 ms (at 100 Hz)
        skip = 0;
        Serial.print(lastLoopUsefulTime);
        Serial.print(",");
        Serial.print(lastLoopTime);
        Serial.print("\n");
    }
}

/* MOTORES */
int Drive_Motor(int torque) {
    if (torque >= 0) { // drive motors forward
        digitalWrite(InA_R, LOW);
        digitalWrite(InB_R, HIGH);
        digitalWrite(InA_L, LOW);
        digitalWrite(InB_L, HIGH);
    } else { // drive motors backward
        digitalWrite(InA_R, HIGH);
        digitalWrite(InB_R, LOW);
        digitalWrite(InA_L, HIGH);
        digitalWrite(InB_L, LOW);
        torque = abs(torque);
    }
    if (torque > 5) map(torque, 0, 255, 30, 255);
    analogWrite(PWM_R, torque * motorOffsetR);
    //Original value  
    //analogWrite(PWM_L,torque * .9);                            // motor are not built equal...
    analogWrite(PWM_L, torque * motorOffsetL);
}