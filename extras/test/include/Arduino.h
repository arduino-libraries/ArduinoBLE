/*
  This file is part of the ArduinoBLE library.
  Copyright (c) 2018 Arduino SA. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef TEST_ARDUINO_H_
#define TEST_ARDUINO_H_

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "String.h"
#include "Stream.h"
#include "itoa.h"
#include "Common.h"

/******************************************************************************
   TYPEDEF
 ******************************************************************************/

typedef arduino::String String;
typedef bool boolean;

#if defined(__cplusplus)

#undef F
// C++11 F replacement declaration
template <typename T1>
auto F(T1&& A)
  -> const arduino::__FlashStringHelper*
{
  return (const arduino::__FlashStringHelper*)A;
}

#endif

/******************************************************************************
   FUNCTION PROTOTYPES
 ******************************************************************************/

#endif /* TEST_ARDUINO_H_ */
