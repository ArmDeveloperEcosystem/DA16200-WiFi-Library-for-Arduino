/*
 * Copyright (c) 2022 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1
 * 
 */

#ifndef _WIFI_CLIENT_H_
#define _WIFI_CLIENT_H_

#include <Client.h>
#if __has_include(<api/RingBuffer.h>)
#include <api/RingBuffer.h>
#else
#include <RingBuffer.h>
#endif

class WiFiClient : public Client {
  public:
    WiFiClient();
    virtual ~WiFiClient();

    virtual int connect(IPAddress ip, uint16_t port);
    virtual int connect(const char* host, uint16_t port);
    virtual size_t write(uint8_t);
    virtual size_t write(const uint8_t* buf, size_t size);
    virtual int available();
    virtual int read();
    virtual int read(uint8_t* buf, size_t size);
    virtual int peek();
    virtual void flush();
    virtual void stop();
    virtual uint8_t connected();
    virtual operator bool();

    using Print::write;

    virtual IPAddress remoteIP();
    virtual uint16_t remotePort();

  protected:
    friend class WiFiServer;

    WiFiClient(int cid, IPAddress remoteIp, uint16_t remotePort);

  private:
    static WiFiClient* _inst;
    
    int _cid;
    IPAddress _remoteIp;
    uint16_t _remotePort;
};

#endif
