/*
 * M5Stack+ADMP401を使い、SAMPLING_FREQUENCYで音を測定し、値をシリアルにプリント。
 */
#include <M5Stack.h>

#define MIC 36

#define SAMPLES 300
#define SAMPLING_FREQUENCY 40000
unsigned int sampling_period_us;

double buff[SAMPLES];

double minY = 0.0;
double maxY = 3.6;
#define DRANGE 2.0
#define X0 10

int d2y(double d, double minY, double maxY, int HEIGHT) {
    return HEIGHT - ((int)((d - minY) / (maxY - minY) * (float)HEIGHT) + 1);
}

void drawChart() {
    int HEIGHT = M5.Lcd.height();
    double newmin = 3.6;
    double newmax = 0;
    for (int i = 1; i < SAMPLES; i++) {
        int d0 = d2y(buff[i - 1], minY, maxY, HEIGHT);
        int d1 = d2y(buff[i], minY, maxY, HEIGHT);
        M5.Lcd.drawLine(i - 1 + X0, d0, i + X0, d1, WHITE);
        if (newmin > buff[i]) newmin = buff[i];
        if (newmax < buff[i]) newmax = buff[i];
    }
    if ((newmax - newmin) < DRANGE) {
        minY = newmin - DRANGE / 2;
        maxY = newmax + DRANGE / 2;
    } else {
        minY = newmin - (newmax - newmin) / 2;
        maxY = newmin + (newmax - newmin) / 2;
    }
}

void setup() {
    M5.begin();
    Serial.begin(115200);
    while (!Serial) ;
    M5.lcd.setBrightness(20);  // LCDバックライトの輝度を下げる

    sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY));

    pinMode(MIC, INPUT);
}

void loop() {
    delay(500);
    for (int i = 0; i < SAMPLES; i++) {
        unsigned long t = micros();
        buff[i] = (double)analogRead(MIC) / 4095.0 * 3.6 + 0.1132; // ESP32のADCの特性を補正
        while ((micros() - t) < sampling_period_us) ;
    }
    M5.Lcd.fillScreen(BLACK);
    drawChart();
}
