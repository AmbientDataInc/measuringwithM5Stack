/*
 * M5Stackで50ミリ秒マイクの値を読み、ピークツーピーク値を計算し、プリント。
 */
#include <M5Stack.h>

#define MIC 36

const int sampleWindow = 50;

void setup() {
    M5.begin();
    Serial.begin(115200);
    while (!Serial) ;
    M5.lcd.setBrightness(20);  // LCDバックライトの輝度を下げる

    pinMode(MIC, INPUT);
}

void loop() {
    unsigned long t = millis();
    unsigned int sample;
    unsigned int sMax = 0, sMin = 4095;

    while (millis() - t < sampleWindow) {
        sample = analogRead(MIC);
        if (sample > sMax) sMax = sample;
        if (sample < sMin) sMin = sample;
    }
    float volts = (float)(sMax - sMin) / 4095.0 * 3.6;
    Serial.println(volts);
    delay(100);
}
