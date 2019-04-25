from m5stack import lcd
from machine import I2C, Pin, Timer
from mpu9250 import MPU9250
import time
import gc

SAMPLE_PERIOD = 10 # ミリ秒
SAMPLE_SIZE = 300

tm = Timer(0)
i2c = I2C(sda = 21, scl = 22)
IMU = MPU9250(i2c)

minz, maxz, drange = (500, 1500, 80)
X0 = 10
_width, _height = lcd.screensize()

def data2y(d):
    return int(_height - (d - minz) / (maxz - minz) * _height)

def sample(buff):
    for i in range(SAMPLE_SIZE):
        t = tm.value()
        ax, ay, az = IMU.acceleration
        buff.append(az * 100)
        gc.collect()

        if (i != 0):
            if (data2y(buff[i - 1]) != data2y(buff[i])):
                lcd.line(i - 1 + X0, data2y(buff[i - 1]), i + X0, data2y(buff[i]), lcd.GREEN)
            else:
                lcd.pixel(i - 1 + X0, data2y(buff[i - 1]), lcd.GREEN)
                lcd.pixel(i + X0, data2y(buff[i]), lcd.GREEN)

        while (tm.value() - t) < SAMPLE_PERIOD:
            pass

def main():
    tm.init(mode=Timer.CHRONO)
    tm.start()

    while True:
        lcd.clear()
        buff = []
        sample(buff)
        buff.sort()
        global minz, maxz
        d = max(buff[-1] - buff[0], drange) / 2
        minz = buff[0]  - d
        maxz = buff[-1] + d
        print((buff[0], buff[-1], minz, maxz))

if __name__ == '__main__':
    main()
