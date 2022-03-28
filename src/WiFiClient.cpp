/*
 * Copyright (c) 2022 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1
 * 
 */

#include "WiFi.h"
#include "WiFiServer.h"

#include "WiFiClient.h"

WiFiClient* WiFiClient::_inst = NULL;

WiFiClient::WiFiClient() :
  WiFiClient(-1, (uint32_t)0, 0)
{
}

WiFiClient::WiFiClient(int cid, IPAddress remoteIp, uint16_t remotePort) :
  _cid(cid),
  _remoteIp(remoteIp),
  _remotePort(remotePort)
{
}

WiFiClient::~WiFiClient()
{
  if (_inst == this) {
    _inst = NULL;
  }
}

int WiFiClient::connect(IPAddress ip, uint16_t port)
{
  if (_inst != NULL && _inst != this) {
    return 0;
  }

  stop();

  char args[1 + 15 + 1 + 5 + 1 + 5 + 1];

  sprintf(args, "=%d.%d.%d.%d,%d,%d", ip[0], ip[1], ip[2], ip[3], port, 0);

  if (WiFi.AT("+TRTC", args, 10000) != 0) {
    return 0;
  }

  _inst = this;
  _cid = 1;
  _remoteIp = ip;
  _remotePort = port;
  WiFi.socketBuffer().begin(_cid);
  WiFi.socketBuffer().clear(_cid);
  WiFi.socketBuffer().connect(_cid);

  return 1;
}

int WiFiClient::connect(const char* host, uint16_t port)
{
  IPAddress ip;

  if (!WiFi.hostByName(host, ip)) {
    return 0;
  }

  return connect(ip, port);
}

size_t WiFiClient::write(uint8_t b)
{
  return write(&b, sizeof(b));
}

size_t WiFiClient::write(const uint8_t* buf, size_t size)
{
  if (_cid < 0) {
    return 0;
  }

  if (size > 2048) {
    size = 2048;
  }

  char args[1 + 4 + 1 + 15 + 1 + 5 + 1 + 1];

  sprintf(
    args, "%d%d,%d.%d.%d.%d,%d,",
    _cid, size,
    _remoteIp[0], _remoteIp[1], _remoteIp[2], _remoteIp[3],
    _remotePort
  );

  if (WiFi.ESC("S", args, buf, size) != 0) {
    setWriteError();
    return 0;
  }

  return size;
}

int WiFiClient::available()
{
  if (_cid < 0) {
    return 0;
  }


  if (WiFi.socketBuffer().available(_cid) == 0) {
    WiFi.poll(0);
  }

  if (WiFi.socketBuffer().remoteIP(_cid) != _remoteIp || WiFi.socketBuffer().remotePort(_cid) != _remotePort) {
    return 0;
  }

  return WiFi.socketBuffer().available(_cid);
}

int WiFiClient::read()
{
  if (!available()) {
    return -1;
  }

  uint8_t b;

  read(&b, sizeof(b));

  return b;
}

int WiFiClient::read(uint8_t* buf, size_t size)
{
  if (_cid < 0) {
    return 0;
  }

  int avail = available();
  if (size > (size_t)avail) {
    size = avail;
  }

  return WiFi.socketBuffer().read(_cid, buf, size);
}

int WiFiClient::peek()
{
  if (available()) {
    return WiFi.socketBuffer().peek(_cid);
  }

  return -1;
}

void WiFiClient::flush()
{
}

void WiFiClient::stop()
{
  if (_cid > -1) {
    if (_cid == 0) {
      if (WiFiServer::_inst != NULL) {
        WiFiServer::_inst->begin();
      }
    } else if (WiFi.socketBuffer().connected(_cid)) {
      WiFi.AT("+TRTRM", "=1", 5000);
    }

    WiFi.socketBuffer().clear(_cid);
    WiFi.socketBuffer().disconnect(_cid);

    _cid = -1;
    _remoteIp = (uint32_t)0;
    _remotePort = 0;

    if (_inst == this) {
      _inst = NULL;
    }
  }
}

uint8_t WiFiClient::connected()
{
  if (_cid < 0) {
    return 0;
  }

  WiFi._modem.poll(0);

  if (_cid == 0) {
    if (WiFiServer::_inst != NULL) {
      return WiFiServer::_inst->connected(_cid, _remoteIp, _remotePort);
    }

    return 0;
  }

  return WiFi.socketBuffer().available(_cid) || WiFi.socketBuffer().connected(_cid);
}

WiFiClient::operator bool()
{
  return (_cid > -1);
}

IPAddress WiFiClient::remoteIP()
{
  return _remoteIp;
}

uint16_t WiFiClient::remotePort()
{
  return _remotePort;
}
