from machine import UART
import micropyGPS
import utime, gc, _thread
from m5stack import lcd
from math import radians, sin, cos

gps_s = UART(2, tx=17, rx=16, baudrate=9600, timeout=200, buffer_size=256, lineend='\r\n')
gps = micropyGPS.MicropyGPS(9, 'dd')

def GPSwatch():
    n = 0
    tm_last = 0
    satellites = dict()
    while True:
        utime.sleep_ms(100)
        len = gps_s.any()
        if len>0:
            b = gps_s.read(len)
            for x in b:
                if 10 <= x <= 126:
                    stat = gps.update(chr(x))
                    if stat:
                        tm = gps.timestamp
                        tm_now = (tm[0] * 3600) + (tm[1] * 60) + int(tm[2])
                        if (tm_now - tm_last) >= 10:
                            n += 1
                            tm_last = tm_now
                            print("{} {}:{}:{}".format(gps.date_string(), tm[0], tm[1], int(tm[2])))
                            str = '%.10f %c, %.10f %c' % (gps.latitude[0], gps.latitude[1], gps.longitude[0], gps.longitude[1])
                            print(str)
                            lcd.clear()
                            lcd.print(str, 10, 0)
                            if gps.satellite_data_updated():
                                putSatellites(satellites, gps.satellite_data, tm_now)
                            drawGrid()
                            drawSatellites(satellites)
                            if (n % 10) == 0:
                                print("Mem free:", gc.mem_free())
                                gc.collect()

def putSatellites(sats, new_sats, tm):
    for k, v in new_sats.items():  # 衛星の辞書に新しい衛星データーと現在時刻を追加する
        sats.update({k: (v, tm)})
    for k, v in sats.items():  # 衛星の辞書中で300秒以上古いものを削除する
        if tm - v[1] > 300:
            print('pop(%s)' % str(k))
            sats.pop(k)

def drawGrid():
    for x in range(40, 121, 40):
        lcd.circle(160, 120, x, lcd.DARKGREY)
    for x in range(0, 360, 45):
        lcd.lineByAngle(160, 120, 0, 120, x, lcd.DARKGREY)
    for x in (('N', 165, 10), ('E', 295, 115), ('S', 165, 220), ('W', 15, 115)):
        lcd.print(x[0], x[1], x[2])
    for x in (('90', 155, 108), ('60', 195, 108), ('30', 235, 108), ('0', 275, 108)):
        lcd.print(x[0], x[1], x[2])

def drawSatellites(sats):
    for k, v in sats.items():
        print(k, v[0])
        if v[0][0] != None and v[0][1] != None:
            l = int((90 - v[0][0]) / 90.0 * 120.0)
            lcd.lineByAngle(160, 120, 0, l, v[0][1])
            x = 160 + sin(radians(v[0][1])) * l
            y = 120 - cos(radians(v[0][1])) * l
            lcd.circle(int(x), int(y), 4, lcd.GREEN, lcd.GREEN)
            lcd.print(str(k), int(x) + 9, int(y) - 7)

testth=_thread.start_new_thread("GPS", GPSwatch, ())
