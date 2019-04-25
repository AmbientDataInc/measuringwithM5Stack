/*
 * MPU9250を読んでFFTして、周波数成分をLCDに表示する
 * 10ミリ秒間隔で128回サンプリングし、描画
 */

#include <M5Stack.h>
#include "utility/MPU9250.h"
#include "arduinoFFT.h"

MPU9250 IMU;

#define TIMER0 0
#define SAMPLE_PERIOD 10    // サンプリング間隔(ミリ秒)

const uint16_t FFTsamples = 128; //This value MUST ALWAYS be a power of 2
double vReal[FFTsamples];
double vImag[FFTsamples];
const double samplingFrequency = 1 / ((double)SAMPLE_PERIOD / 1000.0); // 100Hz
arduinoFFT FFT = arduinoFFT(vReal, vImag, FFTsamples, samplingFrequency);  // FFTオブジェクトを作る


hw_timer_t * samplingTimer = NULL;

volatile int t0flag;

void IRAM_ATTR onTimer0() {
    t0flag = 1;
}

void sample(int nsamples) {
    int16_t accelCount[3];

    t0flag = 0;
    timerAlarmEnable(samplingTimer);
    for (int i = 0; i < nsamples; i++) {
        while (t0flag == 0) {
            delay(0);
        }
        while (!(IMU.readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01)) {
            delay(0);
        }
        IMU.readAccelData(accelCount);  // Read the x/y/z adc values
        vReal[i] = (accelCount[2] * IMU.aRes - IMU.accelBias[2]) * 1000;
        vImag[i] = 0;

        t0flag = 0;
    }
    timerAlarmDisable(samplingTimer);
}

int X0 = 10;
int Y0 = 20;
int _height = 240 - Y0;
int _width = 320;
float dmax = 1000.0;

void drawChart(int nsamples) {
    int band_width = floor(_width / nsamples);
    int band_pad = band_width - 1;

    for (int band = 0; band < nsamples; band++) {
        int hpos = band * band_width + X0;
        float d = vReal[band];
        if (d > dmax) d = dmax;
        int h = (int)((d / dmax) * (_height));
        M5.Lcd.fillRect(hpos, _height - h, band_pad, h, WHITE);
        if ((band % 8) == 0) {
            M5.Lcd.setCursor(hpos, _height + Y0 - 10);
            M5.Lcd.printf("%dHz", (int)((band * 1.0 * samplingFrequency) / FFTsamples));
        }
    }
}

void panic(char * str) {
    Serial.println(str);
    M5.Lcd.fillScreen(RED);
    M5.Lcd.setTextColor(WHITE, RED);
    M5.Lcd.setCursor(0, 32);
    M5.Lcd.print(str);
    while (true) delay(0);
}

void setup() {
    M5.begin();
    Wire.begin();

    byte c = IMU.readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250);
    if (c != 0x71) {
        panic("Can't connect to MPU9250");
    }

    IMU.MPU9250SelfTest(IMU.SelfTest);
    IMU.calibrateMPU9250(IMU.gyroBias, IMU.accelBias);
    IMU.initMPU9250();

    samplingTimer = timerBegin(TIMER0, 80, true);  // divider=80 (1μ秒), countUp=true
    timerAttachInterrupt(samplingTimer, &onTimer0, true);
    timerAlarmWrite(samplingTimer, SAMPLE_PERIOD * 1000, true);

    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(GREEN ,BLACK);
}

void DCRemoval(double *vData, uint16_t samples) {
    double mean = 0;
    for (uint16_t i = 1; i < samples; i++) {
        mean += vData[i];
    }
    mean /= samples;
    for (uint16_t i = 1; i < samples; i++) {
        vData[i] -= mean;
    }
}

int count = 0;
void loop() {
    if (count++ < 3) {
    IMU.getAres();
    sample(FFTsamples);

    DCRemoval(vReal, FFTsamples);
    FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);  // 窓関数
    FFT.Compute(FFT_FORWARD); // FFT処理(複素数で計算)
    FFT.ComplexToMagnitude(); // 複素数を実数に変換
    double x = FFT.MajorPeak();

    M5.Lcd.fillScreen(BLACK);
    drawChart(FFTsamples / 2);
    M5.Lcd.setCursor(40, 0);
    M5.Lcd.printf("Peak: %.0fHz", x);
    }
}
