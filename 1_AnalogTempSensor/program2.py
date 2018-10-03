import machine
import time
from m5stack import lcd

PIN = 36
MULTISAMPLES = 10
X0 = 120
Y0 = 110
font = lcd.FONT_Dejavu24
# font = lcd.FONT_7seg

def temp():
    adc = machine.ADC(PIN)  # ADコンバータのインスタンスを生成
    lcd.font(font)  # フォントを指定
    lcd.clear()  # 画面をクリア

    while True:
        t = 0
        for i in range(MULTISAMPLES):
            t = t + adc.read() / 10  # ADコンバータを読み、温度に変換
        t = t / MULTISAMPLES
        lcd.print('\r', X0, Y0)  # 表示位置以降をクリア
        lcd.print("%4.1f" % t, X0, Y0)  # 温度を表示
        time.sleep(5)
