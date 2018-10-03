/*
 * M5Stackでアナログ温度センサLM35DZを読み、シリアルとLCDに表示する
 */
#include <M5Stack.h>

// #define FONT_7SEG

#define PIN 36
#define MULTISAMPLES 10

void setup() {
    M5.begin();
    M5.Speaker.write(0); // スピーカーをオフする
    Serial.begin(115200);

    pinMode(PIN, INPUT);

#ifdef FONT_7SEG
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(RED, BLACK);
#else
    M5.Lcd.setTextSize(3);
#endif // FONT_7SEG
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

#ifdef FONT_7SEG
    char str[16];
    sprintf(str, "%4.1f\r\n", vout / 0.01);
    M5.Lcd.drawString(str, 60, 60, 7);
#else
    M5.Lcd.setCursor(40, 100);
    M5.Lcd.printf("temp: %4.1f'C", vout / 0.01);
#endif // FONT_7SEG

    delay(1000);
}

