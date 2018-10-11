/*
 * GPSモジュールからデータを読み、シリアルに出力する
 */
#include <M5Stack.h>

HardwareSerial GPS_s(2);

void setup() {
    M5.begin();
    dacWrite(25, 0); // Speaker OFF

    Serial.begin(115200);
    delay(20);
    GPS_s.begin(9600);
}

void loop() {
    while (GPS_s.available() > 0) {
        Serial.write(GPS_s.read());
    }
}

