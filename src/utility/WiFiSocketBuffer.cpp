/*
 * Copyright (c) 2022 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1
 * 
 */

#include "WiFiSocketBuffer.h"

WiFiSocketBuffer::WiFiSocketBuffer()
{
  memset(&_sockets, 0x00, sizeof(_sockets));
}

WiFiSocketBuffer::~WiFiSocketBuffer()
{
  for (int i = 0; i < 3; i++) {
    if (i < 2) {
      if (_sockets[i].rxBuffer.tcp != NULL) {
        delete _sockets[i].rxBuffer.tcp;
      }
    } else {
      if (_sockets[i].rxBuffer.udp != NULL) {
        delete _sockets[i].rxBuffer.udp;
      }
    }
  }
}

void WiFiSocketBuffer::begin(int cid)
{
  if (cid < 2) {
    if (_sockets[cid].rxBuffer.tcp == NULL) {
      _sockets[cid].rxBuffer.tcp = new RingBufferN<WIFI_SOCKET_TCP_BUFFER_SIZE>;
    }
  } else {
    if (_sockets[cid].rxBuffer.udp == NULL) {
      _sockets[cid].rxBuffer.udp = new RingBufferN<WIFI_SOCKET_UDP_BUFFER_SIZE>;
    }
  }
}

int WiFiSocketBuffer::available(int cid)
{
  if (cid < 2) {
    return _sockets[cid].rxBuffer.tcp->available();
  } else {
    return _sockets[cid].rxBuffer.udp->available();
  }
}

int WiFiSocketBuffer::read(int cid, uint8_t* buf, size_t size)
{
  int avail = available(cid);

  if (size > (size_t)avail) {
    size = avail;
  }

  if (cid < 2) {
    for (size_t i = 0; i < size; i++) {
      buf[i] = _sockets[cid].rxBuffer.tcp->read_char();
    }
  } else {
    for (size_t i = 0; i < size; i++) {
      buf[i] = _sockets[cid].rxBuffer.udp->read_char();
    }
  }

  return size;
}

int WiFiSocketBuffer::peek(int cid)
{
  if (cid < 2) {
    return _sockets[cid].rxBuffer.tcp->peek();
  } else {
    return _sockets[cid].rxBuffer.udp->peek();
  }
}

void WiFiSocketBuffer::clear(int cid)
{
  _sockets[cid].remoteIp = (uint32_t)0;
  _sockets[cid].remotePort = 0;
  _sockets[cid].connected = false;

  if (cid < 2) {
    _sockets[cid].rxBuffer.tcp->clear();
  } else {
    _sockets[cid].rxBuffer.udp->clear();
  }
}

IPAddress WiFiSocketBuffer::remoteIP(int cid)
{
  return _sockets[cid].remoteIp;
}

uint16_t WiFiSocketBuffer::remotePort(int cid)
{
  return _sockets[cid].remotePort;
}

bool WiFiSocketBuffer::connected(int cid)
{
  return _sockets[cid].connected;
}

void WiFiSocketBuffer::clear()
{
  for (int i = 0; i < 3; i++) {
    clear(i);
  }
}

void WiFiSocketBuffer::connect(int cid)
{
  _sockets[cid].connected = true;
}

void WiFiSocketBuffer::receive(int cid, IPAddress ip, uint16_t port, Stream& s, int length)
{
  int read = 0;

  if (cid < 2) {
    // TODO: timeout and handle overflows
    while (read < length) {
      if (s.available()) {
        _sockets[cid].rxBuffer.tcp->store_char(s.read());

        read++;
      }
    }

    _sockets[cid].remoteIp = ip;
    _sockets[cid].remotePort = port;
  } else {
    if (_sockets[cid].rxBuffer.udp->available() == 0) {
      // TODO: timeout and handle overflow
      while (read < length) {
        if (s.available()) {
          _sockets[cid].rxBuffer.udp->store_char(s.read());

          read++;
        }
      }

      _sockets[cid].remoteIp = ip;
      _sockets[cid].remotePort = port;
    } else {
      // drop packet ...
    }
  }

  length -= read;

  // TODO: timeout
  while (length) {
    if (s.available()) {
      s.read();

      length--;
    }
  }
}

void WiFiSocketBuffer::disconnect(int cid)
{
  _sockets[cid].connected = false;
}
