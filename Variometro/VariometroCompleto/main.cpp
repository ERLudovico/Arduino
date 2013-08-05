#include <Arduino.h>
#include <Time.h> 
#include <Wire.h> 
#include <Timer.h>
#include <EEPROM.h>
#include <Arduino.h>
//#include <DateTime.h>
#include <Enerlib.h>
#include <EDB.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <Adafruit_BMP085.h>
#include <MomentaryButton.h>
#include <DS1307RTC.h>
/*
 * CONSTANTES PARA DEBUG
 */
#define debugSerial   false
#define debugTimer    false
#define debugClock    false 
#define debugPCD8544  false
#define debugTeclado  false
#define debugDB       false
#define debugRtc      false
#define debugShutdown false
#define debugBMP085   false
/*
 * CONSTANTES PARA DEFINICAO
 * DOS PERIFERICOS 
 */
#define usaPcd8544    true
#define usaDB         true
#define usaRtc        true
#define usaBMP085     true  
/* 
 * Pino de Alimentacao do DISPLAY
 */
#define vccDisplay  13 // VERDE
/*
 * Pinos para leitura dos botoes
 */
#define pinUp       11 // AZUL
#define pinDown      8 // BRANCO 
#define pinDisplay   2 // CINZA
#define pinSetup    12 // ROXO
/* 
 * Pino de Alimentacao do DISPLAY
 */
#define pinReset  8 // Conectar o pinReset no pino de reset do arduino!!!
/*
 * Pinos para o RTC DS1302 :(
 */
//#define CE_PIN     A2 // 10 //  RST
//#define IO_PIN     A1 //  9 //  DAT
//#define SCLK_PIN   A0 //  8 //  CLK
/*
 * Definicoes para uso do BANCO
 */
#define TABLE_SIZE 1024 // Tamanho da Memoria do Arduino
#define RECORDS_TO_CREATE 20 // Tamanho da Tabela de Registros
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
const char ShutdownL1[]  PROGMEM      = { 
  "Desliga ?"      };

const char AjustaRelogio[]  PROGMEM   = { 
  "Salva Ajustes?"   };

const char TruncaDBL1[]  PROGMEM      = { 
  "Apaga Logbook?" };

const char ResetaAltitudeL1[]  PROGMEM      = { 
  "Salva Ajustes?" };

const char SimNao[]  PROGMEM          = { 
  "SIM|NAO"        };

/*
 * DEFINICOES DE VARIAVEIS GLOBAIS
 */

boolean emConfirmacao = false ; 

byte volume = 1 ;
byte Pcd8544Tela = 0 ;
byte confirmaItem = 0 ;
byte showRegistroID = 0 ;
byte setupClockCount = 0 ;
byte setupAltitudeCount = 0 ;

int startupAltitude = 0 ;
int acelVertUp , acelVertDown = 0 ;
int hora , minuto , segundo , dia , mes , ano = 0 ;
int upitmeDays , uptimeHours , uptimeMinutes , uptimeSeconds = 0 ;

long tempoTotal = 0 ;
long temperatura, pressao , altitude , altitudeMax, altitudeMin = 0.00 ;

float acelVert = 0 ;
float alturaConhecida , pressaoConhecida , pressaoNivelDoMarE , alturaE = 0 ;

char startupDate[15] ;
char uptimeDuracao[6] ;

String digito , horaRelogio, dataRelogio = "";

struct LogBook {
  int id ; 
  char dataInicial[15];
  char duracao[15] ;
  char acelVertUp[2] ;
  char acelVertDown[2] ;
  char startupAltitude[5];
  char shutdownAltitude[5];
  char altitudeMax[5] ;
  char altitudeMin[5] ;
}
registro;

/*
 * CONSTRUTORES
 */

/* Este bloco deve ser declarado nesta ordem
 * pois e dependencia para a instanciacao da
 * classe DB */
void writer(unsigned long address, byte data) { 
  EEPROM.write(address, data);
}
byte reader(unsigned long address){ 
  return EEPROM.read(address); 
}
EDB db(&writer, &reader);

Energy energy;
Timer scheduler;

Adafruit_BMP085 sensorP;

MomentaryButton bDisplay(pinDisplay);
MomentaryButton bSetup(pinSetup);
MomentaryButton bUp(pinUp);
MomentaryButton bDown(pinDown);

Adafruit_PCD8544 display = Adafruit_PCD8544(P8544_CLK, P8544_DIN, P8544_DC, P8544_CS, P8544_RST);

extern HardwareSerial Serial;
/*
 *  Declaraco de FUNCOES
 */
/*Beep*/
void defaultBeep();
void beepAcelVertUP();
void beepAcelVertDown();

