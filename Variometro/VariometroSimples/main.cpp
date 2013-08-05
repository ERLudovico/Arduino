#include <Arduino.h>
#include <Wire.h> 
#include <wiring.c>
#include <Timer.h>
#include <Enerlib.h>
#include <EDB.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <Adafruit_BMP085.h>
#include "MomentaryButton.h"
#include "RunningAverage.h"
#include "toneAC.h"
/*
 * CONSTANTES PARA DEBUG
 */
#define debugSerial   false
#define debugPCD8544  false
#define debugTeclado  false
#define debugShutdown false
#define debugBMP085   false
#define debugBeep     false
#define altitudeDummy false
/*
 * CONSTANTES PARA DEFINICAO
 * DOS PERIFERICOS 
 */
#define usaPcd8544    true
#define usaBMP085     true
#define usaBeep       true
/* 
 * Pino de Alimentacao do DISPLAY
 */
#define vccDisplay  13
/*
 * Pinos para leitura dos botoes
 */
#define pinUp       11
#define pinDown      8 // 10  <-- Emprestado do reset para teste do beep
#define pinDisplay   2
#define pinSetup    12
/* 
 * Pino Conectado ao RESET
 */
// #define pinReset  8
/*
 * Definicoes para uso do LCD
 */
#define P8544_RST  3 // RESET
#define P8544_CS   4 // Chip Select
#define P8544_DC   5 // Data / Command
#define P8544_DIN  6 // Data in
#define P8544_CLK  7 // Clock
/*
 * Definicoes para uso do VUMeter
 */
#define barraHor_X  3
#define barraHor_Y 10
#define barraVer_X 12
#define barraVer_Y  3
/*
 * Definicoes para uso da SRAM
 * Mensagens na Tela
 */
#define FS(x) (__FlashStringHelper*)(x)
const char ShutdownL1[] PROGMEM = {
    "Desliga ?"};

const char ResetaAltitudeL1[] PROGMEM = {
    "Salva Ajustes?"
};

const char SimNao[] PROGMEM = {
    "SIM|NAO"};

/*
 * DEFINICOES DE VARIAVEIS GLOBAIS
 */
boolean setupModeOn = false;
boolean emConfirmacao = false;
boolean beepAcelVertON = false;

byte volume = 1;
byte Pcd8544Tela = 0;
byte confirmaItem = 0;
byte setupAltitudeCount = 0;

int acelVert0 = 0.00;
int beepAcelVertID = -1;
/* Running Average*/
int myRACountSamples = 0;
int myRABufferSamples = 300;

//int startupAltitude = 0 ;
float acelVertUp, acelVertDown = 0.00;

float tempoTotal = 0.00;
float tempo0, altitude0EmMts, altitude0EmCm = 0;
float temperatura, pressao, altitude, altitudeNow, altitudeMax, altitudeMin = 0.00;

float acelVert = 0.00;
float alturaConhecida, pressaoConhecida, pressaoNivelDoMarE, alturaE = 0.00;


/*
 * CONSTRUTORES
 */
Energy energy;
Timer scheduler;
Adafruit_BMP085 sensorP;
RunningAverage myRA(10);

MomentaryButton bDisplay(pinDisplay);
MomentaryButton bSetup(pinSetup);
MomentaryButton bUp(pinUp);
MomentaryButton bDown(pinDown);

Adafruit_PCD8544 display = Adafruit_PCD8544(P8544_CLK, P8544_DIN, P8544_DC, P8544_CS, P8544_RST);


/*********************
 * FUNCOES AUXILIARES
 *********************/
/*Beep*/
void defaultBeep();
void beepAcelVertUP();
void beepAcelVertDown();

/*Display*/
void turnONDisplay(); // Energiza pino do Display
void initDisplay();   // Envia comando de inicializacao ao Display
void displayPCDTela_Confirmacao();
void displayPCDTela_0() ;
void telaConfirma(byte item);
/*Sensor*/
void initSensorPress();  // Coleta e processa os primeiros dados do sensor
void valoresPadroesSensorP();
void leSensorPressao(); // Le os valores do sensor
void ajustaSensorP();

void calcAcelVert2();

