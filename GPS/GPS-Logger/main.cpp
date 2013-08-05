#include <Arduino.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <SD.h>
#include <stdlib.h>

#define GPSRX   A0
#define GPSTX   A1

#define CS      10

TinyGPS gps;

SoftwareSerial serialgps(GPSRX, GPSTX);

int c = 0 ;
char buff[25] ;

int year;
int sequencial = 0; // Usado na geracao do nome do arquivo
byte month, day, hour, minute, second, hundredths;
unsigned long chars;
unsigned short sentences, failed_checksum;
extern HardwareSerial Serial;
float altitude,latitude, longitude = 0.000000;
//Define  String
//String SD_date_time = "invalid";
//String SD_lat = "invalid";
//String SD_lon = "invalid";
String dataString = "";
static char cabecalho[35] = "name,elevation,latitude,longitude";
//static String cabecalho = "name,elevation,latitude,longitude";

/* Cartao */
char fileName1[30] = "";
static char dirName[5] = "logs";
static char prefixoFileName1[5] = "LOG-";
static char extFileName[5] = ".csv";

void initSD();
void gravaDadosSD();

void setup() {


    Serial.begin(115200);
    serialgps.begin(4800);
    Serial.println(F("GPS Iniciado!"));

    //Connect to the SD Card
    pinMode(CS, OUTPUT);
    initSD();

}

void loop() {

    serialgps.listen();
    while (serialgps.available()) {
        int c = serialgps.read();
        if (gps.encode(c)) {

            gps.f_get_position(&latitude, &longitude);

            Serial.print(F("Lat/Long: "));
            Serial.print(latitude, 7);
            Serial.print(F(", "));
            Serial.println(longitude, 7);

            gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths);
            Serial.print(F("Date: "));
            Serial.print(month, DEC);
            Serial.print(F("/"));
            Serial.print(day, DEC);
            Serial.print(F("/"));
            Serial.print(year);
            Serial.print(F(" Time: "));
            Serial.print(hour, DEC);
            Serial.print(F(":"));
            Serial.print(minute, DEC);
            Serial.print(F(":"));
            Serial.print(second, DEC);
            Serial.print(F("."));
            Serial.println(hundredths, DEC);
            Serial.print(F("Altitude (meters): "));
            altitude = gps.f_altitude();
            Serial.println(altitude);
            Serial.print(F("Course (degrees): "));
            Serial.println(gps.f_course());
            Serial.print(F("Speed(kmph): "));
            Serial.println(gps.f_speed_kmph());
            Serial.print(F("Satellites: "));
            Serial.println(gps.satellites());
            Serial.println();
            gps.stats(&chars, &sentences, &failed_checksum);
        }
    }
    
    gravaDadosSD();
    delay(10);
}

void initSD() {
    if (!SD.begin(CS)) {
        Serial.println("Falha no cartao!");
        return;
    } else {
        Serial.println("Cartao OK!");        
    }

    /* Resolvendo o diretorio*/
    Serial.println("Criando Diretorio...");
    if (SD.mkdir(dirName)) {
        Serial.print("Diretorio ja existe: ");
        Serial.println(dirName);
    }
    
    sprintf(fileName1, "%s/%s%03d%s", dirName, prefixoFileName1, sequencial, extFileName);
    Serial.print("Nome do arquivo: " );
    Serial.println(fileName1);
    while (SD.exists(fileName1)) {
        Serial.print("Arquivo ja Existe: ");
        Serial.println(fileName1);
        sequencial++;
        sprintf(fileName1, "%s/%s%03d%s", dirName, prefixoFileName1, sequencial, extFileName);
        Serial.print("Criando o Arquivo: ");
        Serial.println(fileName1);
    }

    /* Incluindo cabecalho*/
    File dataFile = SD.open(fileName1, FILE_WRITE);
    if (dataFile) {
        dataFile.println(cabecalho);
        dataFile.flush();
        dataFile.close();
        Serial.println("Cabecalho gravado!");
    } else {
        Serial.println("Nao consegui abrir o arquivo de log!");
    }
}

void gravaDadosSD() {
    
    c++ ;    
    dataString = "" ;
    dataString.concat(c);
    dataString.concat( "," );    
    dataString.concat( dtostrf(altitude,4,2, buff));
    dataString.concat( "," );
    dataString.concat( dtostrf(latitude,3,6, buff));
    dataString.concat( "," );
    dataString.concat( dtostrf(longitude,3,6,buff));

    
    File dataFile = SD.open(fileName1, FILE_WRITE);
 
    if (dataFile) {
        dataFile.println(dataString);
        Serial.println(dataString);
        dataFile.flush();
        dataFile.close();
    } else {
        Serial.println(F("Nao consegui abrir o arquivo de log!"));
    }
}