/*Display*/
void turnONDisplay(); // Energiza pino do Display
void initDisplay();   // Envia comando de inicializacao ao Display
void displayPCDTela_Confirmacao();
void displayPCDTela_0() ;
void displayPCDTela_1();
void displayPCDTela_2();
void telaConfirma(byte item);
void atualizaDisplayRelogio();
String criaDigito(int val);

/*Sensor*/
void initSensorPress();  // Coleta e processa os primeiros dados do sensor
void valoresPadroesSensorP();
void leSensorPressao(); // Le os valores do sensor
void ajustaSensorP();

/*Altitude*/
void atualizaAltitudeMaxEMin(); // Baseado no novos dados define as altitudes maximas e minimas
void calcMediaAltitude(); // Calcula a altitude media das 10 ultimas amostragens... 
float alturaEstimada();
void calcAcelVert();
void calcAcelVert2();

/*Teclado*/
void leTeclado3Botoes();

/*Shutdown*/
void resetArdu();
void shutdownNow();
void acorda_ISR(void);

/*Relogio*/
void sincRtcToRelogio();
void sincRelogioToRtc();
byte ano2d();
void uptime();

/* DB */
byte reader(unsigned long address);
void writer(unsigned long address, byte data);
void initDB();
void initDados();
void gravaRegistro();
void selectRegistro(byte indice);
void printError(EDB_Status err);
void calculaDuracaoEGrava(boolean gravaEvento);
void truncaDB();

/*VU*/
 void vuParaCima (boolean a, byte b, byte c, byte d, byte e);
 void vuParaBaixo(boolean a, byte b, byte c, byte d, byte e);
 void vuParaCima (boolean a, byte b, byte c, byte d, byte e); 
 void vuParaBaixo(boolean a, byte b, byte c, byte d, byte e);
      
void setup(){
  /* Prepara Pino de Reset*/
  digitalWrite(pinReset, HIGH);
  pinMode(pinReset, OUTPUT);
  
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

  /* Ajuste do Relogio do Ardu baseado no RTC */
  if (usaRtc){
    //rtc.write_protect(false);
    //rtc.halt(false); 
    sincRtcToRelogio();    
  } 
  else {
    setTime(11,11,00,21,12,2012); // Fim no calendario MAIA =)
  }  

  if (usaPcd8544){
    /* Ajuste Vcc do Display*/
    turnONDisplay();
    initDisplay();
    atualizaDisplayRelogio(); // eh necessario ??
  }  
  
  /* DB */
  if (usaDB){
    initDB(); 
  }

  /* Sensort Pressao */
  if (usaBMP085) {
    sensorP.begin();
    initSensorPress();
    //valoresPadroesSensorP();
  }
  
  /* Schedula os processos =)*/
  int clockID     = scheduler.every(1000,  atualizaDisplayRelogio); //   1 Segundos
  int sensorID    = scheduler.every(1000,  leSensorPressao);        //   1 Segundos
  int uptimeID    = scheduler.every(1000,  uptime);                 //   1 Segundos
  int acelVertID  = scheduler.every(1000,  calcAcelVert);           //   1 Segundos
  int tecladoID   = scheduler.every(500,   leTeclado3Botoes);       // 0,5 Segundos
  int sincID      = scheduler.every(60000, sincRtcToRelogio);       //  60 Segundos
  
  /* Registar Dados de StartUP do Ardu */
  initDados();
  //sprintf(startupDate, "%02d:%02d %02d/%02d/%02d", hora,minuto,dia,mes,ano2d());
  //startupAltitude = altitude ;
  
  /* DEBUG */
  if ( debugSerial ){
    if (debugTimer)   { 
      Serial.print(F("Id do Processo clock: "))    ; 
      Serial.println(clockID);    
    }    
    if (debugTeclado) { 
      Serial.print(F("Id do Processo Teclado: "))  ; 
      Serial.println(tecladoID);
    }    
    if (debugRtc)     { 
      Serial.print(F("Id do Processo Sinc RTC: ")) ; 
      Serial.println(sincID);
      Serial.println(startupDate);  
    }      
    Serial.println(F("-------"));  
  }

}

void loop(){

  scheduler.update();

  if (usaPcd8544){
    display.clearDisplay();
    if (emConfirmacao){
      displayPCDTela_Confirmacao();
    } 
    else {
      switch(Pcd8544Tela){
      case 0:      
        displayPCDTela_0(); // Altimetro 
        break;
      case 1:
        displayPCDTela_1(); // Relogio
        break;
      case 2:
        displayPCDTela_2(); // LogBook
        break;
      }     
    }
  }
}

