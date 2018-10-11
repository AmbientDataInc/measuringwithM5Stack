# 温度、湿度、気圧センサBME280からI2Cでデータを読み、
# M5StackのLCDにグラフ表示する
#
from machine import I2C, Pin
import bme280
import time
from m5stack import lcd
import gc

i2c = I2C(scl=Pin(22), sda=Pin(21))
bme = bme280.BME280(i2c=i2c)

X0 = 15
Y0 = 0
font = lcd.FONT_DejaVu18

HEIGHT = lcd.screensize()[1]

NDATA = 150

def putData(data, buff):
    buff.append(data)  # 最新データをリストの末尾に追加
    if len(buff) > NDATA:
        buff.pop(0)  # 先頭のデータを捨てる

def th2y(th, minY, maxY, HEIGHT):
    return int(HEIGHT - (th - minY) / (maxY - minY) * HEIGHT + 1)

def drawChart(buff):
    tmp = tuple(map(sorted, zip(*buff)))
    minT = int(tmp[0][0]) - 1
    maxT = int(tmp[0][-1]) + 1
    minH = int(tmp[1][0]) - 10
    maxH = int(tmp[1][-1]) + 10
    for i in range(len(buff) - 1):
        lcd.line(i * 2 + X0, th2y(buff[i][0], minT, maxT, HEIGHT),
                 (i + 1) * 2 + X0, th2y(buff[i + 1][0], minT, maxT, HEIGHT), lcd.GREEN)
        lcd.line(i * 2 + X0, th2y(buff[i][1], minH, maxH, HEIGHT),
                 (i + 1) * 2 + X0, th2y(buff[i + 1][1], minH, maxH, HEIGHT), lcd.RED)

def temp():
    lcd.font(font)  # フォントを指定
    lcd.clear()  # 画面をクリア
    buff = []

    while True:
        data = bme.read_compensated_data()
        putData((data[0] / 100, data[2] / 1024), buff)
        str = "%.2f'C, %.2f%%, %.1fhPa" % (data[0] / 100, data[2] / 1024, data[1] / 25600)
        lcd.clear()
        drawChart(buff)
        lcd.print(str, X0, Y0)  # 温度を表示]
        print(str)
        gc.collect()
        time.sleep(1)
