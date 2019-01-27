/*
 * M5StackとBME280をI2C接続し、温度、湿度、気圧を測定しプリントアプトする
 */
#include <M5Stack.h>
#include <Wire.h>
#include "bme280_i2c.h"

#define SDA 21
#define SCL 22

BME280 bme280(BME280_I2C_ADDR_PRIM);

void setup(){
    M5.begin();

    Wire.begin(SDA, SCL, 400000);
    pinMode(SDA, INPUT_PULLUP); // SDAピンのプルアップの指定
    pinMode(SCL, INPUT_PULLUP); // SCLピンのプルアップの指定
    Serial.begin(115200);
    M5.Lcd.setTextSize(3);

    bme280.begin(); // BME280の初期化
}

void loop() {
    struct bme280_data data;

    bme280.get_sensor_data(&data);
    Serial.printf("%0.2f, %0.2f, %0.2f\r\n",
        data.temperature, data.humidity, data.pressure / 100);
    M5.Lcd.setCursor(30, 40);
    M5.Lcd.printf("temp: %4.1f'C", data.temperature);
    M5.Lcd.setCursor(30, 100);
    M5.Lcd.printf("humid: %4.1f%%", data.humidity);
    M5.Lcd.setCursor(30, 160);
    M5.Lcd.printf("press: %4.1fhPa", data.pressure / 100);

    delay(5000);
}