/*********************
 * FUNCOES AUXILIARES
 *********************/

/* BEEP */
void defaultBeep(){
   // Temporario
   // Terra para o PIEZO
   // pinMode(8,OUTPUT);
   // digitalWrite(8,LOW);
   //int duration = 20; // how long the tone lasts
   //int frequency = 2000 ; //map(sensor0Reading, 0, 1023, 100,5000); //100Hz to 5kHz ;
   //tone(speakerPin, frequency, duration); // play the tone
   if ( volume == 1 ){
     tone(9,2000,20); 
   }
}

/* TECLADO */
void leTeclado3Botoes(){

  /********************* Botao SETUP *********************/
  bSetup.check();
  if (bSetup.wasClicked()) {
    if ( (debugSerial) && ( debugTeclado)) Serial.println(F("SETUP Clicked!"));
    /***** SETUP CLICKED *****/
    if ( Pcd8544Tela == 0) { // Caso na Tela do Altimetro  
      // Ajusta a Altura
    } 
    else if ( Pcd8544Tela == 1) { // Caso na Tela ddo Relogio
      // Ajusto o Relogio ?
    } 
    else if ( Pcd8544Tela == 2) { // Caso na Tela de Registros  
      // Apaga todos os Registros ?
    }
  } 
  else if (bSetup.wasHeld()) {
    if ( (debugSerial) && ( debugTeclado)) Serial.println(F("SETUP was Held!"));
    /***** SETUP HELD *****/
    /** Tela de Confirmacao **/
    if ( emConfirmacao ) {
      switch(confirmaItem){
      case 0: // Shutdown ???     
        shutdownNow(); // Desliga o Arduino!!
        break;
      case 1: // Ajusta Relogio !!!             
        sincRelogioToRtc();
        setupClockCount = 0 ;  
        break;
      case 2: // Trunca DB ???     
        truncaDB(); // Trunca Registros!!
        //Pcd8544Tela = 0 ;
        break;
      case 3: // Salva Configuracao ???             
        Pcd8544Tela = 0 ;
        setupAltitudeCount = 0 ;
        break;
      }     
      emConfirmacao = false ;
    } 
    else if ( ! emConfirmacao ) {/** FIM Tela de Confirmacao **/
      if ( Pcd8544Tela == 0) { // Caso na Tela do Altimetro  
        // Ajusta a Altura
        setupAltitudeCount++ ;        
        if ( setupAltitudeCount == 4 ) setupAltitudeCount = 1 ;
      } 
      else if ( Pcd8544Tela == 1) { // Caso na Tela ddo Relogio
        // Ajusto o Relogio ?
        setupClockCount++ ;        
        if ( setupClockCount >= 6 ) setupClockCount = 1 ;
      } 
      else if ( Pcd8544Tela == 2) { // Caso na Tela de Registros  
        // Apaga todos os Registros ?
      }
    }
  }
  /********************* Fim Botao SETUP *********************/

  /********************* Botao DISPLAY *********************/
  bDisplay.check();

  if (bDisplay.wasClicked()) {
    if ( (debugSerial) && ( debugTeclado)) Serial.println(F("DISPLAY Clicked!"));
    /***** DISPLAY CLICKED *****/
    if ( setupClockCount == 0 ){ // Caso Nao esteja ajustando o Relogio....
      Pcd8544Tela++ ;
      if ( Pcd8544Tela >= 3 ){
        Pcd8544Tela = 0 ; 
      }
    
      if ( (debugSerial) && ( debugTeclado)) {
        Serial.print(F("Tela: "));
        Serial.println(Pcd8544Tela);
      }
    }
  } 
  else if (bDisplay.wasHeld()) {
    if ( (debugSerial) && ( debugTeclado)) Serial.println(F("DISPLAY Was Held!"));    
    /***** DISPLAY HELD *****/
    if ( emConfirmacao ) { // Cancelo todas as operacoes
      switch(confirmaItem){
      case 0: // Shutdown ???     
        // Cancelar o SHUTDOWN!!        
        break;
      case 1: // NAO salvo a nova data no RTC !!!             
        setupClockCount = 0 ;   // Retira do modo Ajuste de Relogio        
        sincRtcToRelogio();     // Resgata a Hora do RTC
        break;
      case 2: // Trunca DB ???             
        // Cancelar o Trunca DB
        break;
      case 3: // Uso a altitude default do sensor...             
        valoresPadroesSensorP();
        setupAltitudeCount = 0 ;
        break;      
      }     
      emConfirmacao = false ;      
    } 
    else {      
      if ( Pcd8544Tela == 0 ) { // Caso na Tela do Altimetro    
        if ( setupAltitudeCount == 0 ){ // Garante que o Altimetro nao estah sendo ajustado
          telaConfirma(0); // Confirma o Shutdown...
          if ( (debugSerial) && ( debugTeclado) ) Serial.println(F("Shutdown NOW?"));
        } 
        else if ( setupAltitudeCount > 0 ){ // O altimetro estah sendo ajustado...
          telaConfirma(3); // Salva nova Altitude...?
          if ( (debugSerial) && ( debugTeclado) ) Serial.println(F("Confirma Nova Altitude?"));  
        }
      } 
      else if ( ( Pcd8544Tela == 1 ) && ( setupClockCount > 0 )) { // Caso na Tela do Relogio    
        telaConfirma(1); // Ajusta o RTC ?
        if ( (debugSerial) && ( debugTeclado) ) Serial.println(F("Ajusta Hora?"));
      }
      else if ( Pcd8544Tela == 2) { // Caso na Tela de Registros    
        telaConfirma(2); // Trunca DB
        if ( (debugSerial) && ( debugTeclado) ) Serial.println(F("Trunca DB?"));
      }
    }

  } 
  else if (bDisplay.onHeld()) {
    if ( (debugSerial) && ( debugTeclado) ) Serial.println(F("DISPLAY On Held!"));

    /***** DISPLAY ONHELD *****/
  }

  /********************* Fim Botao DISPLAY *********************/


  /********************* Botao UP *********************/
  bUp.check();

  if (bUp.wasClicked()) {
    if ( (debugSerial) && ( debugTeclado)) Serial.println(F("UP Clicked!"));
    /***** UP CLICKED *****/
    if ( Pcd8544Tela == 0 ){ // Na tela do Altimetro
      if ( setupAltitudeCount == 1 ){
        alturaConhecida += 1.00 ;
        ajustaSensorP();
        leSensorPressao();
        //atualizaAltitudeMaxEMin(); 
      } 
    } else if ( Pcd8544Tela == 2) { // Caso na Tela de Registros
      showRegistroID++ ;
      if ( showRegistroID > db.count() ) showRegistroID = 0 ;
      selectRegistro(showRegistroID);
    }
  } 
  else if (bUp.wasHeld()) {
    if ( (debugSerial) && ( debugTeclado)) Serial.println(F("UP Was Held!"));
    /***** UP HELD *****/
    if ( setupAltitudeCount == 2 ){
      volume++ ;
      if ( volume == 2 ){ 
        volume = 0  ;
      }
    } 
  } 
  else if (bUp.onHeld()) {
    if ( (debugSerial) && ( debugTeclado) ) Serial.println(F("UP On Held!"));    
    /***** UP ONHELD *****/
    if ( Pcd8544Tela == 0 ){ // Na tela do Altimetro
      if ( setupAltitudeCount == 1 ){
        alturaConhecida += 5.00 ;
        ajustaSensorP();
        leSensorPressao();
        //atualizaAltitudeMaxEMin();  
      }        
    }
    else if ( Pcd8544Tela == 1 ){ // Na tela do Relogio
      switch(setupClockCount){
      case 1:    
        adjustTime(60); // Minuto
        break;
      case 2:
        adjustTime(3600); // Hora
        break;
      case 3:
        adjustTime(86400); // Dia
        break;  
      case 4:
        if ( mes == 12 ) mes = 0 ;
        mes += 1 ;
        setTime(hora,minuto,segundo,dia,mes,ano); // Mes
        break;  
      case 5:
        ano += 1 ;
        setTime(hora,minuto,segundo,dia,mes,ano); // Ano
        break;
      }
      atualizaDisplayRelogio();    
    }
  }

  /********************* Fim Botao UP *********************/

  /********************* Botao DOWN *********************/
  bDown.check();

  if (bDown.wasClicked()) {
    if ( (debugSerial) && ( debugTeclado)) Serial.println(F("DOWN Clicked!"));
    /***** DOWN CLICKED *****/
    if ( Pcd8544Tela == 0 ){ // Na tela do Altimetro
      if ( setupAltitudeCount == 1 ){
        alturaConhecida -= 1.00 ;
        ajustaSensorP();
        leSensorPressao(); 
        //atualizaAltitudeMaxEMin();
      }            
    } else if ( Pcd8544Tela == 2) { // Caso na Tela de Registros
      showRegistroID-- ;
      if ( showRegistroID <= 0 ) showRegistroID = db.count() ;
      selectRegistro(showRegistroID);
    }
  } 
  else if (bDown.wasHeld()) {
    if ( (debugSerial) && ( debugTeclado)) Serial.println(F("DOWN Was Held!"));
    /***** DOWN WAS HELD *****/
    if ( setupAltitudeCount == 2 ){
      volume-- ;
      if ( volume < 0 ){ 
        volume = 0  ;
      }
    } 

  } 
  else if (bDown.onHeld()) {
    if ( (debugSerial) && ( debugTeclado) ) Serial.println(F("DOWN On Held!"));
    /***** DOWN ONHELD *****/
    if ( Pcd8544Tela == 0 ){ // Na tela do Altimetro
      if ( setupAltitudeCount == 1 ){
        alturaConhecida -= 5.00 ;
        ajustaSensorP();
        leSensorPressao();
        //atualizaAltitudeMaxEMin(); 
      }            
    }
    else if ( Pcd8544Tela == 1 ){ // Na tela do Relogio
      switch(setupClockCount){
      case 1:    
        adjustTime(-60); // Minuto
        break;
      case 2:
        adjustTime(-3600); // Hora
        break;
      case 3:
        adjustTime(-86400); // Dia
        break;  
      case 4:
        if ( mes == 1 ) mes = 13 ;
        mes -= 1 ;
        setTime(hora,minuto,segundo,dia,mes,ano); // Mes
        break;  
      case 5:
        ano -= 1 ;
        setTime(hora,minuto,segundo,dia,mes,ano); // Ano
        break;
      }
      atualizaDisplayRelogio();    
    }    
  }

  /********************* Fim Botao DOWN *********************/
}

