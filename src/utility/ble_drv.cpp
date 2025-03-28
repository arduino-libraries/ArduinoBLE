
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "Arduino.h"
#include <spi_drv.h>
#include "utility/ble_drv.h"


extern "C" {
#include "utility/ble_spi.h"
}


int BleDrv::bleBegin() {
    WAIT_FOR_SLAVE_SELECT();

    SpiDrv::sendCmd(BLE_BEGIN, PARAM_NUMS_0);

    SpiDrv::spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv::waitForSlaveReady();
    SpiDrv::spiSlaveSelect();

    uint8_t len = 1;
    uint8_t result = 0;
    SpiDrv::waitResponseCmd(BLE_BEGIN, PARAM_NUMS_1, (uint8_t*)&result, &len);
    SpiDrv::spiSlaveDeselect();

    return result == 0;
}

void BleDrv::bleEnd() {
    WAIT_FOR_SLAVE_SELECT();

    SpiDrv::sendCmd(BLE_END, PARAM_NUMS_0);

    SpiDrv::spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv::waitForSlaveReady();
    SpiDrv::spiSlaveSelect();

    uint8_t len = 1;
    uint8_t result = 0;
    SpiDrv::waitResponseCmd(BLE_END, PARAM_NUMS_1, (uint8_t*)&result, &len);
    SpiDrv::spiSlaveDeselect();
}

int BleDrv::bleAvailable() {
    WAIT_FOR_SLAVE_SELECT();
    uint16_t result = 0;

    SpiDrv::sendCmd(BLE_AVAILABLE, PARAM_NUMS_0);

    SpiDrv::spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv::waitForSlaveReady();
    SpiDrv::spiSlaveSelect();

    uint8_t len = 2;
    SpiDrv::waitResponseCmd(BLE_AVAILABLE, PARAM_NUMS_1, (uint8_t*)&result, &len);
    SpiDrv::spiSlaveDeselect();

    return result;
}

int BleDrv::bleRead(uint8_t data[], size_t length) {
    WAIT_FOR_SLAVE_SELECT();

    SpiDrv::sendCmd(BLE_READ, PARAM_NUMS_1);

    int commandSize = 7; // 4 for the normal command length + 3 for the parameter
    uint16_t param = length; // TODO check length doesn't exceed 2^16
    SpiDrv::sendParam((uint8_t*)&param, sizeof(param), LAST_PARAM);

    // pad to multiple of 4
    while (commandSize % 4 != 0) {
        SpiDrv::readChar();
        commandSize++;
    }

    SpiDrv::spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv::waitForSlaveReady();
    SpiDrv::spiSlaveSelect();

    uint16_t res_len = 0;
    SpiDrv::waitResponseData16(BLE_READ, data, (uint16_t*)&res_len);

    SpiDrv::spiSlaveDeselect();

    return res_len;
}

int BleDrv::blePeek(uint8_t data[], size_t length) {
    WAIT_FOR_SLAVE_SELECT();

    SpiDrv::sendCmd(BLE_PEEK, PARAM_NUMS_1);

    int commandSize = 7; // 4 for the normal command length + 3 for the parameter
    uint16_t param = length; // TODO check length doesn't exceed 2^16
    SpiDrv::sendParam((uint8_t*)&param, sizeof(param), LAST_PARAM);

    // pad to multiple of 4
    while (commandSize % 4 != 0) {
        SpiDrv::readChar();
        commandSize++;
    }

    SpiDrv::spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv::waitForSlaveReady();
    SpiDrv::spiSlaveSelect();

    uint16_t res_len = 0;
    SpiDrv::waitResponseData16(BLE_READ, data, (uint16_t*)&res_len);

    SpiDrv::spiSlaveDeselect();

    return res_len;
}

size_t BleDrv::bleWrite(const uint8_t* data, size_t len) {
    WAIT_FOR_SLAVE_SELECT();

    int commandSize = 4;
    SpiDrv::sendCmd(BLE_WRITE, PARAM_NUMS_1);

    SpiDrv::sendBuffer((uint8_t*)data, len, LAST_PARAM);
    commandSize += len+2;

    // pad to multiple of 4
    while (commandSize % 4 != 0) {
        SpiDrv::readChar();
        commandSize++;
    }

    SpiDrv::spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv::waitForSlaveReady();
    SpiDrv::spiSlaveSelect();

    uint8_t res_len = 1;
    uint16_t res = 0;
    SpiDrv::waitResponseCmd(BLE_WRITE, PARAM_NUMS_1, (uint8_t*)&res, &res_len);
    SpiDrv::spiSlaveDeselect();

    return res;
}


BleDrv bleDrv;