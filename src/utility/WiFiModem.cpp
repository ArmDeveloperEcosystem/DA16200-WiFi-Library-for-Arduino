/*
 * Copyright (c) 2022 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1
 * 
 */

#include "WiFiModem.h"

#if __has_include(<ArduinoLowPower.h>)
#include <ArduinoLowPower.h>)
#endif

WiFiModem::WiFiModem(HardwareSerial& serial, int rtcWakePin, int wakeUpPin) :
  _serial(&serial),
  _rtcWakePin(rtcWakePin),
  _wakeUpPin(wakeUpPin),
  _debug(NULL)
{
}

WiFiModem::~WiFiModem()
{
}

void WiFiModem::begin(unsigned long baudrate)
{
  pinMode(_rtcWakePin, OUTPUT);
  digitalWrite(_rtcWakePin, LOW);

  _serial->begin(baudrate);

  memset(&_extendedResponse, 0x00, sizeof(_extendedResponse));
}

void WiFiModem::end()
{
  _serial->end();

  pinMode(_rtcWakePin, INPUT);
  detachInterrupt(_wakeUpPin);
}

int WiFiModem::AT(const char* command, const char* args, unsigned long timeout)
{
  poll(0);

  this->print("AT");
  this->print(command);
  if (args != NULL) {
    this->print(args);
  }
  this->println();
  this->flush();

  return waitForResponse(timeout);
}

int WiFiModem::ESC(const char* sequence, const char* args, const uint8_t* buffer, int length, unsigned long timeout)
{
  this->print("\e");
  this->print(sequence);
  if (args != NULL) {
    this->print(args);
  }
  if (length > 0) {
    this->write(buffer, length);
  }
  this->flush();

  return waitForResponse(timeout);
}

void WiFiModem::poll(unsigned long timeout) {
  int bufferIndex = 0;
  char buffer[32 + 1];

  if (!this->available()) {
    for (unsigned long start = millis(); (millis() - start) < timeout;) {
      if (this->available()) {
        break;
      }
    }

    if (!this->available()) {
      return;
    }
  }

  timeout = 10;

  for (unsigned long start = millis(); (millis() - start) < timeout;) {
    if (this->available()) {
      char c = this->read();

      buffer[bufferIndex++] = c;

      if (c == '\n') {
        bufferIndex = 0;
      } else if (c == '+') {
        start = millis();
      } else if (c == ':' && buffer[0] == '+') {
        buffer[bufferIndex] = '\0';

        _extendedResponse.handler(_extendedResponse.context, buffer, *this);

        break;
      }

      start = millis();
    }
  }
}
void WiFiModem::wakeup()
{
  digitalWrite(_rtcWakePin, HIGH);
  delayMicroseconds(250);
  digitalWrite(_rtcWakePin, LOW);
}

void WiFiModem::onExtendedResponse(void(*handler)(void*, const char*, Stream&), void* context)
{
  _extendedResponse.handler = handler;
  _extendedResponse.context = context;
}

void WiFiModem::onIrq(void (*handler)(void))
{
  pinMode(_wakeUpPin, INPUT_PULLUP);
  attachInterrupt(_wakeUpPin, handler, RISING);
#if __has_include(<ArduinoLowPower.h>)
  LowPower.attachInterruptWakeup(_wakeUpPin, handler, RISING);
#endif
}

int WiFiModem::available()
{
  return _serial->available();
}

int WiFiModem::read()
{
  int b = _serial->read();

  if (_debug != NULL)  {
    if (b != -1) {
      _debug->write((uint8_t)b);
    }
  }

  return b;
}

int WiFiModem::peek()
{
  return _serial->peek();
}

size_t WiFiModem::write(uint8_t b)
{
  if (_debug != NULL)  {
    _debug->write(b);
  }

  return _serial->write(b);
}

size_t WiFiModem::write(const uint8_t* buffer, size_t size)
{
  if (_debug != NULL)  {
    _debug->write(buffer, size);
  }

  return _serial->write(buffer, size);
}

int WiFiModem::availableForWrite()
{
  return _serial->availableForWrite();
}

void WiFiModem::flush()
{
  _serial->flush();
}

void WiFiModem::debug(Print& p)
{
  _debug = &p;
}

void WiFiModem::noDebug()
{
  _debug = NULL;
}

int WiFiModem::waitForResponse(unsigned long timeout)
{
  int responseCode = -100;

  int bufferIndex = 0;
  char buffer[32 + 1];

  for (unsigned long start = millis(); (millis() - start) < timeout;) {
    if (this->available()) {
      char c = this->read();

      buffer[bufferIndex++] = c;

      if (c == '\n') {
        buffer[bufferIndex] = '\0';

        if (strcmp("OK\r\n", buffer) == 0) {
          responseCode = 0;
          break;
        } else if (strncmp("ERROR:", buffer, 6) == 0) {
          sscanf(buffer, "ERROR:%d\r\n", &responseCode);
          break;
        }

        bufferIndex = 0;
      } else if (c == ':' && buffer[0] == '+') {
        buffer[bufferIndex] = '\0';

        _extendedResponse.handler(_extendedResponse.context, buffer, *this);

        bufferIndex = 0;
      }
    }
  }

  return responseCode;
}
