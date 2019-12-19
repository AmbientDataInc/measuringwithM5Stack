#include "HCSR04.h"

void HCSR04::begin(int Trig, int Echo) {
    _trig = Trig;
    _echo = Echo;
    pinMode(_trig, OUTPUT);
    pinMode(_echo, INPUT);
}

float HCSR04::distance(void) {
    int Duration;
    float Distance;

    digitalWrite(_trig, LOW);
    delayMicroseconds(1);
    digitalWrite(_trig,HIGH);
    delayMicroseconds(11);
    digitalWrite(_trig,LOW);
    Duration = pulseIn(_echo,HIGH);
    if (Duration <= 0) {
        return -1;
    }
    Distance = Duration/2;
    Distance = Distance*340*100/1000000; // ultrasonic speed is 340m/s = 34000cm/s = 0.034cm/us 
    return Distance;
}
