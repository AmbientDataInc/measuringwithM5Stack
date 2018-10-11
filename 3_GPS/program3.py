from machine import UART
import micropyGPS
import utime, gc, _thread
from m5stack import lcd

gps_s = UART(2, tx=17, rx=16, baudrate=9600, timeout=200, buffer_size=256, lineend='\r\n')
gps = micropyGPS.MicropyGPS(9, 'dd')

def GPSparse():
    n = 0
    tm_last = 0
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
                            lcd.clear()
                            lcd.print("Lat: %.8f %s" % (gps.latitude[0], gps.latitude[1]), 10, 40)
                            lcd.print("Lng: %.8f %s" % (gps.longitude[0], gps.longitude[1]), 10, 80)
                            if (n % 10) == 0:
                                print("Mem free:", gc.mem_free())
                                gc.collect()

def GPStest():
    lcd.font(lcd.FONT_DejaVu18)
    testth=_thread.start_new_thread("GPS", GPSparse, ())
