/*
 * GPSモジュールからデータを読み、シリアルに出力する
 */
#include <M5Stack.h>

HardwareSerial GPS_s(2);

void setup() {
    M5.begin();

    Serial.begin(115200);
    while (!Serial) ;
    GPS_s.begin(9600);
}

void loop() {
    while (GPS_s.available() > 0) {
        Serial.write(GPS_s.read());
    }
}

