#include <Arduino.h>
#include <SoftwareSerial.h>
#include <SD.h>
#include <stdlib.h>

#define CS      10

int year;
int num = 0;
byte month, day, hour, minute, second, hundredths;
unsigned long chars;
unsigned short sentences, failed_checksum;
extern HardwareSerial Serial;

//Define String
String SD_date_time = "invalid";
String SD_lat = "invalid";
String SD_lon = "invalid";
String dataString = "";
String cabecalho = "name,elevation,latitude,longitude";

//String fileName = "LOG-" ;
//String fileExt = ".csv"
char fileName1[30]              = "";
static char dirName[5]          = "logs";
static char prefixoFileName1[5] = "LOG-";
static char extFileName[5]      = ".csv";

void initSD();
void gravaDadosSD();

void setup() {

    Serial.begin(115200);

    pinMode(CS, OUTPUT);
    //Connect to the SD Card
    Serial.println("Inciando o cartao SD");
    initSD();
}

void loop() {
    gravaDadosSD();
}

void initSD() {

    if (!SD.begin(CS)) {
        Serial.println("Falha no cartao!");
        return;
    }

    /* Resolvendo o diretorio*/
    Serial.println("Criando Diretorio...");
    if (SD.mkdir(dirName)) {
        Serial.print("Diretorio ja existe: ");
        Serial.println(dirName);
    }

    sprintf(fileName1, "%s/%s%03d%s", dirName, prefixoFileName1, num, extFileName);
    
    while (SD.exists(fileName1)) {
        Serial.print("Arquivo ja Existe: ");
        Serial.println(fileName1);
        num++;
        sprintf(fileName1, "%s/%s%03d%s", dirName, prefixoFileName1, num, extFileName);
        Serial.print("Criando o Arquivo: ");
        Serial.println(fileName1);
    }

    /* Incluindo cabecalho*/
    File dataFile1 = SD.open(fileName1, FILE_WRITE);
    if (dataFile1) {
        dataFile1.println(cabecalho);
        dataFile1.flush();
        dataFile1.close();
    } else {
        Serial.println("\nCouldn't open the log file!");
    }
}
int c = 0 ;
char buff[15];
float f = -10.000001 ;

void gravaDadosSD() {
    c++ ;
    f += -00.000001 ;
    
    dataString = "" ;
    dataString.operator += (c) ;
    dataString += "," ;
    dataString.operator +=(millis());
    dataString += "," ;
    dataString.concat(dtostrf(f,3,6,buff));    
    
    File dataFile = SD.open(fileName1, FILE_WRITE);
    if (dataFile) {
        dataFile.println(dataString);
        Serial.println(dataString);        
        dataFile.close();
    } else {
        Serial.println("\nCouldn't open the log file!");
    }
}