/******************
 *  BANCO 
 ******************/

void initDB(){

  db.open(0);
  if ((debugSerial) && ( debugDB )) Serial.println(F("Banco Alarme aberto!"));
  if ( db.count() == 0 ){
    db.create(0, TABLE_SIZE, (unsigned int)sizeof(registro));
    if ((debugSerial) && (debugDB)){
      Serial.println(F("Banco Alarme criado!"));
    }
  } 
  else {
    showRegistroID = db.count() ;
    selectRegistro(showRegistroID);
  }
}

void printError(EDB_Status err){
  if ((debugSerial) && (debugDB)){
    Serial.print(F("ERROR: "));
  }
  switch (err)
  {
  case EDB_OUT_OF_RANGE:
    if ((debugSerial) && (debugDB)) Serial.println(F("Recno out of range"));
    break;
  case EDB_TABLE_FULL:
    if ((debugSerial) && (debugDB)) Serial.println(F("Table full"));
    break;
  case EDB_OK:
  default:
    if ((debugSerial) && (debugDB)) Serial.println(F("OK"));
    break;
  }
}

void gravaRegistro(){

  registro.id = db.count() + 1 ;

  sprintf(registro.dataInicial, "%s", startupDate );
  sprintf(registro.duracao, "%02dD %02d:%02d:%02d", upitmeDays,uptimeHours,uptimeMinutes,uptimeSeconds);
  sprintf(registro.acelVertUp, "%01d" , acelVertUp );
  sprintf(registro.acelVertDown, "%01d" , ( acelVertDown * -1 ) );  
  sprintf(registro.startupAltitude , "%d" , (char*)startupAltitude );
  sprintf(registro.shutdownAltitude, "%d" , (char*)altitude );
  sprintf(registro.altitudeMax , "%d" , (char*)altitudeMax );
  sprintf(registro.altitudeMin , "%d" , (char*)altitudeMin );
  
  EDB_Status result = db.appendRec(EDB_REC registro);
  if (result != EDB_OK) printError(result);

}

