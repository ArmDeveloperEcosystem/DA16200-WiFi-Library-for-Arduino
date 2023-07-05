/*
 * Copyright (c) 2022 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1
 * 
 */

#ifndef _WIFI_SERVER_H_
#define _WIFI_SERVER_H_

#include <Arduino.h>
#include <IPAddress.h>
#include <Stream.h>

#include <Server.h>

class WiFiClient;

#define WIFI_SERVER_MAX_CLIENTS 8

class WiFiServer : public Server {
  public:
    WiFiServer(uint16_t);
    virtual ~WiFiServer();

    WiFiClient available(uint8_t* status = NULL);
    void begin();
    virtual size_t write(uint8_t);
    virtual size_t write(const uint8_t *buf, size_t size);
    uint8_t status();

    using Print::write;

  protected:
    friend class WiFiClass;
    friend class WiFiClient;

    static WiFiServer* _inst;

    void connect(int cid, IPAddress ip, uint16_t port);
    bool connected(int cid, IPAddress ip, uint16_t port);
    void disconnect(int cid, IPAddress ip, uint16_t port);

  private:
    uint16_t _port;
    int _cid;
    struct {
      IPAddress remoteIp;
      uint16_t remotePort;
    } _clients[WIFI_SERVER_MAX_CLIENTS];
};

#endif
