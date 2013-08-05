#include <Arduino.h>
#include <IRremote.h>

#define debugIR false

long lastDecodeSign = 0;

extern HardwareSerial Serial;
int RECV_PIN = 10;
IRrecv irrecv(RECV_PIN);
decode_results results;


void dump(decode_results *results);
void classificaComando(decode_results *results);


void setup() {
    Serial.begin(115200);
    irrecv.enableIRIn(); // Start the receiver
}

void loop() {
    if (irrecv.decode(&results)) {
        //Serial.println(results.value, HEX);
        if (debugIR) dump(&results);
        classificaComando(&results);
        irrecv.resume(); // Receive the next value
    }
}

void classificaComando(decode_results *results){
    if (results->decode_type == NEC) {
        long decodeSign = results->value;
//        Serial.print(" Decoded: ");
//        Serial.print(decodeSign);
//        Serial.println(" : ");
        if (decodeSign == -1) decodeSign = lastDecodeSign ;
        
        lastDecodeSign = decodeSign ;
        switch (decodeSign) {
            case 16753245:
                Serial.println(" Power ON ");
                break;
            case 16736925:
                Serial.println(" Mode ");
                break;
            case 16769565:
                Serial.println(" Mute ");
                break;
            case 16720605:
                Serial.println(" Play/Pause ");
                break;
            case 16712445:
                Serial.println(" Volta ");
                break;
            case 16761405:
                Serial.println(" Adianta ");
                break;
            case 16769055:
                Serial.println(" EQ ");
                break;
            case 16754775:
                Serial.println(" Menos ");
                break;
            case 16748655:
                Serial.println(" Mais ");
                break;
            case 16738455:
                Serial.println(" Zero ");
                break;
            case 16750695:
                Serial.println(" Inverte ");
                break;
            case 16756815:
                Serial.println(" USD");
                break;
            case 16724175:
                Serial.println(" UM ");
                break;
            case 16718055:
                Serial.println(" DOIS ");
                break;
            case 16743045:
                Serial.println(" TRES ");
                break;
            case 16716015:
                Serial.println(" QUATRO ");
                break;
            case 16726215:
                Serial.println(" CINCO ");
                break;
            case 16734885:
                Serial.println(" SEIS ");
                break;
            case 16728765:
                Serial.println(" SETE ");
                break;
            case 16730805:
                Serial.println(" OITO ");
                break;
            case 16732845:
                Serial.println(" NOVE ");
                break;
        }
    }    
}

void dump(decode_results *results) {
    
        int count = results->rawlen;
        if (results->decode_type == UNKNOWN) {
            Serial.print("Unknown encoding: ");
        } else if (results->decode_type == NEC) {
            Serial.print("Decoded NEC: ");
        } else if (results->decode_type == SONY) {
            Serial.print("Decoded SONY: ");
        } else if (results->decode_type == RC5) {
            Serial.print("Decoded RC5: ");
        } else if (results->decode_type == RC6) {
            Serial.print("Decoded RC6: ");
        } else if (results->decode_type == PANASONIC) {
            Serial.print("Decoded PANASONIC - Address: ");
            Serial.print(results->panasonicAddress, HEX);
            Serial.print(" Value: ");
        } else if (results->decode_type == JVC) {
            Serial.print("Decoded JVC: ");
        }
        Serial.print(results->value, HEX);
        Serial.print(" (");
        Serial.print(results->bits, DEC);
        Serial.println(" bits)");
        Serial.print("Raw (");
        Serial.print(count, DEC);
        Serial.print("): ");

        for (int i = 0; i < count; i++) {
            if ((i % 2) == 1) {
                Serial.print(results->rawbuf[i] * USECPERTICK, DEC);
            } else {
                Serial.print(-(int) results->rawbuf[i] * USECPERTICK, DEC);
            }
            Serial.print(" ");
        }
        Serial.println("");
    


}

