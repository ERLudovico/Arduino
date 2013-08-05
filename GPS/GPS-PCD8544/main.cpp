#include <Arduino.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <EDB.h>
#include <EEPROM.h>
//#include <SD.h>
#include <stdlib.h>

/* 
 * @Incluir: A. Incluir Display - @OK.
 * @Incluir: B. Incluir Botoes para manipular as telas do GPS
 * @Incluir: C. Incluir DB para manutencao dos Waypoint - @OK.
 * @Incluir: D. Incluir Rotas com distancia e orientacao
 * @Incluir: E. Logger track no cartao SD
 * 
 * Conexao GPS
 * 1 <--> GND
 * 2 <--> 5v
 * 3 <--> A0
 * 4 <--> A1
 * 5 <--> GND
 * 5 <--> N/C
 */

/************************
       DEFINICOES
 ************************/
/* DEBUG */
#define debugSerial     true
#define debugGPS        true
#define debugDB         true
#define debugSD         false

/* Hardware */
#define useGPS          true
#define useSerial       true
#define usaPcd8544      true
#define usaDB           true
#define usaSD           false
/*
 * Definicoes para uso do LCD
 */
#define P8544_RST       3 // RESET
#define P8544_CS        4 // Chip Select
#define P8544_DC        5 // Data / Command
#define P8544_DIN       6 // Data in
#define P8544_CLK       7 // Clock

/* 
 * Pino de Alimentacao do DISPLAY
 */
#define vccDisplay       9   // Pino de alimentacao do LCD
#define GMTLocal        -3      

/* DB */
#define TABLE_SIZE      1024 // Tamanho da Memoria do Arduino
#define RECORDS_TO_CREATE 2 // Tamanho da Tabela de Registros

/* GPS */
#define GPSRX           A0
#define GPSTX           A1

/* SD */
#define CS              10
/************************
        VARIAVEIS
 ************************/

/* GPS */
/* To test whether the data returned is stale, examine the (optional) 
 * parameter “fix_age” which returns the number of milliseconds 
 * since the data was encoded.
 * @Pronto: Implementar fix_age
 * 
 */
boolean fix_valid;
int gpsYear, gpsHdop; // HDOP (Horizontal Dilution of Precision),  It is a measure based solely on the geometry of the satellites
byte gpsMonth, gpsDay, gpsHourGMT0, gpsHourGMTLocal, gpsMinute, gpsSecond, gpsHundredths;
unsigned long chars, fix_age;
unsigned short sentences, failed_checksum;

float gpsLatitude, gpsLongitude = 0.000000;
float gpsAltitude, gpsCurso, gpsVelocidade, gpsSateletes = 0;
/* Serial */
extern HardwareSerial Serial;

/* DB */
struct Waypoints {
    int id;
    char nomeWaypoint[15];
    float altWaypoint;
    float latWaypoint;
    float lonWaypoint;
}

waypoint;

byte showWaypointID = 0;

/* SD */
int num, point = 0;
String dataString = "";
char buff[25];
//char fileName1[30] = "";
//static char dirName[5] = "logs";
//static char prefixoFileName1[5] = "LOG-";
//static char extFileName[5] = ".csv";
//static char cabecalho[35] = "name,elevation,latitude,longitude";

/************************
        INSTANCIAS
 ************************/


/*
 * CONSTRUTORES
 */

/* Este bloco deve ser declarado nesta ordem
 * pois e dependencia para a instanciacao da
 * classe DB */
void writer(unsigned long address, byte data) {
    EEPROM.write(address, data);
}

byte reader(unsigned long address) {
    return EEPROM.read(address);
}

EDB db(&writer, &reader);

TinyGPS gps;

SoftwareSerial serialgps(GPSRX, GPSTX);
Adafruit_PCD8544 display = Adafruit_PCD8544(P8544_CLK, P8544_DIN, P8544_DC, P8544_CS, P8544_RST);

/************************
        FUNCOES
 ************************/
/* GPS */
void atualizaGPS();
void printDebugGps();

static bool feedgps();
static void displayFormatStr(const char *str, int len);
static void displayFormatInt(unsigned long val, unsigned long invalid, int len);
static void displayFormatFloat(float val, float invalid, int len, int prec);

/* DISPLAY */
void turnONDisplay(); // Energiza pino do Display
void initDisplay(); // Envia comando de inicialização ao Display
void displayPCDTela_0(); // Tela default do GPS
void displayPCDTela_1(); // Tela de Navegacao
void displayPCDTela_2(); // Tela de Manutencao dos Waypoints
void displayPCDTela_10(); // Tela sem dados validos


