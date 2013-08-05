#include <Arduino.h>
/*
 * MMA7361
 */

/**************
 * DEFINICOES
 **************/
#define pinEixoX      0
#define pinEixoY      1
#define pinEixoZ      2

// DEBUG
#define debugSerial                true
#define debugCalibraOffSetSensor   false
#define debugAtualizaSensors       false
#define debugLoopTime              true
// Definicao de Hardware
#define useMMA7361                 true

// Para uso nos calculos
#define amostrasPinEixos           10
#define amostrasOffset             50



/**************
 * VARIAVEIS
 **************/
const float vRef = 5.0 ; // Acelerometro vai conectado ao 5v do Arduino
/* Extraido do DATASHEET do produto
   O pino GS da IMU ajusta a sensibilidade entre 1,5G ou 6G (G Select):
   Quando LOW  1,5G = 800 mV/g ou 0.800 V/g
   Quando HIGH 6,0G = 206 mV/g ou 0.206 V/g
   Bandwidth Response  400Hz
 */
const float vgSensiL = 0.800; // Sensibilidade do acelerometro em v/g 
const float vgSensiH = 0.206 ;// Sensibilidade do acelerometro em v/g 
const float vgSensi = vgSensiL ; 

const float valorGuid = ( vRef / 1024 ) ; // valorGuid tem o valor em volts de cada guid
            
// Constante em zero G ( -1g = 0.85V, 0g = 1.65V e 1g = 2.45V )
//float vZeroGX = 1.65 ; // 1.66 ; //1.71 ; //1.12 ; // 1.65

// Variaveis para uso dos calculos
int sensorZeroGuid[3]      = { 0, 0, 0 }; 
int sensorValorGuid[3]     = { 0, 0, 0 };
float sensorValorForcaG[3] = { 0, 0, 0 };
float sensorValorGraus[3]  = { 0, 0, 0 };

float pitch = 0.00 ;
float roll  = 0.00 ;

// Dt
int STD_LOOP_TIME           = 9;  
int lastLoopTime            = STD_LOOP_TIME;
int lastLoopUsefulTime      = STD_LOOP_TIME;
unsigned long loopStartTime = 0;

/* SERIAL */

extern HardwareSerial Serial;

/* FUNCOES */


void converteGuid2Graus();
void calibraOffSetSensor();
void atualizaSensors() ;

void setup() {
 if (debugSerial){
  Serial.begin(9600);
  Serial.println(F("===================  SETUP ================="));
  if (debugCalibraOffSetSensor){
    Serial.print(F(" Valor de cada GUID: "));
    Serial.println(valorGuid);
  }
 } 
 
 calibraOffSetSensor();

 if (debugSerial){
  Serial.println(F("=============== FIM  SETUP ================="));
 } 

}

void loop() {
if (debugLoopTime) Serial.println(lastLoopTime);

  atualizaSensors();
  converteGuid2Graus();
  Serial.print(F("pitch: "));Serial.println(pitch);
 // *********************** loop timing control **************************
  lastLoopUsefulTime = millis()-loopStartTime;
  if(lastLoopUsefulTime<STD_LOOP_TIME) { 
    delay(STD_LOOP_TIME-lastLoopUsefulTime);
    if (debugLoopTime){
      Serial.print("  Esperei: ");
      Serial.println(STD_LOOP_TIME-lastLoopUsefulTime);
    }
  }
  lastLoopTime = millis() - loopStartTime;
  loopStartTime = millis();
}

/* MMA7361 */

/*
 * Define o valor de offset de cada eixo.
 * Esta funcao eh chamada somente no setup e o 
 * acelerometro devera estar, com os eixos X e Y
 * nivelados com o chao .
 */
void calibraOffSetSensor(){
  long valor;

  for(int pinEixo = 0 ; pinEixo < 2; pinEixo++) { // Leio as tres portas
    valor = 0;
    for(int c = 0; c < amostrasOffset; c++){ // Faco a media da leitura
      valor += analogRead(pinEixo);
    }
    sensorZeroGuid[pinEixo] = valor / amostrasOffset;
    sensorZeroGuid[2] = 300; // <== Valor de ajuste do eixo Z
    
    if (debugCalibraOffSetSensor){
      Serial.print(F(" Valor de offset do Eixo: "));
      Serial.print(pinEixo);
      Serial.print(F(" = "));
      Serial.println(sensorZeroGuid[pinEixo]);            
    }      
  } 
}

/*
 * Leio cada eixo do sensor
 */
void atualizaSensors() {           

  long valor;

  for(int pinEixo=0; pinEixo<3; pinEixo++) { // Leio as tres portas
    valor = 0;
    for(int c=0; c<amostrasPinEixos; c++){       
      valor += analogRead(pinEixo);
    }
    sensorValorGuid[pinEixo] = ( ( valor/amostrasPinEixos ) - sensorZeroGuid[pinEixo] ); // Tiro a media e subtraio o offset
  }
  if (debugAtualizaSensors){
    Serial.print(F(" Guids offset: "));                  
    for(int pinEixo=0; pinEixo<3; pinEixo++) {      
      Serial.print(sensorValorGuid[pinEixo]);                  
      Serial.print(F(" , "));
    }    
  }
}

void converteGuid2Graus(){
 
  for(int pinEixo=0; pinEixo<3; pinEixo++) { // Leio as tres portas    
    //sensorValorForcaG[pinEixo] = ((float) sensorValorGuid[pinEixo] * ( vRef / 1023 ) ) / vgSensi ; // O resultado eh em g/s
    sensorValorForcaG[pinEixo] = ((float) sensorValorGuid[pinEixo] * valorGuid ) / vgSensi ; // O resultado eh em g/s
  }

  pitch = atan2(sensorValorForcaG[pinEixoX], sensorValorForcaG[pinEixoZ]) * ( 180 / PI ) ; // ou radians ( + PI ) 
  roll  = atan2(sensorValorForcaG[pinEixoY], sensorValorForcaG[pinEixoZ]) * ( 180 / PI ) ; // ou radians ( + PI ) 
  
  float R = sqrt( pow( sensorValorForcaG[pinEixoX], 2) + pow(sensorValorForcaG[pinEixoY],2) + pow (sensorValorForcaG[pinEixoZ],2) ); // R eh o vetor resultante das componente RXG, RYG e RZG
  
  for(int pinEixo=0; pinEixo<3; pinEixo++) { // Leio as tres portas
    sensorValorGraus[pinEixo] =  acos(sensorValorForcaG[pinEixo]/R)  * ( 180 / PI ) ;   // Converte para Graus!
  }

  // DEBUG //
  if (debugAtualizaSensors){
    Serial.print(" | G/s: ");                  
    for(int pinEixo=0; pinEixo<3; pinEixo++) {      
      Serial.print(sensorValorForcaG[pinEixo]);                  
      if ( pinEixo<2 )  Serial.print(F(" , "));            
    }        
    Serial.print(" | Graus: ");                  
    for(int pinEixo=0; pinEixo<3; pinEixo++) {      
      Serial.print(sensorValorGraus[pinEixo]);                  
      if ( pinEixo<2 ) Serial.print(F(" , "));
    }
    
    Serial.print(F(" | PITCH: "));
    Serial.print(pitch);
    Serial.print(F(" | ROLL: "));
    Serial.print(roll);
    
    //----------------  FIM //
    Serial.println("");                  
  }  
}
