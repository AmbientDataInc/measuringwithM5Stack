/*
 * M5StackでシリアルでMH-Z19Bを、I2CでBME280読み、LCDに表示する
 * Aボタンで数字表示とグラフ表示を切り替える
 * Ambientにデーターを送信する
 */
#include <M5Stack.h>
#include <Wire.h>
#include "bme280_i2c.h"
#include <math.h>
#include "Ambient.h"

HardwareSerial u2(2);

#define PERIOD 10

#define SDA 21
#define SCL 22

WiFiClient client;
const char* ssid = "ssid";
const char* password = "password";

Ambient ambient;
unsigned int channelId = 100; // AmbientのチャネルID
const char* writeKey = "writeKey"; // ライトキー

BME280 bme280(BME280_I2C_ADDR_PRIM);

uint16_t heat(double);

// 温度、CO2を入れるリングバッファ
#define NDATA 150
struct d {
    bool valid;
    float d1;
    float d2;
} data[NDATA];
int dataIndex = 0;

// リングバッファに温度、CO2データを挿入する
void putData(float d1, float d2) {
    if (++dataIndex >= NDATA) {
        dataIndex = 0;
    }
    data[dataIndex].valid = true;
    data[dataIndex].d1 = d1;
    data[dataIndex].d2 = d2;
}

#define X0 10

// 温度、CO2の値からy軸の値を計算する
int data2y(float d, float minY, float maxY, int HEIGHT) {
    return HEIGHT - ((int)((d - minY) / (maxY - minY) * (float)HEIGHT) + 1);
}

// リングバッファから温度、CO2を読み、グラフ表示する
void drawChart() {
    int HEIGHT = M5.Lcd.height();
    float mint = 100.0, maxt = -100.0, minc = 2500.0, maxc = 0.0;
    for (int i = 0; i < NDATA; i++) {
        if (data[i].valid == false) continue;
        if (data[i].d1 < mint) mint = data[i].d1;
        if (data[i].d1 > maxt) maxt = data[i].d1;
    }
    int minT = (int)mint - 1;
    int maxT = (int)maxt + 1;
    int minC = 200;
    int maxC = 1800;
    for (int i = 0, j = dataIndex + 1; i < (NDATA - 1); i++, j++) {
        if (data[j % NDATA].valid == false) continue;
        int t0 = data2y(data[j % NDATA].d1, minT, maxT, HEIGHT);
        int t1 = data2y(data[(j + 1) % NDATA].d1, minT, maxT, HEIGHT);
        M5.Lcd.drawLine(i * 2 + X0, t0, (i + 1) * 2 + X0, t1, WHITE);
        int c0 = data2y(data[j % NDATA].d2, minC, maxC, HEIGHT);
        int c1 = data2y(data[(j + 1) % NDATA].d2, minC, maxC, HEIGHT);
        int low = 400, high = 1200;
        uint16_t color = heat((double)map(constrain(data[(j + 1) % NDATA].d2, low, high), low, high, 50, 100) / 100.0);
        M5.Lcd.drawLine(i * 2 + X0, c0, (i + 1) * 2 + X0, c1, color);
    }
}

#define N_MENU 12
#define H_MENU 20

class Menu {
public:
    Menu(void) {};
    void setMenu(char *a = NULL, char* b = NULL, char *c = NULL, uint16_t textcolor = WHITE, uint16_t backcolor = BLACK);
    void putMenu(char *s, int x);
    int height() { return H_MENU; }
};

void Menu::putMenu(char *s, int x) {
    char buf[N_MENU];
    int len;

    if (s) {
        M5.Lcd.setCursor(x, M5.Lcd.height() - H_MENU + 4);
        len = min(strlen(s), N_MENU);
        strncpy(buf, s, len);
        buf[len] = '\0';
        M5.Lcd.print(buf);
    }
}

void Menu::setMenu(char *a, char* b, char *c, uint16_t textcolor, uint16_t backcolor) {
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(textcolor, backcolor);
    putMenu(a, 38);
    putMenu(b, 130);
    putMenu(c, 225);
}

Menu menu;

bool dispNum = true;

void dispData(struct bme280_data *data, short co2) {
    M5.Lcd.fillScreen(BLACK);
    int low = 400, high = 1200;
    int c = map(constrain(co2, low, high), low, high, 50, 100);
    uint16_t backcolor = heat((double)c / 100.0);
    uint16_t textcolor = heat((double)((c + 25) % 100) / 100.0);
    if (dispNum) {
        M5.Lcd.setTextSize(3);
        M5.Lcd.fillScreen(backcolor);
        M5.Lcd.setTextColor(textcolor, backcolor);
        M5.Lcd.setCursor(20, 30);
        M5.Lcd.printf("temp: %4.1f'C", data->temperature);
        M5.Lcd.setCursor(20, 80);
        M5.Lcd.printf("humid: %4.1f%%", data->humidity);
        M5.Lcd.setCursor(20, 130);
        M5.Lcd.printf("press: %4.1fhPa", data->pressure / 100);
        M5.Lcd.setCursor(20, 180);
        M5.Lcd.printf("co2: %dppm", co2);
        menu.setMenu("num/graph", "", "", textcolor, backcolor);
    } else {
        drawChart();
        M5.Lcd.setTextSize(2);
        M5.Lcd.setTextColor(WHITE, BLACK);
        M5.Lcd.setCursor(15, 0);
        M5.Lcd.printf("%4.1f'C, %4.1f%%, %dppm", data->temperature, data->humidity, co2);
        menu.setMenu("num/graph", "", "", WHITE, BLACK);
    }
}

void setup() {
    M5.begin();
    M5.Speaker.write(0); // スピーカーをオフする
    Serial.begin(115200);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.print("\r\nWiFi connected: ");
    Serial.println(WiFi.localIP());

    u2.begin(9600);

    Wire.begin(SDA, SCL, 400000);

    bme280.begin(); // BME280の初期化

    ambient.begin(channelId, writeKey, &client); // チャネルIDとライトキーを指定してAmbientの初期化

    for (int i; i < NDATA; i++) {
        data[i].valid = false;
    }
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

    unsigned long startt = millis(); // loopの開始時刻を記録

    bme280.get_sensor_data(&data);
    short co2 = readMHZ19();
    if (co2 < 0) {
        Serial.println("readMHZ19 failed");
    } else {
        putData(data.temperature, (float)co2);
        Serial.printf("t: %0.1f, h: %0.1f, p: %0.1f, co2: %d\r\n",
            data.temperature, data.humidity, data.pressure / 100, co2);
        dispData(&data, co2);

        ambient.set(1, data.temperature);
        ambient.set(2, data.humidity);
        ambient.set(3, data.pressure / 100);
        ambient.set(4, co2);
        ambient.send();

        while ((millis() - startt) < PERIOD * 1000) {
            M5.update();
            if (M5.BtnA.wasPressed()) {
                dispNum = !dispNum;
                dispData(&data, co2);
            }
        }
    }
}