void truncaDB(){
  if ( (debugSerial) && ( debugDB) ){
    Serial.println(F("Apagar TUDO!"));
  }
  db.clear();  
}

void selectRegistro(byte indice){
  EDB_Status result = db.readRec(indice, EDB_REC registro);
  if (result == EDB_OK) {
    if ((debugSerial) && (debugDB)){
      Serial.print(F("Indice: ")); 
      Serial.println(indice);
      Serial.print(F(" ID: ")); 
      Serial.println(registro.id);
      Serial.print(F("Data Inicial: ")); 
      Serial.println(registro.dataInicial);
    }   
  }
  else printError(result);
}

void calculaDuracaoEGrava(boolean gravaEvento){
  tempoTotal = ( millis() / 1000 );

  int days = elapsedDays(tempoTotal);
  int hours = numberOfHours(tempoTotal);
  int minutes = numberOfMinutes(tempoTotal);
  int seconds = numberOfSeconds(tempoTotal);

  if ( gravaEvento ) sprintf(registro.duracao, "%02d-%02d:%02d:%02d", upitmeDays,uptimeHours,uptimeMinutes,uptimeSeconds);
}

/******************
 * DISPLAY
 ******************/

void displayPCDTela_0(){ // Altimetro
  if ( setupAltitudeCount > 0 ){ // SETUP CLOCK
    display.setTextSize(1);
    display.setTextColor(WHITE, BLACK);
    display.setCursor(0,0);
    if ( setupAltitudeCount == 1 ) display.print("Ajusta Altura?");
    if ( setupAltitudeCount == 2 ) display.print("Ajusta Volume?");    
    if ( setupAltitudeCount == 3 ) display.print("Ajusta Sensi.?");        
  } else {
    display.setTextSize(1);
    display.setTextColor(BLACK);
    /* Cronometro */
    display.setCursor(0,0);    
    display.print( uptimeDuracao ); 
    /* Altitude Maxima */     
    display.setCursor(45,0);
    display.print(altitudeMax);
    /* VU Meter */
    int Y = 0 ;
    if ( acelVert > 0 ) {
      //Y = map( acelVert, 7 , 0 , 7 , 0 );
      //vuParaCima (1,6,Y,72,0);
      vuParaCima (1,6,acelVert,72,0);
      vuParaBaixo(1,6,0,72,24);
    } else if ( acelVert < 0 ) {
      //Y = map( acelVert, 0 , -7 , 0 , 7 );
      vuParaCima (1,6,0,72,0); 
      //vuParaBaixo(1,6,Y,72,24);
      vuParaBaixo(1,6,( acelVert * -1 ),72,24);
    }
  }
  display.drawLine(0, 8, 70, 8, BLACK); // Linha Horizontal
  // Altura
  display.setTextSize(3);
  display.setTextColor(BLACK);
  display.setCursor(0,12);
  display.print(altitude);
  if ( altitude < 1000 ){ 
    display.setTextSize(1);
    display.setTextColor(BLACK);  
    display.print(F("Mts"));
  }
  // Linha Horizontal 
  display.drawLine(0, 38, 70, 38, BLACK); // Linha Horizontal
  //Hora
  display.setTextSize(1);
  display.setTextColor(BLACK);
  /* Volume */
  display.setCursor(0,40);
  if ( volume == 1 ) display.write((char)14);
//  /* Numero de Registro */ 
//  display.setCursor(10,40);  
//  display.print(db.count());
  /* Hora do Relogio */
  display.setCursor(10,40);
  display.print(horaRelogio);  
  /* Altitude Minima */     
  display.setCursor(45,40);
  display.print(altitudeMin); 
  /* Rederiza */
  display.display(); 
}