/* DB */
byte reader(unsigned long address);
void writer(unsigned long address, byte data);
void initDB();
void initDados();
void gravaRegistro();
void selectRegistro(byte indice);
void printError(EDB_Status err);
void truncaDB();
void selectAll();

/* SD */
//void initSD();
//void gravaDadosSD();

/************************
        EXECUCAO
 ************************/

void setup() {

    if (useSerial) {
        Serial.begin(115200);
    }
    if (useGPS) {
        serialgps.begin(4800);
        if ((debugGPS) & (debugSerial)) {
            Serial.println(F("Iniciando GPS..."));
        }
    }
    if (usaPcd8544) {
        /* Ajuste Vcc do Display*/
        turnONDisplay(); // Energiza pino do Display
        initDisplay(); // Envia comando de inicializacao ao Display

    }
    /* DB */
    if (usaDB) {
        if ((debugDB) & (debugSerial)) Serial.println(F("Iniciando DB..."));
        initDB();
        gravaRegistro();
        selectAll();
    }
    /* SD */
//    if (usaSD) {
//        if ((debugSD)& (debugSerial)) Serial.println(F("Iniciando SD..."));
//        initSD();
//    }
}

void loop() {

    atualizaGPS();
    if ((debugGPS)& (debugSerial)) printDebugGps();
    if (usaPcd8544) { // Caso esteja usando o DISPLAY...
        display.clearDisplay();
        displayPCDTela_0(); // entro na tela do Altimetro 

    }
    //gravaDadosSD();
}

/********************* 
 *      GPS 
 *********************/

void atualizaGPS() {
    // @Pronto: Ajustar o GMT -3
    // @Melhoria: Oraganizar coleta de Informacao, coletar tudo de uma unica vez, como? 
    // @Melhoria: Incluir selecao de satelites
    /*
 //Here you can put in the time difference from GMT so we
 //can later convert to local time
 int timeDiff = 4;
 
 Convert the given time into our local time
     
 int localHour = hour - timeDiff;
 
 You can get negative hours from the previous subtraction
 In this case, just add 24 hours to be within a valid range again
 if (localHour < 0)
{
   localHour = localHour + 24;
 }
     */

    serialgps.listen();
    while (serialgps.available()) {
        int c = serialgps.read();
        if (gps.encode(c)) {

            gpsHdop = gps.hdop();
            gps.f_get_position(&gpsLatitude, &gpsLongitude, &fix_age);
            (fix_age == TinyGPS::GPS_INVALID_AGE) ? fix_valid = false : fix_valid = true;
            gps.crack_datetime(&gpsYear, &gpsMonth, &gpsDay, &gpsHourGMT0, &gpsMinute, &gpsSecond, &gpsHundredths);
            //converteHourGMT(gpsHourGMT0);
            /* Ajusta a hora para o GMT Local*/
            gpsHourGMTLocal = gpsHourGMT0 + GMTLocal;
            if (gpsHourGMTLocal < 0) gpsHourGMTLocal = gpsHourGMTLocal + 24;
            gpsAltitude = gps.f_altitude();
            gpsCurso = gps.f_course();
            gpsVelocidade = gps.f_speed_kmph();
            gpsSateletes = gps.satellites();
            gps.stats(&chars, &sentences, &failed_checksum);

        }
    }
}

void printDebugGps() {
    if ((debugGPS)& (debugSerial)) {
        Serial.print(F("Lat/Long: "));
        Serial.print(gpsLatitude, 5);
        Serial.print(F(", "));
        Serial.println(gpsLongitude, 5);

        Serial.print(F("Date: "));
        Serial.print(gpsMonth, DEC);
        Serial.print(F("/"));
        Serial.print(gpsDay, DEC);
        Serial.print(F("/"));
        Serial.print(gpsYear);
        Serial.print(F(" Time: "));
        Serial.print(gpsHourGMTLocal, DEC);
        Serial.print(F(":"));
        Serial.print(gpsMinute, DEC);
        Serial.print(F(":"));
        Serial.print(gpsSecond, DEC);
        Serial.print(F("."));
        Serial.println(gpsHundredths, DEC);

        Serial.print(F("Altitude (meters): "));
        Serial.println(gpsAltitude);

        Serial.print(F("Course (degrees): "));
        Serial.println(gps.f_course());

        Serial.print(F("Speed(kmph): "));
        Serial.println(gpsVelocidade);

        Serial.print(F("Satellites: "));
        Serial.println(gpsSateletes);
        Serial.println();
    }
}