void atualizaAltitudeMaxEMin(); // Baseado no novos dados define as altitudes maximas e minimas
void calcMediaAltitude(); // Calcula a altitude media das 10 ultimas amostragens... 
float alturaEstimada();

/*Teclado*/
void leTeclado3Botoes();

/*Shutdown*/
void resetArdu();
void shutdownNow();

/*Serial*/
extern HardwareSerial Serial;

void setup() {
  /* Prepara Pino de Reset*/
  //digitalWrite(pinReset, HIGH);
  //pinMode(pinReset, OUTPUT);

  /* BEEP */
  defaultBeep();
  /* Inicializa Serial */
  
  if ( debugSerial ) { 
    Serial.begin(9600);
    Serial.println(F("-------"));
  }

  /* Inicializa os Botoes */
  bSetup.setup();
  bDisplay.setup();
  bUp.setup();
  bDown.setup();


  if (usaPcd8544){
    /* Ajuste Vcc do Display*/
    turnONDisplay(); // Energiza pino do Display
    initDisplay();   // Envia comando de inicializacao ao Display

  }  


  /* Sensort Pressao */
  if (usaBMP085) {
    sensorP.begin();    // Inicializa a comunicacao com o Sensor
    initSensorPress();  // Coleta e processa os primeiros dados do sensor
  }

  /* Schedula os processos =)*/
  int tecladoID       = scheduler.every(500,   leTeclado3Botoes);       // 0,5 Segundos
  int calcAcelVertID  = scheduler.every(1100,  calcAcelVert2);          //   1 Segundos

  /* Running Average */
  myRA.clear(); // explicitly start clean
  
  /* DEBUG */
  if ( debugSerial ){
    if (debugTeclado) { 
      Serial.print(F("Id do Processo Teclado: "))  ; 
      Serial.println(tecladoID);
    }    
    Serial.println(F("-------"));  
  }


}

void loop() {
    scheduler.update(); // Verifica processos agendados

    (setupAltitudeCount >= 1) ? setupModeOn = true : setupModeOn = false;

    if (!emConfirmacao) { // Caso a tela seja de confirmacao
        //if (! setupModeOn){  
        leSensorPressao(); // Le os valores do sensor
        atualizaAltitudeMaxEMin(); // Baseado no novos dados define as altitudes maximas e minimas
        calcMediaAltitude(); // Calcula a altitude media das 10 ultimas amostragens... 
    }


    /* DISPLAY */
    if (usaPcd8544) { // Caso esteja usando o DISPLAY...
        display.clearDisplay();
        if (emConfirmacao) { // caso esteja em tela de confirmacao, rederizo a tela...
            displayPCDTela_Confirmacao();
        } else { // ...caso nao...
            displayPCDTela_0(); // entro na tela do Altimetro 
        }
    }
}

/*
 * Mostra valores no VU
 */
void vuParaCima(boolean fullVu, byte compVu, byte posVu, byte colIni, byte linIni) {
    if (posVu > compVu) posVu = compVu; // Anti overflow
    byte uniY = (barraVer_Y + 1);
    byte linFin = linIni + (compVu * uniY);
    byte linB = linFin - (posVu * uniY);

    for (; linIni < linB; linIni += uniY) display.drawRect(colIni, linIni, barraVer_X, barraVer_Y, BLACK);
    for (; linB < linFin; linB += uniY) display.fillRect(colIni, linB, barraVer_X, barraVer_Y, BLACK);
}

void vuParaBaixo(boolean fullVu, byte compVu, byte posVu, byte colIni, byte linIni) {
    if (posVu > compVu) posVu = compVu; // Anti overflow
    byte uniY = (barraVer_Y + 1);
    byte linFin = linIni + (compVu * uniY);
    byte linB = linIni + (posVu * uniY);

    for (; linIni < linB; linIni += uniY) display.fillRect(colIni, linIni, barraVer_X, barraVer_Y, BLACK);
    for (; linB < linFin; linB += uniY) display.drawRect(colIni, linB, barraVer_X, barraVer_Y, BLACK);
    //display.display();
}

/* 
 * BEEP 
 */
