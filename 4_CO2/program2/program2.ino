/*
 * M5StackでシリアルでMH-Z19Bを、I2CでBME280読み、LCDに数字で表示する
 */
#include <M5Stack.h>
#include <Wire.h>
#include "bme280_i2c.h"
#include <math.h>

HardwareSerial u2(2);

#define PERIOD 5

#define SDA 21
#define SCL 22

BME280 bme280(BME280_I2C_ADDR_PRIM);

void setup(){
    M5.begin();
    M5.Speaker.write(0); // スピーカーをオフする
    Serial.begin(115200);
    u2.begin(9600);

    Wire.begin(SDA, SCL, 400000);
    M5.Lcd.setTextSize(3);

    bme280.begin(); // BME280の初期化
}

short readMHZ19() {
    char cmd[9] = {0xFF, 0x01, 0x86, 0x0, 0x0, 0x0, 0x0, 0x0, 0x79};
    char res[9];

    for (int i = 0; i < 9; i++) {
        u2.write(cmd[i]);
    }
    int i = 0;
    char checksum = 0;
    while (u2.available() > 0) {
        char c = u2.read();
        res[i++] = c;
        checksum += c;
    }
    checksum += 1;

    return ((checksum == 0) ? res[2] << 8 | res[3] : (-1));
}

uint16_t heat(double value) {
    int r, g, b;
    int col = (int)(( -cos(4 * M_PI * value) / 2 + 0.5) * 255);
    if ( value >= 1.0 )     { r = 255; g = 0;   b = 0; }
    else if (value >= 0.75) { r = 255; g = col; b = 0; }
    else if (value >= 0.5 ) { r = col; g = 255; b = 0; }
    else if (value >= 0.25) { r = 0;   g = 255; b = col; }
    else if (value >= 0.0 ) { r = 0;   g = col; b = 255; }
    else                    { r = 0;   g = 0;   b = 255; }
    return (((r)>>3)<<11) | (((g)>>2)<<5) | ((b)>>3);
}

void loop() {
    struct bme280_data data;

    bme280.get_sensor_data(&data);

    short co2 = readMHZ19();
    if (co2 < 0) {
        Serial.println("readMHZ19 failed");
    } else {
        Serial.printf("t: %0.1f, h: %0.1f, p: %0.1f, co2: %d\r\n",
            data.temperature, data.humidity, data.pressure / 100, co2);
        int low = 400, high = 1200;
        int c = map(constrain(co2, low, high), low, high, 50, 100);
        uint16_t backcolor = heat((double)c / 100.0);
        uint16_t textcolor = heat((double)((c + 25) % 100) / 100.0);
        M5.Lcd.fillScreen(backcolor);
        M5.Lcd.setTextColor(textcolor, backcolor);
        M5.Lcd.setCursor(20, 30);
        M5.Lcd.printf("temp: %4.1f'C", data.temperature);
        M5.Lcd.setCursor(20, 80);
        M5.Lcd.printf("humid: %4.1f%%", data.humidity);
        M5.Lcd.setCursor(20, 130);
        M5.Lcd.printf("press: %4.1fhPa", data.pressure / 100);
        M5.Lcd.setCursor(20, 180);
        M5.Lcd.printf("co2: %dppm", co2);
    }

    delay(PERIOD * 1000);
}

