/*
 * M5StackでシリアルでMH-Z19Bを読む
 */
#include <M5Stack.h>

HardwareSerial u2(2);

#define PERIOD 5

void setup(){
    M5.begin();
    M5.Speaker.write(0); // スピーカーをオフする
    Serial.begin(115200);
    u2.begin(9600);

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

void loop() {
    short co2 = readMHZ19();
    if (co2 < 0) {
        Serial.println("readMHZ19 failed");
    } else {
        Serial.printf("co2: %d\r\n", co2);
    }
    delay(PERIOD * 1000);
}

