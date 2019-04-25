/* 
 * MPU9250を読んでシリアルに値を表示する
 */
#include <M5Stack.h>
#include "utility/MPU9250.h"

MPU9250 IMU;

void setup()
{
    M5.begin();
    Serial.begin(115200);
    while (!Serial) ;
    Wire.begin();

    byte c = IMU.readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250);
    if (c != 0x71) {
        Serial.println("Can't connect to MPU9250");
        while (true) delay(0);
    }

    IMU.MPU9250SelfTest(IMU.SelfTest);
    IMU.calibrateMPU9250(IMU.gyroBias, IMU.accelBias);
    IMU.initMPU9250();
}

unsigned long t = 0;

void loop()
{
    if (IMU.readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01) {
        IMU.readAccelData(IMU.accelCount);  // Read the x/y/z adc values
        IMU.getAres();

        IMU.ax = (float)IMU.accelCount[0]*IMU.aRes - IMU.accelBias[0];
        IMU.ay = (float)IMU.accelCount[1]*IMU.aRes - IMU.accelBias[1];
        IMU.az = (float)IMU.accelCount[2]*IMU.aRes - IMU.accelBias[2];
    }

    if ((millis() - t) > 50) {
        Serial.printf("%.2f, %.2f, %.2f\r\n", 1000*IMU.ax, 1000*IMU.ay, 1000*IMU.az);
        t = millis();
    }
}
