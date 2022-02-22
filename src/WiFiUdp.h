/*
 * Copyright (c) 2022 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1
 * 
 */

#ifndef _WIFI_UDP_H_
#define _WIFI_UDP_H_

#include <Udp.h>

class WiFiUDP : public UDP {
  public:
    WiFiUDP();
    virtual ~WiFiUDP();

    virtual uint8_t begin(uint16_t);
    virtual void stop();

    virtual int beginPacket(IPAddress ip, uint16_t port);
    virtual int beginPacket(const char* host, uint16_t port);
    virtual int endPacket();
    virtual size_t write(uint8_t);
    virtual size_t write(const uint8_t* buffer, size_t size);

    using Print::write;

    virtual int parsePacket();
    virtual int available();
    virtual int read();
    virtual int read(unsigned char* buffer, size_t len);
    virtual int read(char* buffer, size_t len);
    virtual int peek();
    virtual void flush();

    virtual IPAddress remoteIP();
    virtual uint16_t remotePort();

  private:
    static WiFiUDP* _inst;

    uint8_t _txBuffer[1500];
    int _txBufferIndex;
};

#endif
