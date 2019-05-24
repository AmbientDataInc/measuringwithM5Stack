/*
 * M5Stack+ADMP401を使い、SAMPLING_FREQUENCYで音を測定し、値をシリアルにプリント。
 */
#include <M5Stack.h>

#define MIC 36

#define SAMPLES 500
#define SAMPLING_FREQUENCY 40000  // 40kHz

void setup() {
    M5.begin();
    Serial.begin(115200);
    while (!Serial) ;

    unsigned int sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY));

    pinMode(MIC, INPUT);

    M5.lcd.setBrightness(0);  // LCDバックライトを消す
    delay(1000);

    int buff[SAMPLES];

    for (int i = 0; i < SAMPLES; i++) {
        unsigned long t = micros();
        buff[i] = analogRead(MIC);
        while ((micros() - t) < sampling_period_us) ;
    }
    M5.lcd.setBrightness(80);  // LCDバックライトを戻す

    for (int i = 0; i < SAMPLES; i++) {
        Serial.println(buff[i]);
    }
}

void loop() {
}
