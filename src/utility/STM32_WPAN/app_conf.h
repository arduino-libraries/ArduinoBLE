//-----------------------------
// @file app_conf.h
// @author Kasper Meldgaard
// @brief Wrapper for BLE app configuration based on comment by fpistm
// (https://github.com/stm32duino/STM32duinoBLE/issues/34).
// @date 15-11-2021
// @copyright Copyright (c) 2021

#ifndef APP_CONF_H
#define APP_CONF_H

#include "hw.h"
#include "ble_bufsize.h"

#if __has_include("app_conf_custom.h")
  #include "app_conf_custom.h"
#endif
#include "app_conf_default.h"

#endif /* APP_CONF_H */