void defaultBeep() {
    //int duration = 20; // how long the tone lasts
    //int frequency = 2000 ; //map(sensor0Reading, 0, 1023, 100,5000); //100Hz to 5kHz ;
    //tone(speakerPin, frequency, duration); // play the tone
    if (volume == 1) {
        tone(9, 2000, 20);
    }
}

void beepAcelVert() {
    if (!usaBeep) return;

    if ((debugSerial) && (debugBeep)) {
        Serial.print(F("Beep: "));
        Serial.println(acelVert);
    }
    int beepSigh = (int) acelVert;

    if ((debugSerial) && (debugBeep)) {
        Serial.print(F("Beep Sigh: "));
        Serial.println(beepSigh);
        Serial.println(1000 / beepSigh);
    }

    scheduler.stop(beepAcelVertID);
    if (beepSigh >= 1) {
        beepAcelVertID = scheduler.every((1000 / beepSigh), beepAcelVertUP);
        //beepAcelVertID  = scheduler.every( 100 , beepAcelVertUP);         
    } else if (beepSigh <= -1) {
        beepAcelVertID = scheduler.every(1000, beepAcelVertDown);
    }

    if (beepAcelVertON == false) beepAcelVertON = true;
}

/* Gera beep para aceleracao descendente */
void beepAcelVertUP() {
    if ((debugSerial) && (debugBeep)) {
        Serial.println(F(" >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> BEEP "));
    }
    tone(9, 4000, 20);
}

void beepAcelVert2() {
    if (!usaBeep) return;

    if ((debugSerial) && (debugBeep)) {
        Serial.print(F("Beep: "));
        Serial.println(acelVert);
    }
    int beepSigh = (int) acelVert;

    if ((debugSerial) && (debugBeep)) {
        Serial.print(F("Beep Sigh: "));
        Serial.println(beepSigh);
        Serial.println(1000 / beepSigh);
    }

    scheduler.stop(beepAcelVertID);
    if (beepSigh >= 1) {
        beepAcelVertID = scheduler.every((1000 / beepSigh), beepAcelVertUP);
        //beepAcelVertID  = scheduler.every( 100 , beepAcelVertUP);         
    } else if (beepSigh <= -1) {
        beepAcelVertID = scheduler.every(1000, beepAcelVertDown);
    }

    if (beepAcelVertON == false) beepAcelVertON = true;
}

/* Gera beep para aceleracao vertical descendente */
void beepAcelVertDown() {
    if ((debugSerial) && (debugBeep)) {
        Serial.print(F(" >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> BOOOOOHHHHH "));
        Serial.println(acelVert);
    }

    int freque = (int) (acelVert * -1);
    freque = freque * 100;

    if ((debugSerial) && (debugBeep)) {
        Serial.println(freque);
    }

    tone(9, freque, 2000);
}