static bool feedgps() {
    while (serialgps.available()) {
        if (gps.encode(serialgps.read()))
            return true;
    }
    return false;
}

/********************* 
 *      DISPLAY 
 *********************/

/* Liga o Display */
void turnONDisplay() {
    pinMode(vccDisplay, OUTPUT);
    digitalWrite(vccDisplay, LOW);
}

/* Inicializa o Display */
void initDisplay() {
    display.begin();
    display.setContrast(30);
    display.clearDisplay();
}

/* Tela do GPS */
void displayPCDTela_0() {
    //if (!fix_valid) {
    //    displayPCDTela_10();
    //} else {
    display.setTextSize(1);
    display.setTextColor(BLACK);
    display.setCursor(0, 0);
    // Sateletes
    //display.print(gpsSateletes, 0);
    displayFormatInt(gpsSateletes, TinyGPS::GPS_INVALID_SATELLITES, 3);
    //display.print("-");
    // Data e Hora
    display.print(gpsHourGMTLocal);
    display.print(F(":"));
    display.print(gpsMinute);
    //display.print(":");
    //display.print(gpsSecond);
    //display.print(" ");
    //display.print(gpsDay);
    //display.print("/");
    //display.print(gpsMonth);
    display.print(F(" "));

    displayFormatInt(gpsHdop, TinyGPS::GPS_INVALID_HDOP, 5);
    //display.print("/");
    //display.print(gpsYear);    

    display.drawLine(0, 8, 84, 8, BLACK);
    //******** Linha Horizontal ********
    // Area do meio, mostro a Altura corrente
    display.setTextSize(1);
    display.setTextColor(BLACK);
    display.setCursor(10, 14);
    //display.print((int) gpsAltitude);
    displayFormatFloat(gpsAltitude, TinyGPS::GPS_INVALID_F_ALTITUDE, 6, 2);
    display.print(F("Mts"));

    display.setCursor(10, 24);
    //display.print((int) gpsVelocidade);
    displayFormatFloat(gpsVelocidade, TinyGPS::GPS_INVALID_F_SPEED, 6, 2);
    display.setTextSize(1);
    display.print(F("Kmph"));

    display.setCursor(0, 28);
    //display.print((int) gpsCurso);
    //displayFormatStr(gps.f_course() == TinyGPS::GPS_INVALID_F_ANGLE ? "*** " : TinyGPS::cardinal(gps.f_course()), 6);
    //display.print(F(" Graus"));


    //****** Linha Horizontal ********
    display.drawLine(0, 38, 84, 38, BLACK); // Linha Horizontal    
    display.setTextSize(1);
    display.setTextColor(BLACK);
    display.setCursor(0, 40);
    //Latitude e Longitude
    //display.print((float)gpsLatitude, 4);
    //displayFormatFloat(gpsLatitude, TinyGPS::GPS_INVALID_F_ANGLE, 6, 5);
    // @Pronto: Alterar gps.f_course() por variavel global
    displayFormatFloat(gpsCurso, TinyGPS::GPS_INVALID_F_ANGLE, 7, 2);
    displayFormatStr(gpsCurso == TinyGPS::GPS_INVALID_F_ANGLE ? "*** " : TinyGPS::cardinal(gps.f_course()), 5);
    //display.print(F("Grs"));
    //display.print((float)gpsLongitude, 4);
    //displayFormatFloat(gpsLongitude, TinyGPS::GPS_INVALID_F_ANGLE, 5, 5);
    /*  */
    //display.setCursor(27, 40);
    //if (volume == 1) display.write((char) 14);
    /* Altitude Minima */
    //display.setCursor(37, 40);
    //display.print((int) altitudeMin);

    //}
    /* Rederiza */
    display.display();
}

