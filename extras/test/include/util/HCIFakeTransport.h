//#include "Common.h"
#pragma once

#include "HCITransport.h"

class HCIFakeTransportClass : public HCITransportInterface
{
public:
    HCIFakeTransportClass() {};
    ~HCIFakeTransportClass() {};

    int begin() {return 0;}
    void end() {return;}
    void wait(unsigned long timeout) {return;}
    int available() {return 0;}
    int peek() {return 0;}
    int read() {return 0;}
    size_t write(const uint8_t* data, size_t length) {return 0;}
};