/* TECLADO */
void leTeclado3Botoes() {

    /********************* Botao SETUP *********************/
    bSetup.check();
    if (bSetup.wasClicked()) {
        if ((debugSerial) && (debugTeclado)) Serial.println(F("SETUP Clicked!"));
        /***** SETUP CLICKED *****/
        if (Pcd8544Tela == 0) { // Caso na Tela do Altimetro  
            // Ajusta a Altura
        }
        else if (Pcd8544Tela == 1) { // Caso na Tela ddo Relogio
            // Ajusto o Relogio ?
        }
        else if (Pcd8544Tela == 2) { // Caso na Tela de Registros  
            // Apaga todos os Registros ?
        }
    }
    else if (bSetup.wasHeld()) {
        if ((debugSerial) && (debugTeclado)) Serial.println(F("SETUP was Held!"));
        /***** SETUP HELD *****/
        /** Tela de Confirmacao **/
        if (emConfirmacao) {
            switch (confirmaItem) {
                case 0: // Shutdown ???     
                    shutdownNow(); // Desliga o Arduino!!
                    break;
                case 1: // Ajusta Relogio !!!             
                    break;
                case 2: // Trunca DB ???     
                    break;
                case 3: // Salva Configuracao ???             
                    //Pcd8544Tela = 0 ;
                    setupAltitudeCount = 0;
                    break;
            }
            emConfirmacao = false;
        }
        else if (!emConfirmacao) {/** FIM Tela de Confirmacao **/
            if (Pcd8544Tela == 0) { // Caso na Tela do Altimetro  
                // Ajusta a Altura
                setupAltitudeCount++;
                if (setupAltitudeCount == 4) setupAltitudeCount = 1;
            }
            else if (Pcd8544Tela == 1) { // Caso na Tela ddo Relogio
                // Ajusto o Relogio ?
            }
            else if (Pcd8544Tela == 2) { // Caso na Tela de Registros  
                // Apaga todos os Registros ?
            }
        }
    }
    /********************* Fim Botao SETUP *********************/

    /********************* Botao DISPLAY *********************/
    bDisplay.check();

    if (bDisplay.wasClicked()) {
        if ((debugSerial) && (debugTeclado)) Serial.println(F("DISPLAY Clicked!"));
        /***** DISPLAY CLICKED *****/
    }
    else if (bDisplay.wasHeld()) {
        if ((debugSerial) && (debugTeclado)) Serial.println(F("DISPLAY Was Held!"));
        /***** DISPLAY HELD *****/
        if (emConfirmacao) { // Cancelo todas as operacoes
            switch (confirmaItem) {
                case 0: // Shutdown ???     
                    // Cancelar o SHUTDOWN!!        
                    break;
                case 1: // NAO salvo a nova data no RTC !!!             
                    break;
                case 2: // Trunca DB ???             
                    // Cancelar o Trunca DB
                    break;
                case 3: // Uso a altitude default do sensor...             
                    valoresPadroesSensorP();
                    setupAltitudeCount = 0;
                    break;
            }
            emConfirmacao = false;
        }
        else {
            if (Pcd8544Tela == 0) { // Caso na Tela do Altimetro    
                if (setupAltitudeCount == 0) { // Garante que o Altimetro nao estah sendo ajustado
                    telaConfirma(0); // Confirma o Shutdown...
                    if ((debugSerial) && (debugTeclado)) Serial.println(F("Shutdown NOW?"));
                }
                else if (setupAltitudeCount > 0) { // O altimetro estah sendo ajustado...
                    telaConfirma(3); // Salva nova Altitude...?
                    if ((debugSerial) && (debugTeclado)) Serial.println(F("Confirma Nova Altitude?"));
                }
            }
        }
    }
    else if (bDisplay.onHeld()) {
        if ((debugSerial) && (debugTeclado)) Serial.println(F("DISPLAY On Held!"));

        /***** DISPLAY ONHELD *****/
    }

    /********************* Fim Botao DISPLAY *********************/


    /********************* Botao UP *********************/
    bUp.check();

    if (bUp.wasClicked()) {
        if ((debugSerial) && (debugTeclado)) Serial.println(F("UP Clicked!"));
        /***** UP CLICKED *****/
        if (Pcd8544Tela == 0) { // Na tela do Altimetro
            if (setupAltitudeCount == 1) {
                alturaConhecida += 1.00;
                ajustaSensorP();
                leSensorPressao();
                //atualizaAltitudeMaxEMin(); 
            }
        }
        else if (Pcd8544Tela == 2) { // Caso na Tela de Registros
        }
    }
    else if (bUp.wasHeld()) {
        if ((debugSerial) && (debugTeclado)) Serial.println(F("UP Was Held!"));
        /***** UP HELD *****/
        if (setupAltitudeCount == 2) {
            volume++;
            if (volume == 2) {
                volume = 0;
            }
        }
    }
    else if (bUp.onHeld()) {
        if ((debugSerial) && (debugTeclado)) Serial.println(F("UP On Held!"));
        /***** UP ONHELD *****/
        if (Pcd8544Tela == 0) { // Na tela do Altimetro
            if (setupAltitudeCount == 1) {
                alturaConhecida += 5.00;
                ajustaSensorP();
                leSensorPressao();
                //atualizaAltitudeMaxEMin();  
            }

        }
    }

    /********************* Fim Botao UP *********************/

    /********************* Botao DOWN *********************/
    bDown.check();

    if (bDown.wasClicked()) {
        if ((debugSerial) && (debugTeclado)) Serial.println(F("DOWN Clicked!"));
        /***** DOWN CLICKED *****/
        if (Pcd8544Tela == 0) { // Na tela do Altimetro
            if (setupAltitudeCount == 1) {
                alturaConhecida -= 1.00;
                ajustaSensorP();
                leSensorPressao();
                //atualizaAltitudeMaxEMin();
            }
        }
    }
    else if (bDown.wasHeld()) {
        if ((debugSerial) && (debugTeclado)) Serial.println(F("DOWN Was Held!"));
        /***** DOWN WAS HELD *****/
        if (setupAltitudeCount == 2) {
            volume--;
            if (volume < 0) {
                volume = 0;
            }
        }

    }
    else if (bDown.onHeld()) {
        if ((debugSerial) && (debugTeclado)) Serial.println(F("DOWN On Held!"));
        /***** DOWN ONHELD *****/
        if (Pcd8544Tela == 0) { // Na tela do Altimetro
            if (setupAltitudeCount == 1) {
                alturaConhecida -= 5.00;
                ajustaSensorP();
                leSensorPressao();
                //atualizaAltitudeMaxEMin(); 
            }
        }
    }

    /********************* Fim Botao DOWN *********************/
}