void displayPCDTela_1(){ // Relogio
  // SETUP MODE
  if ( setupClockCount >= 1 ){ // SETUP CLOCK
    display.setTextSize(1);
    display.setTextColor(WHITE,BLACK);
    display.setCursor(0,0);
    display.write((char)175);
    if ( setupClockCount == 1 )display.print("Ajusta MINUTO"); 
    if ( setupClockCount == 2 )display.print("Ajusta HORA"); 
    if ( setupClockCount == 3 )display.print("Ajusta DIA"); 
    if ( setupClockCount == 4 )display.print("Ajusta MES"); 
    if ( setupClockCount == 5 )display.print("Ajusta ANO");
  } else { // Mostra Temperatura
    display.setTextSize(1);
    display.setTextColor(BLACK);
    /* Cronometro */
    display.setCursor(0,0);    
    display.print( uptimeDuracao ); 
    /* Temperatura */     
    display.setCursor(38,0);
    display.print(temperatura);
    display.write((char)247);
    display.print("C");  
  }   
  display.drawLine(0, 8, 84, 8, BLACK); // Linha Horizontal    
  // Hora
  display.setTextSize(2);
  display.setTextColor(BLACK);
  display.setCursor(13,13);
  display.print(horaRelogio);
  display.setTextSize(1);
  display.setTextColor(BLACK);  
  display.setCursor(70,30);
  display.print(segundo);
  // Linha Horizontal 
  display.drawLine(0, 38, 84, 38, BLACK); // Linha Horizontal
  //Data
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0,40);
  display.print(db.count());
  display.setCursor(24,40);
  display.print(dataRelogio);  
  display.display();
}