void displayPCDTela_1() {
    if (!fix_valid) {
        displayPCDTela_10();
    } else {

        display.setTextSize(1);
        display.setTextColor(BLACK);
        display.setCursor(0, 0);
        // Posicao Atual
        displayFormatFloat(gps.f_course(), TinyGPS::GPS_INVALID_F_ANGLE, 7, 2);
        displayFormatStr(gps.f_course() == TinyGPS::GPS_INVALID_F_ANGLE ? "*** " : TinyGPS::cardinal(gps.f_course()), 4);
        displayFormatInt(gpsHdop, TinyGPS::GPS_INVALID_HDOP, 5);
        //******** Linha Horizontal ********
        display.drawLine(0, 8, 84, 8, BLACK);
        //******** Linha Horizontal ********

        //Area central, mostro a Latitude e Longitude corrente
        display.setTextSize(1);
        display.setTextColor(BLACK);
        display.setCursor(0, 14);
        //display.print((int) gpsAltitude);
        display.print(F("Lat: "));
        displayFormatFloat(gpsLatitude, TinyGPS::GPS_INVALID_F_ANGLE, 9, 5);

        display.setCursor(0, 24);
        display.print(F("Lon: "));
        displayFormatFloat(gpsLongitude, TinyGPS::GPS_INVALID_F_ANGLE, 10, 5);

        //****** Linha Horizontal ********
        display.drawLine(0, 38, 84, 38, BLACK); // Linha Horizontal    
        //****** Linha Horizontal ********

        display.setTextSize(1);
        display.setTextColor(BLACK);
        display.setCursor(0, 40);

        displayFormatFloat(gps.f_course(), TinyGPS::GPS_INVALID_F_ANGLE, 7, 2);
        displayFormatStr(gps.f_course() == TinyGPS::GPS_INVALID_F_ANGLE ? "*** " : TinyGPS::cardinal(gps.f_course()), 5);

        display.display();
    }
}

void displayPCDTela_10() {
    display.setTextSize(2);
    display.setTextColor(BLACK);
    display.setCursor(0, 0);
    display.print(F("No Data"));
    display.setCursor(0, 20);
    display.print(fix_age);
    display.print(F(" ms"));
    display.display();
}

static void displayFormatInt(unsigned long val, unsigned long invalid, int len) {
    char sz[32];
    if (val == invalid)
        strcpy(sz, "*******");
    else
        sprintf(sz, "%ld", val);
    sz[len] = 0;
    for (int i = strlen(sz); i < len; ++i)
        sz[i] = ' ';
    if (len > 0)
        sz[len - 1] = ' ';

    display.print(sz);
    feedgps();
}

static void displayFormatFloat(float val, float invalid, int len, int prec) {
    char sz[32];
    if (val == invalid) {
        strcpy(sz, "*******");
        sz[len] = 0;
        if (len > 0)
            sz[len - 1] = ' ';
        for (int i = 7; i < len; ++i)
            sz[i] = ' ';
        display.print(sz);
    } else {
        display.print(val, prec);
        int vi = abs((int) val);
        int flen = prec + (val < 0.0 ? 2 : 1);
        flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
        for (int i = flen; i < len; ++i)
            display.print(F(" "));
    }

    feedgps();
}

static void displayFormatStr(const char *str, int len) {
    // @Melhoria: Retornar um objeto String
    int slen = strlen(str);
    for (int i = 0; i < len; ++i)
        display.print(i < slen ? str[i] : ' ');
    feedgps();
}

/* 
 * DB 
 */

void initDB() {

    db.open(0);
    if ((debugSerial) && (debugDB)) Serial.println(F("Banco Waypoint aberto!"));

    if (db.count() == 0) {
        db.create(0, TABLE_SIZE, (unsigned int) sizeof (waypoint));
        if ((debugSerial) && (debugDB)) {
            Serial.println(F("Banco Waypoint criado!"));
        }
    } else {
        showWaypointID = db.count();
        selectRegistro(showWaypointID);
    }
}

