//#include "Common.h"
#pragma once

#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2

class Stream 
{
public:
  Stream(const char *name = NULL);
  ~Stream();

  void flush() {}

  size_t print(const char[]) { return 0; }
  size_t print(char) { return 0; }
  size_t print(unsigned char, int) { return 0; }
  size_t print(int, int) { return 0; }
  size_t print(unsigned int, int) { return 0; }
  size_t print(long, int) { return 0; }
  size_t print(unsigned long, int) { return 0; }
  size_t print(long long, int) { return 0; }
  size_t print(unsigned long long, int) { return 0; }
  size_t print(double, int) { return 0; }
  size_t print(void) { return 0; }

  size_t println(const char[]) { return 0; }
  size_t println(char) { return 0; }
  size_t println(unsigned char, int) { return 0; }
  size_t println(int, int) { return 0; }
  size_t println(unsigned int, int) { return 0; }
  size_t println(long, int) { return 0; }
  size_t println(unsigned long, int) { return 0; }
  size_t println(long long, int) { return 0; }
  size_t println(unsigned long long, int) { return 0; }
  size_t println(double, int) { return 0; }
  size_t println(void) { return 0; }
};