/*
 * DISPLAY
 */

void displayPCDTela_0() { // Altimetro
    if (setupAltitudeCount > 0) { // SETUP CLOCK
        display.setTextSize(1);
        display.setTextColor(WHITE, BLACK);
        display.setCursor(0, 0);
        if (setupAltitudeCount == 1) display.print("Ajusta Altura?");
        if (setupAltitudeCount == 2) display.print("Ajusta Volume?");
        if (setupAltitudeCount == 3) display.print("Ajusta Sensi.?");
    }
    else {
        display.setTextSize(1);
        display.setTextColor(BLACK);
        /* Aceleracao Verticao Positiva */
        display.setCursor(8, 0);
        display.print(acelVertUp, 1);
        /* Altitude Maxima */
        display.setCursor(45, 0);
        display.print((int) altitudeMax);
        /* VU Meter */
        int Y = 0;
        if (acelVert >= 0) {
            vuParaCima(1, 6, acelVert, 72, 0);
            vuParaBaixo(1, 6, 0, 72, 24);
        }
        else if (acelVert < 0) {
            vuParaCima(1, 6, 0, 72, 0);
            vuParaBaixo(1, 6, (acelVert * -1), 72, 24);
        }
        //display.drawLine(70, 23, 84, 23, BLACK); // Linha Horizontal
        //display.drawTriangle(24,20 , 24,28 , 20,28, BLACK);
        //display.drawTriangle(40,0 , 40,28 , 0,28, BLACK);
        //display.drawTriangle(40,10 , 40,30 , 0,20, BLACK);
        display.drawTriangle(68, 26, 71, 23, 68, 20, BLACK); // Seta que indica o meio do VU
    }

    display.drawLine(0, 8, 70, 8, BLACK); // Linha Horizontal
    // Area do meio, mostro a Altura corrente
    display.setTextSize(3);
    display.setTextColor(BLACK);
    display.setCursor(0, 12);
    display.print((int) altitude);
    if (altitude < 1000) {
        display.setTextSize(1);
        display.setTextColor(BLACK);
        display.print(F("Mts"));
    }
    // Linha Horizontal 
    display.drawLine(0, 38, 70, 38, BLACK); // Linha Horizontal
    //Hora
    display.setTextSize(1);
    display.setTextColor(BLACK);
    /* Aceleracao Verticao Negativa */
    display.setCursor(0, 40);
    display.print(acelVertDown, 1);
    /* Volume */
    display.setCursor(27, 40);
    if (volume == 1) display.write((char) 14);
    /* Altitude Minima */
    display.setCursor(37, 40);
    display.print((int) altitudeMin);
    /* Rederiza */
    display.display();
}

void displayPCDTela_Confirmacao() { // ALARMES

    display.setTextSize(1);
    display.setTextColor(BLACK);

    switch (confirmaItem) {
        case 0: // Shutdown      
            display.setCursor(0, 0);
            display.print(FS(ShutdownL1));
            break;
        case 1: // Ajusta Relogio ?      
            display.setCursor(0, 0);
            break;
        case 2: // Apaga LogBook ?    
            display.setCursor(0, 0);
            break;
        case 3: // Reseta a Altitude Estimada ?     
            display.setCursor(0, 0);
            display.print(FS(ResetaAltitudeL1));
            break;

    }

    display.setTextSize(2);
    display.setCursor(0, 20);
    display.print(FS(SimNao)); // Mostra a mensagem "SIM | NAO"       
    display.display();
}



