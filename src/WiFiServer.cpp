/*
 * Copyright (c) 2022 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1
 * 
 */

#include "WiFi.h"
#include "WiFiClient.h"

#include "WiFiServer.h"

WiFiServer* WiFiServer::_inst = NULL;

WiFiServer::WiFiServer(uint16_t port) :
  _port(port),
  _cid(-1)
{
}

WiFiServer::~WiFiServer()
{
  if (_inst == this) {
    _inst = NULL;
  }
}

WiFiClient WiFiServer::available(uint8_t* status)
{
  (void)status;

  for (int i = 0; i < 2; i++) {
    if (WiFi._socketBuffer.available(_cid)) {
      return WiFiClient(_cid, WiFi.socketBuffer().remoteIP(_cid), WiFi.socketBuffer().remotePort(_cid));
    }

    WiFi.poll(_cid);
  }

  return WiFiClient(-1, (uint32_t)0, 0);
}

void WiFiServer::begin()
{
  if (_inst != NULL && _inst != this) {
    return;
  }

  WiFi.AT("+TRTRM", "=0", 5000);
  WiFi.AT("+TRSAVE", NULL, 5000);

  char args[1 + 5 + 1];

  sprintf(args, "=%d", _port);

  if (WiFi.AT("+TRTS", args, 1000) != 0) {
    return;
  }

  _cid = 0;

  WiFi.socketBuffer().begin(_cid);
  WiFi.socketBuffer().clear(_cid);

  _inst = this;
}

size_t WiFiServer::write(uint8_t b)
{
  return write(&b, sizeof(b));
}

size_t WiFiServer::write(const uint8_t *buf, size_t size)
{
  size_t written = 0;

  if (size > 2048) {
    size = 2048;
  }

  for (int i = 0; i < WIFI_SERVER_MAX_CLIENTS; i++) {
    if ((uint32_t)_clients[i].remoteIp != 0 && _clients[i].remotePort != 0) {
      char args[1 + 4 + 1 + 15 + 1 + 5 + 1 + 1];

      sprintf(
        args, "%d%d,%d.%d.%d.%d,%d,",
        _cid, size,
        _clients[i].remoteIp[0], _clients[i].remoteIp[1], _clients[i].remoteIp[2], _clients[i].remoteIp[3], 
        _clients[i].remotePort
      );

      if (WiFi.ESC("S", args, buf, size) != 0) {
        setWriteError();
      } else {
        written += size;
      }
    }
  }

  return written;
}

uint8_t WiFiServer::status()
{
  return 0;
}

void WiFiServer::connect(int cid, IPAddress ip, uint16_t port)
{
  (void)cid;

  for (int i = 0; i < WIFI_SERVER_MAX_CLIENTS; i++) {
    if ((uint32_t)_clients[i].remoteIp == 0 && _clients[i].remotePort == 0) {
      _clients[i].remoteIp = ip;
      _clients[i].remotePort = port;

      break;
    }
  }
}

bool WiFiServer::connected(int cid, IPAddress ip, uint16_t port)
{
  (void)cid;

  for (int i = 0; i < WIFI_SERVER_MAX_CLIENTS; i++) {
    if (_clients[i].remoteIp == ip && _clients[i].remotePort == port) {
      return true;
    }
  }

  return false;
}

void WiFiServer::disconnect(int cid, IPAddress ip, uint16_t port)
{
  (void)cid;

  for (int i = 0; i < WIFI_SERVER_MAX_CLIENTS; i++) {
    if (_clients[i].remoteIp == ip && _clients[i].remotePort == port) {
      _clients[i].remoteIp = (uint32_t)0;
      _clients[i].remotePort = 0;

      break;
    }
  }
}