void displayPCDTela_2(){ // LogBook

  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0,0);
  display.print("LogBook: ");  

  if ( db.count() == 0 ){
    display.setCursor(0,20);
    display.print("Sem Registros!");        
  } 
  else {
    display.print(registro.id);
    display.print("/");  
    display.print(db.count());
    display.drawLine(0, 8, 84, 8, BLACK); // Linha Horizontal
    display.setCursor(0,10);
    display.print(registro.dataInicial);
    display.setCursor(8,18);
    display.print(registro.duracao);

    /* Ascendente */
    display.setCursor(8,26);
    display.print("Asc:");
    display.print(registro.acelVertUp);
    /* Descendente */ 
    display.setCursor(42,26);
    display.print(" Des:");
    display.print(registro.acelVertDown);

    /* Altitude Inicial */
    display.setCursor(8,34);
    display.print(registro.startupAltitude);
    /* Altitude Final */ 
    display.setCursor(42,34);
    display.print(registro.shutdownAltitude);

    /* Altitude Maxima */
    display.setCursor(8,42);
    display.print(registro.altitudeMax);
    /* Altitude Minima */ 
    display.setCursor(42,42);
    display.print(registro.altitudeMin);

  }
  display.display();
}

void displayPCDTela_Confirmacao(){ // ALARMES

  display.setTextSize(1);
  display.setTextColor(BLACK);

  switch(confirmaItem){
  case 0: // Shutdown      
    display.setCursor(0,0);
    display.print(FS(ShutdownL1));  
    break;
  case 1: // Ajusta Relogio      
    display.setCursor(0,0);
    display.print(FS(AjustaRelogio)); 
    break;
  case 2: // Apaga LogBook     
    display.setCursor(0,0);
    display.print(FS(TruncaDBL1)); 
    break;
  case 3: // Reseta a Altitude Estimada     
    display.setCursor(0,0);
    display.print(FS(ResetaAltitudeL1)); 
    break;

  }

  display.setTextSize(2);
  display.setCursor(0,20);
  display.print(FS(SimNao)); // Altimetro       
  display.display();
}

/******************
 *  DISPLAY
 ******************/

void turnONDisplay(){
  pinMode(vccDisplay, OUTPUT);
  digitalWrite(vccDisplay, LOW); 
}

void turnOFFDisplay(){
  digitalWrite(13, HIGH); 
}

void initDisplay(){
  display.begin();
  display.setContrast(50);
  display.clearDisplay();

}

void telaConfirma(byte item){
  emConfirmacao = true ;
  confirmaItem = item ;
}

/****************** 
 * ISR 
 ******************/
void acorda_ISR(void) {

  if (energy.WasSleeping()) {
    if ( debugSerial ){
      Serial.begin(9600);
      if ( debugShutdown ) Serial.println(F("Levantando!!"));
    }
    detachInterrupt(0);
    defaultBeep();
    resetArdu();
    // turnONDisplay(); 
    // initDisplay();
    // initDados();
    // initSensorPress();
    // Inicializar o valor de alturas, millis... etc
  }
}

void shutdownNow(){
  if ((debugSerial)&&(debugShutdown)) Serial.println(F("Shutdown!!"));
  gravaRegistro();
  attachInterrupt(0, acorda_ISR, LOW);
  turnOFFDisplay();
  defaultBeep();
  energy.PowerDown();
}

void resetArdu(){
   digitalWrite(pinReset, LOW);
}

/* Guarda Dados iniciais*/
void initDados(){
  sprintf(startupDate, "%02d:%02d %02d/%02d/%02d", hora,minuto,dia,mes,ano2d()); // Data e Hora que o Ardu foi iniciado
  startupAltitude = altitude ; // Altitude que Arduino estava quando iniciado
  // Verificar como zerar o contador da funcao millis()   

}

/****************** 
 * RELOGIO 
 ******************/

void uptime(){
  tempoTotal = ( millis() / 1000 );
  upitmeDays = elapsedDays(tempoTotal);
  uptimeHours = numberOfHours(tempoTotal);
  uptimeMinutes = numberOfMinutes(tempoTotal);
  uptimeSeconds = numberOfSeconds(tempoTotal);
  
  sprintf(uptimeDuracao, "%02d:%02d", uptimeHours,uptimeMinutes);
}

void sincRelogioToRtc(){
  /* DS1302 */
  //Time t(ano, mes, dia, hora, minuto, segundo, 0);
  //rtc.time(t);
  RTC.set( now() );
  if ( (debugSerial) && (debugRtc) ) Serial.println(F("Relogio To RTC")); 
  sincRtcToRelogio();
}

void sincRtcToRelogio(){
  if (setupClockCount == 0){
    /* DS1302 */
    //Time r = rtc.time();
    //setTime(r.hr, r.min, r.sec, r.date, r.mon ,r.yr );
    /* DS1307 */
    setSyncProvider(RTC.get);
    if ( (debugSerial) && (debugRtc) ) Serial.println(F("RTC To Relogio")); 
  }
}

