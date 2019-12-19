#include <M5Stack.h>
#include "HCSR04.h"
#include <Wire.h>
#include <VL53L0X.h>
#include <LIDARLite_v3HP.h>

int Trig = 2;
int Echo = 5;

HCSR04 hcsr04;

VL53L0X vl53l0x;

LIDARLite_v3HP myLidarLite;

void setup() {
    M5.begin();
    Serial.begin(115200);
    while (!Serial) ;

    hcsr04.begin(Trig, Echo);

    pinMode(21, INPUT_PULLUP);  // SDAをプルアップする
    pinMode(22, INPUT_PULLUP);  // SDAをプルアップする
    Wire.begin();
    Wire.setClock(400000UL); // Set I2C frequency to 400kHz (for Arduino Due)

    vl53l0x.init();
    vl53l0x.setTimeout(500);
    vl53l0x.startContinuous();

    myLidarLite.configure(0);

    M5.Lcd.setTextSize(3);
}

void loop() {
    float HCSR04_d = hcsr04.distance();
    Serial.printf("%.0f ", HCSR04_d * 10);

    uint16_t VL53L0X_d = 0;
    for (int i = 0; i < 10; i++) {
        VL53L0X_d += vl53l0x.readRangeContinuousMillimeters();
    }
    VL53L0X_d /= 10;
    Serial.printf("%d ", VL53L0X_d);

    float v3HP_d = 0;
    for (int i = 0; i < 20; i++) {
        myLidarLite.waitForBusy();
        myLidarLite.takeRange();
        v3HP_d += myLidarLite.readDistance();
    }
    v3HP_d /= 20;
    Serial.printf("%.0f\r\n", v3HP_d * 10);

    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(20, 50);
    M5.Lcd.print("HC-SR04:");
    M5.Lcd.setCursor(170, 50);
    M5.Lcd.printf("%.0f mm", HCSR04_d * 10);

    M5.Lcd.setCursor(20, 120);
    M5.Lcd.print("VL53L0X:");
    M5.Lcd.setCursor(170, 120);
    M5.Lcd.printf("%d mm", VL53L0X_d);

    M5.Lcd.setCursor(20, 190);
    M5.Lcd.print("v3HP:");
    M5.Lcd.setCursor(170, 190);
    M5.Lcd.printf("%.0f mm", v3HP_d * 10);
}
