/*
 * Copyright (c) 2022 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1
 * 
 */

#ifndef _WIFI_MODEM_H_
#define _WIFI_MODEM_H_

#include <Arduino.h>

class WiFiModem : public Stream {
  public:
    WiFiModem(HardwareSerial& serial, int rtcWakePin, int wakeUpPin);
    virtual ~WiFiModem();

    void begin(unsigned long baudrate);
    void end();

    void onExtendedResponse(void (*handler)(void*, const char*, Stream&), void* context);
    void onIrq(void (*handler)(void));

    int AT(const char* command, const char* args, unsigned long timeout);
    int ESC(const char* sequence, const char* args, const uint8_t* buffer, int length, unsigned long timeout);

    void poll(unsigned long timeout);

    void wakeup();

    // from Stream
    virtual int available();
    virtual int read();
    virtual int peek();

    // from Print
    virtual size_t write(uint8_t);
    virtual size_t write(const uint8_t* buffer, size_t size);
    virtual int availableForWrite();
    virtual void flush();

    void debug(Print& p);
    void noDebug();

  private:
    int waitForResponse(unsigned long timeout);

  private:
    HardwareSerial* _serial;
    int _rtcWakePin;
    int _wakeUpPin;

    Print* _debug;

    struct {
      void(*handler)(void*, const char*, Stream&);
      void* context;
    } _extendedResponse;
};

#endif
