/*
 * Copyright (c) 2022 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1
 * 
 */

#ifndef _WIFI_SOCKET_BUFFER_H_
#define _WIFI_SOCKET_BUFFER_H_

#include <Arduino.h>
#include <IPAddress.h>

#define WIFI_SOCKET_TCP_BUFFER_SIZE 4096
#define WIFI_SOCKET_UDP_BUFFER_SIZE 1500

class WiFiSocketBuffer {
  public:
    WiFiSocketBuffer();
    virtual ~WiFiSocketBuffer();

    void begin(int cid);

    int available(int cid);
    int read(int cid, uint8_t* buf, size_t size);
    int peek(int cid);

    void close(int cid);

    void clear(int cid);

    IPAddress remoteIP(int cid);
    uint16_t remotePort(int cid);
    bool connected(int cid);

    void clear();

    void connect(int cid);
    void receive(int cid, IPAddress ip, uint16_t port, Stream& s, int length);
    void disconnect(int cid);

private:
    struct {
      union WiFiSocketBuffer {
        RingBufferN<WIFI_SOCKET_TCP_BUFFER_SIZE>* tcp;
        RingBufferN<WIFI_SOCKET_UDP_BUFFER_SIZE>* udp;
      } rxBuffer;
      IPAddress remoteIp;
      uint16_t remotePort;
      bool connected;
    } _sockets[3];
};

#endif
