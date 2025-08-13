
#pragma once

#include <inttypes.h>



class BleDrv
{
public:
    static int bleBegin();
    static void bleEnd();
    static int bleAvailable();
    static int bleRead(uint8_t data[], size_t length);
    static int blePeek(uint8_t data[], size_t length);
    static size_t bleWrite(const uint8_t* data, size_t length);

};

extern BleDrv bleDrv;