void printError(EDB_Status err) {
    if ((debugSerial) && (debugDB)) {
        Serial.print(F("ERROR: "));
    }
    switch (err) {
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

void gravaRegistro() {

    waypoint.id = db.count() + 1;
    sprintf(waypoint.nomeWaypoint, "%s", "Teste");
    waypoint.altWaypoint = gpsAltitude;
    waypoint.latWaypoint = gpsLatitude;
    waypoint.lonWaypoint = gpsLongitude;

    //  sprintf(waypoint.dataInicial, "%s", startupDate );
    //  sprintf(waypoint.duracao, "%02dD %02d:%02d:%02d", upitmeDays,uptimeHours,uptimeMinutes,uptimeSeconds);
    //  sprintf(waypoint.acelVertUp, "%01d" , acelVertUp );
    //  sprintf(waypoint.acelVertDown, "%01d" , ( acelVertDown * -1 ) );  
    //  sprintf(waypoint.startupAltitude , "%d" , (char*)startupAltitude );
    //  sprintf(waypoint.shutdownAltitude, "%d" , (char*)altitude );
    //  sprintf(waypoint.altitudeMax , "%d" , (char*)altitudeMax );
    //  sprintf(waypoint.altitudeMin , "%d" , (char*)altitudeMin );

    EDB_Status result = db.appendRec(EDB_REC waypoint);
    if (result != EDB_OK) printError(result);

}

void truncaDB() {
    if ((debugSerial) && (debugDB)) {
        Serial.println(F("Apagar TUDO!"));
    }
    db.clear();
}

void selectRegistro(byte indice) {
    EDB_Status result = db.readRec(indice, EDB_REC waypoint);
    if (result == EDB_OK) {
        if ((debugSerial) && (debugDB)) {
            Serial.print(F("Indice: "));
            Serial.println(indice);
            Serial.print(F(" ID: "));
            Serial.println(waypoint.id);
            Serial.print(F("Nome Waypoint: "));
            Serial.println(waypoint.nomeWaypoint);
            Serial.print(F("Altitude: "));
            Serial.println(waypoint.altWaypoint, 5);
            Serial.print(F("Latitude: "));
            Serial.println(waypoint.latWaypoint, 7);
            Serial.print(F("Longitude: "));
            Serial.println(waypoint.lonWaypoint, 7);

        }
    } else printError(result);
}

void selectAll() {
    if ((debugGPS)& (debugSerial)) {
        for (int recno = 1; recno <= db.count(); recno++) {
            EDB_Status result = db.readRec(recno, EDB_REC waypoint);
            if (result == EDB_OK) {
                Serial.print(F("Recno: "));
                Serial.println(recno);
                Serial.print(F(" ID: "));
                Serial.println(waypoint.id);
                Serial.print(F(" Data Inicial: "));
                Serial.println(waypoint.nomeWaypoint);
                Serial.print(F(" Altitude: "));
                Serial.println(waypoint.altWaypoint);
                Serial.print(F(" Latitude: "));
                Serial.println(waypoint.latWaypoint);
                Serial.print(F(" Longitude: "));
                Serial.println(waypoint.lonWaypoint);

            } else printError(result);
        }
    }
}

/* SD */
//void initSD() {
//
//    if (!SD.begin(CS)) {
//        if ((debugSD) && (debugSerial))
//            Serial.println(F("Falha no cartao!"));
//        //return;
//    }
//
//    /* Resolvendo o diretorio*/
//    Serial.println(F("Criando Diretorio..."));
//    if (SD.mkdir(dirName)) {
//        if ((debugSD) && (debugSerial)) {
//            Serial.print(F("Diretorio ja existe: "));
//            Serial.println(dirName);
//        }
//    }
//
//    sprintf(fileName1, "%s/%s%03d%s", dirName, prefixoFileName1, num, extFileName);
//
//    while (SD.exists(fileName1)) {
//        if ((debugSD) && (debugSerial)) {
//            Serial.print(F("Arquivo ja Existe: "));
//            Serial.println(fileName1);
//        }
//        num++;
//        sprintf(fileName1, "%s/%s%03d%s", dirName, prefixoFileName1, num, extFileName);
//        if ((debugSD) && (debugSerial)) {
//            Serial.print(F("Criando o Arquivo: "));
//            Serial.println(fileName1);
//        }
//    }
//
//    /* Incluindo cabecalho*/
//    File dataFile = SD.open(fileName1, FILE_WRITE);
//    if (dataFile) {
//        dataFile.println(cabecalho);
//        if ((debugSD) && (debugSerial))
//            Serial.println(cabecalho);
//        dataFile.close();
//    } else {
//        if ((debugSD) && (debugSerial))
//            Serial.println(F("\nNao consegui abrir o arquivo de log!"));
//    }
//}

//void gravaDadosSD() {
//
//    point++;
//    dataString = "";
//    dataString.concat(point);
//    dataString.concat(",");
//    dataString.concat(dtostrf(gpsAltitude, 4, 2, buff));
//    dataString.concat(",");
//    dataString.concat(dtostrf(gpsLatitude, 3, 6, buff));
//    dataString.concat(",");
//    dataString.concat(dtostrf(gpsLongitude, 3, 6, buff));
//
//    File dataFile = SD.open(fileName1, FILE_WRITE);
//
//    if (dataFile) {
//        dataFile.println(dataString);
//        if ((debugSD) && (debugSerial)) Serial.println(dataString);
//        dataFile.flush();
//        dataFile.close();
//    } else {
//        if ((debugSD) && (debugSerial))
//            Serial.println(F("\nNao consequi abrir o Arquivo de log!"));
//    }
//}
