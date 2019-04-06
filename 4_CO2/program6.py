from m5stack import lcd, buttonA
from machine import I2C, Pin, UART
import bme280
import struct
import utime
import gc

u2 = UART(2, tx=17, rx=16, baudrate=9600, buffer_size=256)

i2c = I2C(scl=Pin(22), sda=Pin(21))
bme = bme280.BME280(i2c=i2c)

def readMHZ19():
    cmd = b'\xff\x01\x86\x00\x00\x00\x00\x00\x79'
    l = 0
    while l != 9:
        u2.write(cmd)
        utime.sleep_ms(100)
        raw = u2.read(9)
        l = len(raw)
    data = struct.unpack('BBBBBBBBB', raw)
    if (sum(data) + 1) % 256 == 0:
        return data[2] * 256 + data[3]
    else:
        return None

X0 = 10
Y0 = 0
HEIGHT = lcd.screensize()[1]
NDATA = 150

def putData(data, buff):
    buff.append(data)  # 最新データをリストの末尾に追加
    if len(buff) > NDATA:
        buff.pop(0)  # 先頭のデータを捨てる

def th2y(d, minY, maxY, HEIGHT):
    return int(HEIGHT - (d - minY) / (maxY - minY) * HEIGHT + 1)

def drawChart(buff):
    tmp = tuple(map(sorted, zip(*buff)))
    minT = int(tmp[0][0]) - 1
    maxT = int(tmp[0][-1]) + 1
    minC = int(tmp[1][0]) - 10
    maxC = int(tmp[1][-1]) + 10
    for i in range(len(buff) - 1):
        lcd.line(i * 2 + X0, th2y(buff[i][0], minT, maxT, HEIGHT),
                 (i + 1) * 2 + X0, th2y(buff[i + 1][0], minT, maxT, HEIGHT), lcd.GREEN)
        lcd.line(i * 2 + X0, th2y(buff[i][1], minC, maxC, HEIGHT),
                 (i + 1) * 2 + X0, th2y(buff[i + 1][1], minC, maxC, HEIGHT), lcd.RED)

dispNum = True

def dispData(buff):
    lcd.clear()
    str = "%.1f'C, %dppm" % (buff[-1][0], buff[-1][1])
    if dispNum:
        lcd.font(lcd.FONT_DejaVu24)  # フォントを指定
        lcd.print("%4.1f'C" % buff[-1][0], 120, 80)
        lcd.print("%dppm" % buff[-1][1], 120, 120)
    else:
        lcd.font(lcd.FONT_DejaVu18)  # フォントを指定
        lcd.print(str, X0, Y0)  # 温度、CO2を表示
        drawChart(buff)
    print(str)

period = 5

def temp_co2():
    lcd.clear()  # 画面をクリア
    buff = []

    while True:
        data = bme.read_compensated_data()
        co2 = readMHZ19()
        if co2 != None:
            putData((data[0] / 100, co2), buff)
        dispData(buff)
        t = utime.ticks_ms()
        while (utime.ticks_ms() - t) < period * 1000:
            if buttonA.wasPressed():
                global dispNum
                dispNum = not dispNum
                dispData(buff)
            utime.sleep_ms(10)
        gc.collect()

if __name__ == '__main__':
    temp_co2()