void atualizaDisplayRelogio(){
  hora ,  minuto , segundo , dia , mes, ano = 0 ;
  horaRelogio = "" ;
  dataRelogio = "" ;

  hora     = hour();
  minuto   = minute();
  segundo  = second();
  dia      = day();
  mes      = month();
  ano      = year();

  horaRelogio.concat( criaDigito( hora ));
  horaRelogio.concat( ":");  
  horaRelogio.concat( criaDigito( minuto ));
  dataRelogio.concat( dia)  ; 
  dataRelogio.concat( "/"); 
  dataRelogio.concat( mes );
  dataRelogio.concat( "/"); 
  dataRelogio.concat( ano );

  if ( debugSerial ){
    if (debugClock) {
      //calculaDuracaoEGrava(0);
      uptime();   
      Serial.print(millis());
      Serial.print(F(" "));            
      Serial.print(horaRelogio);
      Serial.print(F(" "));      
      Serial.print(dataRelogio);
      Serial.println(F(" "));      

    }     
  }  
}

String criaDigito(int val){
  digito = "";
  if (val < 10) digito.concat("0") ;
  digito.concat( val );
  return digito ;
}

byte ano2d(){

  if ( ano < 2000 ){
    return ( ano - 1900 ) ; 
  } 
  else {
    return ( ano - 2000 ) ;    
  } 
}

/* Sensor PRESSAO*/
/*
 * Le o sensor BMP085 e atualiza os valores de pressao
 * , altitude e temperatura.
 */
void initSensorPress(){
   
   valoresPadroesSensorP();
   altitudeMax = altitude ;
   altitudeMin = altitude ;
}
 


void leSensorPressao(){

  temperatura = sensorP.readTemperature();
  pressao = sensorP.readPressure();
  pressaoNivelDoMarE == 0 ? altitude = sensorP.readAltitude() : altitude = sensorP.readAltitude(pressaoNivelDoMarE);
  if ( (debugSerial) && (debugBMP085) ){
    Serial.println(F("\n * BMP085 ***************************"));
    Serial.print(F(" Alt(M): "));                 
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
  atualizaAltitudeMaxEMin();
}

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

void ajustaSensorP(){

  leSensorPressao(); // Atualiza valores de pressao, altura e temperatura
  pressaoNivelDoMarE =  pressao * pow((1 - ( alturaConhecida * -1 )/44330), 5.255) ;  // Pressao do nivel do mar estimada pela altura conhecida em Pa 
}

void valoresPadroesSensorP(){
    leSensorPressao(); // Leio o sensor    
    alturaConhecida = alturaEstimada() ; // Defino altura inicial como confiavel
    ajustaSensorP(); // Ajusto a leitura    
}

void calcAcelVert(){
  // Temporario para teste!!
  pinMode(A3, INPUT);
  int x = analogRead(A3);
  acelVert = map (x, 0 , 1024 , -7 , 7 );
  if ( ( acelVert > 0 ) && ( acelVertUp < acelVert ) ) acelVertUp = acelVert ;
  if ( ( acelVert < 0 ) && ( acelVertDown > acelVert ) ) acelVertDown = acelVert ;
}

void atualizaAltitudeMaxEMin(){ 
  if ( altitude > altitudeMax ) altitudeMax = altitude ;
   //if ( altitude < 0 ) altitude = 0 ; // Nao guarda valores negativos
  if ( altitude < altitudeMin ) altitudeMin = altitude ;
}


/**********************
 * Mostra valores no VU
 **********************/
void vuParaCima(boolean fullVu, byte compVu, byte posVu, byte colIni, byte linIni){
  if ( posVu > compVu ) posVu = compVu ; // Anti overflow
  byte uniY = (barraVer_Y + 1) ;
  byte linFin = linIni + ( compVu * uniY );
  byte linB   = linFin - ( posVu  * uniY ); 

  for ( ; linIni < linB   ; linIni += uniY ) display.drawRect( colIni, linIni, barraVer_X, barraVer_Y, BLACK);   
  for ( ; linB   < linFin ; linB   += uniY ) display.fillRect( colIni, linB  , barraVer_X, barraVer_Y, BLACK);
}


void vuParaBaixo(boolean fullVu, byte compVu, byte posVu, byte colIni, byte linIni){
  if ( posVu > compVu ) posVu = compVu ; // Anti overflow
  byte uniY = (barraVer_Y + 1) ;
  byte linFin = linIni + ( compVu * uniY );
  byte linB   = linIni + ( posVu  * uniY ); 

  for ( ; linIni < linB   ; linIni += uniY ) display.fillRect( colIni, linIni, barraVer_X, barraVer_Y, BLACK);  
  for ( ; linB   < linFin ; linB   += uniY ) display.drawRect( colIni, linB  , barraVer_X, barraVer_Y, BLACK);

}
