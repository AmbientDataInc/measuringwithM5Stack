/*
 * M5StackとBME280をI2C接続し、温度、湿度、気圧を測定しLCDにグラフ表示し、
 * Ambientに送信して可視化する
 */
#include <M5Stack.h>
#include <Wire.h>
#include "bme280_i2c.h"
#include "Ambient.h"

WiFiClient client;
const char* ssid = "ssid";
const char* password = "password";

Ambient ambient;
unsigned int channelId = 100; // AmbientのチャネルID
const char* writeKey = "writeKey"; // ライトキー

#define SDA 21
#define SCL 22

BME280 bme280(BME280_I2C_ADDR_PRIM);

// 温度、湿度を入れるリングバッファ
#define NDATA 150
struct temphumid {
    float t;
    float h;
} th[NDATA];
int dataIndex = 0;

// リングバッファに温度、湿度データを挿入する
void putData(float t, float h) {
    if (++dataIndex >= NDATA) {
        dataIndex = 0;
    }
    th[dataIndex].t = t;
    th[dataIndex].h = h;
}

#define X0 10

// 温度、湿度の値からy軸の値を計算する
int th2y(float th, float minY, float maxY, int HEIGHT) {
    return HEIGHT - ((int)((th - minY) / (maxY - minY) * (float)HEIGHT) + 1);
}

// リングバッファから温度、湿度を読み、グラフ表示する
void drawChart() {
    int HEIGHT = M5.Lcd.height();
    float mint = 100.0, maxt = -100.0, minh = 100.0, maxh = 0.0;
    for (int i = 0; i < NDATA; i++) {
        if (th[i].h == -1.0) continue;
        if (th[i].t < mint) mint = th[i].t;
        if (th[i].t > maxt) maxt = th[i].t;
        if (th[i].h < minh) minh = th[i].h;
        if (th[i].h > maxh) maxh = th[i].h;
    }
    int minT = (int)mint - 1;
    int maxT = (int)maxt + 1;
    int minH = (int)minh - 10;
    int maxH = (int)maxh + 10;
    Serial.printf("minT: %d, maxT: %d, minH: %d, maxH: %d\r\n", minT, maxT, minH, maxH);
    for (int i = 0, j = dataIndex + 1; i < (NDATA - 1); i++, j++) {
        if (th[j % NDATA].h == -1.0) continue;
        int t0 = th2y(th[j % NDATA].t, minT, maxT, HEIGHT);
        int t1 = th2y(th[(j + 1) % NDATA].t, minT, maxT, HEIGHT);
        M5.Lcd.drawLine(i * 2 + X0, t0, (i + 1) * 2 + X0, t1, GREEN);
        int h0 = th2y(th[j % NDATA].h, minH, maxH, HEIGHT);
        int h1 = th2y(th[(j + 1) % NDATA].h, minH, maxH, HEIGHT);
        M5.Lcd.drawLine(i * 2 + X0, h0, (i + 1) * 2 + X0, h1, RED);
    }
}

void setup(){
    M5.begin();

    Wire.begin(SDA, SCL, 400000);
    pinMode(SDA, INPUT_PULLUP); // SDAピンのプルアップの指定
    pinMode(SCL, INPUT_PULLUP); // SCLピンのプルアップの指定
    Serial.begin(115200);
    M5.Lcd.setTextSize(2);

    bme280.begin(); // BME280の初期化

    for (int i; i < NDATA; i++) {
        th[i].h = -1.0;
    }

    WiFi.begin(ssid, password);  //  Wi-Fi APに接続
    while (WiFi.status() != WL_CONNECTED) {  //  Wi-Fi AP接続待ち
        Serial.print(".");
        delay(100);
    }
    Serial.print("WiFi connected\r\nIP address: ");
    Serial.println(WiFi.localIP());

    ambient.begin(channelId, writeKey, &client); // チャネルIDとライトキーを指定してAmbientの初期化
}

void loop() {
    struct bme280_data data;

    bme280.get_sensor_data(&data);
    Serial.printf("%0.2f, %0.2f, %0.2f\r\n",
        data.temperature, data.humidity, data.pressure / 100);
    putData(data.temperature, data.humidity);
    M5.Lcd.fillScreen(BLACK);
    drawChart();
    M5.Lcd.setCursor(15, 0);
    M5.Lcd.printf("%4.1f'C, %4.1f%%, %4.1fhPa", data.temperature, data.humidity, data.pressure / 100);

    ambient.set(1, data.temperature);
    ambient.set(2, data.humidity);
    ambient.set(3, data.pressure / 100);
    ambient.send();

    delay(600 * 1000);
}

