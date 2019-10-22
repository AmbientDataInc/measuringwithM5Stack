#include <M5Stack.h>
#include <Wire.h>

#define FPSC 0x02
#define INTC 0x03
#define AVE 0x07
#define T01L 0x80

#define AMG88_ADDR 0x68 // in 7bit

void write8(int id, int reg, int data) {  // idで示されるデバイスにregとdataを書く
    Wire.beginTransmission(id);  // 送信先のI2Cアドレスを指定して送信の準備をする
    Wire.write(reg);  // regをキューイングする
    Wire.write(data);  // dataをキューイングする
    uint8_t result = Wire.endTransmission();  // キューイングしたデータを送信する
}

void dataread(int id, int reg, int *data, int datasize) {
    Wire.beginTransmission(id);  // 送信先のI2Cアドレスを指定して送信の準備をする
    Wire.write(reg);  // regをキューイングする
    Wire.endTransmission();  // キューイングしたデータを送信する

    Wire.requestFrom(id, datasize);  // データを受信する先のI2Cアドレスとバイト数を指定する
    int i = 0;
    while (Wire.available() && i < datasize) {
        data[i++] = Wire.read();  // 指定したバイト数分、データを読む
    }
}

float gain = 10.0;
float offset_x = 0.2;
float offset_green = 0.6;

float sigmoid(float x, float g, float o) {
    return (tanh((x + o) * g / 2) + 1) / 2;
}

uint16_t heat(float x) {  // 0.0〜1.0の値を青から赤の色に変換する
    x = x * 2 - 1;  // -1 <= x < 1 に変換

    float r = sigmoid(x, gain, -1 * offset_x);
    float b = 1.0 - sigmoid(x, gain, offset_x);
    float g = sigmoid(x, gain, offset_green) + (1.0 - sigmoid(x, gain, -1 * offset_green)) - 1;

    return (((int)(r * 255)>>3)<<11) | (((int)(g * 255)>>2)<<5) | ((int)(b * 255)>>3);
}

void setup() {
    M5.begin();
    Serial.begin(115200);
    pinMode(21, INPUT_PULLUP);
    pinMode(22, INPUT_PULLUP);
    Wire.begin();
    
    write8(AMG88_ADDR, FPSC, 0x00);  // 10fps
    write8(AMG88_ADDR, INTC, 0x00);  // INT出力無効
    write8(AMG88_ADDR, 0x1F, 0x50);  // 移動平均出力モード有効(ここから)
    write8(AMG88_ADDR, 0x1F, 0x45);
    write8(AMG88_ADDR, 0x1F, 0x57);
    write8(AMG88_ADDR, AVE, 0x20);
    write8(AMG88_ADDR, 0x1F, 0x00);  // (ここまで)
}

#define WIDTH (320 / 8)
#define HEIGHT (240 / 8)

void loop() {
    float temp[64];

    int sensorData[128];
    dataread(AMG88_ADDR, T01L, sensorData, 128);
    for (int i = 0 ; i < 64 ; i++) {
        int16_t temporaryData = sensorData[i * 2 + 1] * 256 + sensorData[i * 2];
        if(temporaryData > 0x200) {
            temp[i] = (-temporaryData +  0xfff) * -0.25;
        } else {
            temp[i] = temporaryData * 0.25;
        }
    }

    M5.Lcd.fillScreen(BLACK);
    int x, y;
    for (y = 0; y < 8; y++) {
        for (x = 0; x < 8; x++) {
            float t = temp[(8 - y - 1) * 8 + 8 - x - 1];
            uint16_t color = heat(map(constrain((int)t, 0, 60), 0, 60, 0, 100) / 100.0);
            M5.Lcd.fillRect(x * WIDTH, y * HEIGHT, WIDTH, HEIGHT, color);
            M5.Lcd.setCursor(x * WIDTH + WIDTH / 2, y * HEIGHT + HEIGHT / 2);
            M5.Lcd.setTextColor(BLACK, color);
            M5.Lcd.printf("%d", (int)t);
        }
    }
    delay(500);
}
