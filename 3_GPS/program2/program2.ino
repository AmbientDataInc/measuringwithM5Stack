/*
 * TinyGPS++でGPSデーターを解析する。
 */
#include <M5Stack.h>
#include <TinyGPS++.h>

TinyGPSPlus gps;

HardwareSerial ss(2);

void setup()
{
    M5.begin();
    ss.begin(9600);
}

void loop()
{
    smartDelay(1000);
    M5.Lcd.setTextSize(2);
    M5.Lcd.fillScreen(BLACK);
    if (gps.location.isUpdated()) {
        M5.Lcd.setCursor(10, 40);
        M5.Lcd.printf("Lat: %.8f", gps.location.lat());
        M5.Lcd.setCursor(10, 80);
        M5.Lcd.printf("Lng: %.8f", gps.location.lng());
    }
}

static void smartDelay(unsigned long ms) {
    unsigned long start = millis();
        do {
            while (ss.available())
                gps.encode(ss.read());
        } while (millis() - start < ms);
}