/*
 * Funcoes auxiliares do DISPLAY 
 */

/* Liga o Display */
void turnONDisplay() {
    pinMode(vccDisplay, OUTPUT);
    digitalWrite(vccDisplay, LOW);
}

/* Desliga o Display */
void turnOFFDisplay() {
    digitalWrite(vccDisplay, HIGH);
}

/* Inicializa o Display */
void initDisplay() {
    display.begin();
    display.setContrast(30);
    display.clearDisplay();

}

void telaConfirma(byte item) {
    emConfirmacao = true;
    confirmaItem = item;
}

/* ISR */
void acorda_ISR(void) {

    if (energy.WasSleeping()) {
        if (debugSerial) {
            Serial.begin(9600);
            if (debugShutdown) Serial.println(F("Levantando!!"));
        }
        detachInterrupt(0);
        defaultBeep();
        resetArdu();
    }
}

void shutdownNow() {
    if ((debugSerial)&&(debugShutdown)) Serial.println(F("Shutdown!!"));

    attachInterrupt(0, acorda_ISR, LOW);
    turnOFFDisplay();
    defaultBeep();
    energy.PowerDown();
}

void resetArdu() {
    //digitalWrite(pinReset, LOW);
}

/* Sensor PRESSAO*/
/*
 * Le o sensor BMP085 e atualiza os valores de pressao
 * , altitude e temperatura.
 */
 
void initSensorPress(){

  alturaEstimada();
  altitudeMax = altitude ;
  altitudeMin = altitude ;
}


/* Le os valor do sensor*/
void leSensorPressao(){
  //if (setupModeOn) return ;
  temperatura = sensorP.readTemperature();
  pressao = sensorP.readPressure();
  //pressaoNivelDoMarE == 0 ? altitude = sensorP.readAltitude() : altitude = sensorP.readAltitude(pressaoNivelDoMarE);
  pressaoNivelDoMarE == 0 ? altitudeNow = sensorP.readAltitude() : altitudeNow = sensorP.readAltitude(pressaoNivelDoMarE);
  if ( (debugSerial) && (debugBMP085) && (0 )){
    Serial.println(F("\n * BMP085 ***************************"));
    Serial.print(F(" Alt(M): "));                 
    Serial.print(altitudeNow);
    Serial.print(F(" Alt(M) Media: "));                 
    Serial.print(altitude);
    Serial.print(F(" Temp(C): "));                
    Serial.print(temperatura);
    Serial.print(F(" Altura Conhecida(M): "));    
    Serial.print(alturaConhecida);
    Serial.print(F(" Altura Estimada(M): "));     
    Serial.print(alturaE);
    Serial.print(F(" Pressao Conhecida(pa): "));  
    Serial.print(pressaoConhecida);      
    Serial.print(F(" Pressure(Pa):"));            
    Serial.println(pressao);
  }
}

/* Altura estimada baseado na pressao padrao ao nível do mar (101325)*/
float alturaEstimada(void){
  /*********************************************************************************** 
   *  - Conversor Altitude para Pressao
   * Baseado na pressao conhecida (em Pa) encontro a Altura ( em metros ).
   * (float)44330 * (1 - pow(((float) pressao/p0), 0.190295))
   *
   *  - Conversor Pressao para Altitude 
   * Baseado na altura conecida ( em metros ) encontro a pressao (em Pa ).
   * p0 * pow((1-alturaConhecida/44330), 5.255)
   *
   *  - Estima a Pressao no Nível do Mar
   * Baseado em uma pressao ( Pa ) e altura ( metros ) conhecidas estima
   * a pressao ao nevel do mar ( Pa ) que por padrao é 101325 ( pa ).
   * pressao * pow((1 - ( alturaConhecida * -1 )/44330), 5.255) 
   ***********************************************************************************
   */

  const float p0 = 101325;     // Pressao ao nivel do mar
  float alturaE = (float)44330 * (1 - pow(((float) pressao/p0), 0.190295)); //Altura Estima baseado na pressao (em Pa) conhecida
  return alturaE;
}

