#ifndef HCSR04_H
#define HCSR04_H

#include <Arduino.h>

class HCSR04 {
public:
    HCSR04(void) {};
    virtual ~HCSR04() {};

    void begin(int, int);
    float distance(void);
private:
    uint8_t _trig;
    uint8_t _echo;
};

#endif // HCSR04_H
