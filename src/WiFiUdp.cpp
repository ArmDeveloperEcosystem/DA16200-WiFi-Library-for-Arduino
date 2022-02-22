/*
 * Copyright (c) 2022 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1
 * 
 */

#include "WiFi.h"

#include "WiFiUdp.h"

WiFiUDP* WiFiUDP::_inst = NULL;

WiFiUDP::WiFiUDP() :
  _txBufferIndex(0)
{
}

WiFiUDP::~WiFiUDP()
{
  if (_inst == this) {
    _inst = NULL;
  }
}

uint8_t WiFiUDP::begin(uint16_t port)
{
  if (_inst != NULL) {
    return 0;
  }

  char args[1 + 5 + 1];

  sprintf(args, "=%d", port);

  if (WiFi.AT("+TRUSE", args) != 0) {
    return 0;
  }

  _inst = this;
  _txBufferIndex = 0;
  WiFi.socketBuffer().begin(2);
  WiFi.socketBuffer().clear(2);

  return 1;
}

void WiFiUDP::stop()
{
  if (_inst == this) {
    WiFi.AT("+TRTRM", "=2");

    _inst = NULL;
    _txBufferIndex = 0;
    WiFi.socketBuffer().clear(2);
  }
}

int WiFiUDP::beginPacket(IPAddress ip, uint16_t port)
{
  char args[1 + 15 + 1 + 5 + 1];

  sprintf(args, "=%d.%d.%d.%d,%d", ip[0], ip[1], ip[2], ip[3], port);

  if (WiFi.AT("+TRUR", args) != 0) {
    return 0;
  }

  _txBufferIndex = 0;

  return 1;
}

int WiFiUDP::beginPacket(const char* host, uint16_t port)
{
  IPAddress ip;

  if (!WiFi.hostByName(host, ip)) {
    return 0;
  }

  return beginPacket(ip, port);
}

int WiFiUDP::endPacket()
{
  char args[1 + 4 + 5 + 1];

  sprintf(args, "2%d,0,0,", _txBufferIndex);

  int result = WiFi.ESC("S", args, _txBuffer, _txBufferIndex);

  _txBufferIndex = 0;

  return result;
}

size_t WiFiUDP::write(uint8_t b)
{
  return write(&b, sizeof(b));
}

size_t WiFiUDP::write(const uint8_t* buffer, size_t size)
{
  if ((_txBufferIndex + size) > sizeof(_txBuffer)) {
    size = sizeof(_txBuffer) - _txBufferIndex;
  }

  memcpy(_txBuffer + _txBufferIndex, buffer, size);
  _txBufferIndex += size;

  return size;
}

int WiFiUDP::parsePacket()
{
  WiFi.socketBuffer().clear(2);

  WiFi.poll(0);

  return WiFi.socketBuffer().available(2);
}

int WiFiUDP::available()
{
  return WiFi.socketBuffer().available(2);
}

int WiFiUDP::read()
{
  if (!available()) {
    return -1;
  }

  uint8_t b;

  read(&b, sizeof(b));

  return b;
}

int WiFiUDP::read(unsigned char* buffer, size_t len)
{
  return WiFi.socketBuffer().read(2, buffer, len);
}

int WiFiUDP::read(char* buffer, size_t len)
{
  return read((unsigned char*)buffer, len);
}

int WiFiUDP::peek()
{
  if (available()) {
    return WiFi.socketBuffer().peek(2);
  }

  return -1;
}

void WiFiUDP::flush()
{
}

IPAddress WiFiUDP::remoteIP()
{
  return WiFi.socketBuffer().remoteIP(2);
}

uint16_t WiFiUDP::remotePort()
{
  return WiFi.socketBuffer().remotePort(2);
}
