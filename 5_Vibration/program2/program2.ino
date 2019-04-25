/* 
 * MPU9250を読んでLCDに波形を表示する
 * 10ミリ秒間隔で3秒間サンプリングし、描画
 * 描画の縦軸の最小値、最大値を動的に計算する
 */

#include <M5Stack.h>
#include "utility/MPU9250.h"

MPU9250 IMU;

#define DRANGE 80.0  // ダイナミックレンジ。測定対象に合わせて調整してください。

#define TIMER0 0
#define SAMPLE_PERIOD 10    // サンプリング間隔(ミリ秒)
#define SAMPLE_SIZE 300     // 10ms x 300 = 3秒

hw_timer_t * samplingTimer = NULL;

struct accel {
    float ax, ay, az;
};

struct accel axyz[SAMPLE_SIZE];

volatile int t0flag;

void IRAM_ATTR onTimer0() {
    t0flag = 1;
}

#define MINZ 500.0  // 描画時の縦軸の最小値、最大値の初期値
#define MAXZ 1500.0
float minz = MINZ;
float maxz = MAXZ;
float avez;

int data2y(float d, float mind, float maxd) {
    return M5.Lcd.height() - (int)((d - mind) / (maxd - mind) * M5.Lcd.height());
}

#define X0 10  // 横軸の描画開始座標

void drawline(int i) {
    if (i == 0) return;
    M5.Lcd.drawLine(i - 1 + X0, data2y(axyz[i - 1].az, minz, maxz), i + X0, data2y(axyz[i].az, minz, maxz), GREEN);
}

void sample(int nsamples) {
    int16_t accelCount[3];
    float newminz = MAXZ; // 次のminを探す。初期値はMAX。間違いではない。
    float newmaxz = MINZ;

    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.printf("ave: %.1f, min: %.1f, max: %.1f mG", avez, minz, maxz);
    avez = 0.0;
    t0flag = 0;
    timerAlarmEnable(samplingTimer);
    bool first = true;
    for (int i = 0; i < nsamples; i++) {
        while (t0flag == 0) {
            delay(0);
        }
        while (!(IMU.readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01)) {
            delay(0);
        }
        unsigned long t = micros();
        IMU.readAccelData(accelCount);  // Read the x/y/z adc values
        axyz[i].ax = (accelCount[0] * IMU.aRes - IMU.accelBias[0]) * 1000;
        axyz[i].ay = (accelCount[1] * IMU.aRes - IMU.accelBias[1]) * 1000;
        axyz[i].az = (accelCount[2] * IMU.aRes - IMU.accelBias[2]) * 1000;

        if (newminz > axyz[i].az) newminz = axyz[i].az;
        if (newmaxz < axyz[i].az) newmaxz = axyz[i].az;
        avez += axyz[i].az;

        drawline(i);

        if (first) {
            first = false;
            Serial.println(micros() - t);
        }

        t0flag = 0;
    }
    timerAlarmDisable(samplingTimer);
    Serial.printf("newmin: %.2f, newmax: %.2f, drange: %.2f\r\n", newminz, newmaxz, newmaxz - newminz);
    if ((newmaxz - newminz) < DRANGE) {
        minz = newminz - DRANGE / 2;
        maxz = newmaxz + DRANGE / 2;
    } else {
        minz = newminz - (newmaxz - newminz) / 2;
        maxz = newmaxz + (newmaxz - newminz) / 2;
    }
    avez /= nsamples;
}

void panic(char * str) {
    Serial.println(str);
    M5.Lcd.fillScreen(RED);
    M5.Lcd.setTextColor(WHITE, RED);
    M5.Lcd.setCursor(0, 32);
    M5.Lcd.print(str);
    while (true) delay(0);
}

void setup()
{
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

int count = 0;
void loop()
{
    if (count++ < 3) {
    M5.Lcd.fillScreen(BLACK);

    IMU.getAres();
    sample(SAMPLE_SIZE);
    }
}
