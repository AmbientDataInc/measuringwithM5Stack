/*
 * M5Stackでアナログ温度センサLM35DZを読み、シリアルとLCDに表示する
 */
#include <M5Stack.h>
#include "Ambient.h"

#define PIN 36
#define MULTISAMPLES 10

WiFiClient client;
const char* ssid = "ssid";
const char* password = "password";

Ambient ambient;
unsigned int channelId = 100; // AmbientのチャネルID
const char* writeKey = "writeKey"; // ライトキー

void setup() {
    M5.begin();
    M5.Speaker.write(0); // スピーカーをオフする
    Serial.begin(115200);

    pinMode(PIN, INPUT);

    M5.Lcd.setTextSize(3);

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
    float vout = 0.0;
    // 複数回測り、平均することでノイズの影響を小さくする
    for (int i = 0; i < MULTISAMPLES; i++) {
        // ADコンバータで値を読み、簡易補正関数を使って補正する
        vout += (float)analogRead(PIN) / 4095.0 * 3.6 + 0.1132;
    }
    vout /= MULTISAMPLES;

    // 温度 = Vout / 0.01v
    Serial.printf("%4.1f\r\n", vout / 0.01);

    M5.Lcd.setCursor(40, 100);
    M5.Lcd.printf("temp: %4.1f'C", vout / 0.01);

    ambient.set(1, vout / 0.01);
    ambient.send();

    delay(60 * 1000);
}

