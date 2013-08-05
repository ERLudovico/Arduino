#include <Arduino.h>
#include <EDB.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>

#define TABLE_SIZE 1024 // Arduino 328
#define writeModeON  true
// The number of demo records that should be created.  This should be less 
// than (TABLE_SIZE - sizeof(EDB_Header)) / sizeof(logBook).  If it is higher, 
// operations will return EDB_OUT_OF_RANGE for all records outside the usable range.
#define RECORDS_TO_CREATE 10

// Arbitrary record definition for this table.  
// This should be modified to reflect your record needs.

struct Voo {
    
    int id;
    char dataInicial[20];
    char dataFinal[20];
    int maiorAltitude;
    int menorAltitude;
    byte maiorAceleracaoAsce;
    byte maiorAceleracaoDesce;
    float temperatura;

}
logBook;

extern HardwareSerial Serial;

// The read and write handlers for using the EEPROM Library

void writer(unsigned long address, byte data) {
    EEPROM.write(address, data);
}

byte reader(unsigned long address) {
    return EEPROM.read(address);
}

// Create an EDB object with the appropriate write and read handlers
EDB db(&writer, &reader);

void recordLimit();
void deleteOneRecord(int recno);
void deleteAll();
void countRecords();
void createRecords(int num_recs);
void selectAll();
void updateOneRecord(int recno);
void insertOneRecord(int recno);
void appendOneRecord(int id);
void printError(EDB_Status err);

void setup() {

    Serial.begin(115200);
    Serial.println("Extended Database Library + Arduino Internal EEPROM Demo");
    Serial.println();

    randomSeed(analogRead(0));

    Serial.print("Creating table...");
    // create table at with starting address 0
    //db.create(0, TABLE_SIZE, (unsigned int)sizeof(logBook));
    db.open(0);
    Serial.println("DONE");

    recordLimit();
    countRecords();
    selectAll();
    if (writeModeON) {
        createRecords(RECORDS_TO_CREATE);
        countRecords();
        selectAll();
        deleteOneRecord(RECORDS_TO_CREATE / 2);
        countRecords();
        selectAll();
        appendOneRecord(RECORDS_TO_CREATE + 1);
        countRecords();
        selectAll();
        insertOneRecord(RECORDS_TO_CREATE / 2);
        countRecords();
        selectAll();
        updateOneRecord(RECORDS_TO_CREATE);
        selectAll();
        countRecords();
        deleteAll();
        Serial.println("Use insertRec() and deleteRec() carefully, they can be slow");
        countRecords();
        for (int i = 1; i <= 2; i++) insertOneRecord(1); // inserting from the beginning gets slower and slower
        countRecords();
        selectAll();
        for (int i = 1; i <= 20; i++) deleteOneRecord(1); // deleting records from the beginning is slower than from the end
        countRecords();
        selectAll();
    }


}

void loop() {


}

// utility functions

void recordLimit() {
    Serial.print("Record Limit: ");
    Serial.println(db.limit());
}

void deleteOneRecord(int recno) {
    Serial.print("Deleting recno: ");
    Serial.println(recno);
    db.deleteRec(recno);
}

void deleteAll() {
    Serial.print("Truncating table...");
    db.clear();
    Serial.println("DONE");
}

void countRecords() {
    Serial.print("Record Count: ");
    Serial.println(db.count());
}

void createRecords(int num_recs) {
    Serial.print("Creating Records...");
    for (int recno = 1; recno < num_recs; recno++) {
        int m = random(1, 12);
        int d = random(1, 31);
        int maiorAltitude = random(1200, 1700);
        int menorAltitude = random(100, 200);
        byte maiorAceleracaoAsce = random(2, 7);
        byte maiorAceleracaoDesce = random(3, 0);
        float temp = -1.2345678 ;
        logBook.id = recno;

        sprintf(logBook.dataInicial, "2009-%02d-%02d", m, d);
        sprintf(logBook.dataFinal, "2009-%02d-%02d", m, d);
        logBook.maiorAltitude = maiorAltitude;
        logBook.menorAltitude = menorAltitude;
        logBook.maiorAceleracaoAsce = maiorAceleracaoAsce;
        logBook.maiorAceleracaoDesce = maiorAceleracaoDesce;      
        logBook.temperatura = temp ;
        EDB_Status result = db.appendRec(EDB_REC logBook);
        if (result != EDB_OK) printError(result);
    }
    Serial.println("DONE");
}

void selectAll() {
    for (int recno = 1; recno <= db.count(); recno++) {
        EDB_Status result = db.readRec(recno, EDB_REC logBook);
        if (result == EDB_OK) {
            Serial.print("Recno: ");
            Serial.println(recno);
            Serial.print(" ID: ");
            Serial.println(logBook.id);
            Serial.print(" Data Inicial: ");
            Serial.println(logBook.dataInicial);
            Serial.print(" Data Final: ");
            Serial.println(logBook.dataFinal);
            Serial.print(" maiorAltitude: ");
            Serial.println(logBook.maiorAltitude);
            Serial.print(" menorAltitude: ");
            Serial.println(logBook.menorAltitude);
            Serial.print(" maiorAceleracaoAsce: ");
            Serial.println(logBook.maiorAceleracaoAsce);
            Serial.print(" maiorAceleracaoDesce: ");
            Serial.println(logBook.maiorAceleracaoDesce);
            Serial.print(" Temperatura: ");
            Serial.println(logBook.temperatura, 6);

        } else printError(result);
    }
}

void updateOneRecord(int recno) {
    Serial.print("Updating record at recno: ");
    Serial.print(recno);
    Serial.print("...");
    logBook.id = 1234;
    EDB_Status result = db.updateRec(recno, EDB_REC logBook);
    if (result != EDB_OK) printError(result);
    Serial.println("DONE");
}

void insertOneRecord(int recno) {
    Serial.print("Inserting record at recno: ");
    Serial.print(recno);
    Serial.print("...");
    logBook.id = recno;
    EDB_Status result = db.insertRec(recno, EDB_REC logBook);
    if (result != EDB_OK) printError(result);
    Serial.println("DONE");
}

void appendOneRecord(int id) {
    Serial.print("Appending record...");
    logBook.id = id;
    EDB_Status result = db.appendRec(EDB_REC logBook);
    if (result != EDB_OK) printError(result);
    Serial.println("DONE");
}

void printError(EDB_Status err) {
    Serial.print("ERROR: ");
    switch (err) {
        case EDB_OUT_OF_RANGE:
            Serial.println("Recno out of range");
            break;
        case EDB_TABLE_FULL:
            Serial.println("Table full");
            break;
        case EDB_OK:
        default:
            Serial.println("OK");
            break;
    }
}