/* Calibra QNC */
void ajustaSensorP(){

  leSensorPressao(); // Atualiza valores de pressao, altura e temperatura
  pressaoNivelDoMarE =  pressao * pow((1 - ( alturaConhecida * -1 )/44330), 5.255) ;  // Pressao do nivel do mar estimada pela altura conhecida em Pa 
}

/* Ajusta o QNC padrao*/
void valoresPadroesSensorP(){
  leSensorPressao(); // Leio o sensor    
  alturaConhecida = alturaEstimada() ; // Defino altura inicial como confiavel
  ajustaSensorP(); // Ajusto a leitura    
}

/* Calcula a aceleracao vertical*/
void calcAcelVert2(){
  if ( millis() < 1000 ) return ; 

  if ( tempo0 == 0 ){
    tempo0 = millis();
    altitude0EmCm = ( altitude * 100 ) ;
    acelVert0 = 0 ;
  } 
  else {
    float tempoDecorridoEmSeg = ( ( millis() - tempo0 ) / 1000) ;
//    Serial.println(millis());
//    Serial.println(tempo0);
   int difDeAltitudeEmCm = 0 ;
    //--**  Temporario para teste!!
    if ( altitudeDummy ){
      pinMode(A3, INPUT);
      int x = analogRead(A3);
      int altitudeDummyEmCm = map (x, 0 , 1024 , 20000 , 20900 );

      Serial.println(altitude0EmCm);
      Serial.println(altitudeDummyEmCm);
      difDeAltitudeEmCm = ( altitude0EmCm - altitudeDummyEmCm ) ; 
    //--** FIM
    } else {
      difDeAltitudeEmCm = altitude0EmCm - ( altitude * 100 ) ;    
    }
    
    if ( (debugSerial) && (debugBMP085) ){
      Serial.print(F("Diferenca de Alt em CM : "));    
      Serial.println(difDeAltitudeEmCm);
      Serial.print(F("Tempo Decorrido em Segundos : "));    
      Serial.println(tempoDecorridoEmSeg);
    }

    acelVert = (  difDeAltitudeEmCm / tempoDecorridoEmSeg  ) / 100 ; // Converte para M/s

    if ( acelVertUp < acelVert   ) acelVertUp   = acelVert ;
    if ( acelVertDown > acelVert ) acelVertDown = acelVert ;


    if ( (debugSerial) && (debugBMP085) ){
      Serial.print(F("Ultima Aceleracao Vertical: "));
      Serial.println(acelVert0);
      Serial.print(F("Aceleracao Vertical: "));
      Serial.println(acelVert, 3);
      Serial.print(F("Aceleracao Vertical UP: "));
      Serial.println(acelVertUp);
      Serial.print(F("Aceleracao Vertical DOWN: "));
      Serial.println(acelVertDown);
    }

    if ( ( acelVert <= -1 ) || ( acelVert >= 1 ) ){
      if ( (int)acelVert0 != (int)acelVert )  beepAcelVert() ;
      //if ( acelVert0 != acelVert )  beepAcelVert() ;
    } else if (( acelVert > -1 ) && ( acelVert < 1 )) {
         if ( beepAcelVertON == true ) beepAcelVertON = false ;
         scheduler.stop(beepAcelVertID) ; 
    }
    // Shift dos valores
    tempo0 = millis();
    altitude0EmCm = (int)( altitude * 100 ) ;
    acelVert0 = (int)acelVert ;
  }
}

/* Calcula a media das 10 ultimas amostragens da altitude */
void calcMediaAltitude(){
  
  if (setupModeOn){
    altitude = altitudeNow ;
    return ;
  }
  
  myRA.addValue(altitudeNow);
  myRACountSamples++ ;

  altitude = myRA.getAverage(); 
  
  if (myRACountSamples == myRABufferSamples){
    myRACountSamples = 0;
    myRA.clear();
  }
}

/* Atualiza Altitudes Maxima e Minima*/
void atualizaAltitudeMaxEMin(){ 
  if ( altitude > altitudeMax ) altitudeMax = altitude ;
  if ( altitude < altitudeMin ) altitudeMin = altitude ;
}

