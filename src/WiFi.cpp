/*
 * Copyright (c) 2022 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1
 * 
 */

#include <string.h>
#include <time.h>

#include "WiFiServer.h"

#include "WiFi.h"

#define WIFI_DEFAULT_TIMEOUT (30 * 1000) // 30 seconds

WiFiClass::WiFiClass(HardwareSerial& serial, int rtcWakePin, int wakeUpPin) :
  _modem(serial, rtcWakePin, wakeUpPin),
  _irq(0),
  _status(WL_NO_SHIELD),
  _interface(0),
  _numConnectedSta(0),
  _lowPowerMode(0),
  _timeout(WIFI_DEFAULT_TIMEOUT)
{
  _extendedResponse.reserve(64);
  _scanCache.networkItem = 255;
}

WiFiClass::~WiFiClass()
{
}

uint8_t WiFiClass::status()
{
  if (_status == WL_NO_SHIELD) {
    init();

    return _status;
  }

  if (_interface == 0) {
    if (this->AT("+WFSTAT") == 0) {
      if (_extendedResponse.indexOf("bssid=") != -1) {
        _status = WL_CONNECTED;
      } else {
        _status = WL_DISCONNECTED;
      }
    }
  } else {
    _modem.poll(0);
  }

  return _status;
}

uint8_t WiFiClass::reasonCode()
{
  int reason = 0;

  if (_status == WL_NO_SHIELD) {
    init();

    return reason;
  }

  if (this->AT("+WFSTAT") == 0 && _extendedResponse.startsWith("+WFSTAT:")) {
    int reasonIndex = _extendedResponse.indexOf("\ndisconnect_reason=");
    if (reasonIndex != -1) {
      sscanf(_extendedResponse.c_str() + reasonIndex + 1, "disconnect_reason=%d\n", &reason);
    }
  }

  if (reason < 0) {
    reason *= -1;
  }

  return reason;
}

const char* WiFiClass::firmwareVersion()
{
  if (_status == WL_NO_SHIELD) {
    if (!init()) {
      return "";
    }
  }

  if (this->AT("+SDKVER") != 0 || !_extendedResponse.startsWith("+SDKVER:")) {
    return "";
  }

  memset(_firmwareVersion, 0x00, sizeof(_firmwareVersion));

  strncpy(_firmwareVersion, _extendedResponse.c_str() + 8, sizeof(_firmwareVersion) - 1);

  return _firmwareVersion;
}

uint8_t* WiFiClass::macAddress(uint8_t* mac)
{
  memset(mac, 0x00, 6);

  if (_status == WL_NO_SHIELD) {
    if (!init()) {
      return mac;
    }
  }

  if (this->AT("+WFMAC", "=?") != 0 || !_extendedResponse.startsWith("+WFMAC:")) {
    return mac;
  }

  int tmpMac[6] = { 0, 0, 0, 0, 0, 0};

  sscanf(_extendedResponse.c_str(), "+WFMAC:%x:%x:%x:%x:%x:%x",
         &tmpMac[5], &tmpMac[4], &tmpMac[3], &tmpMac[2], &tmpMac[1], &tmpMac[0]);

  for (int i = 0; i < 6; i++) {
    mac[i] = tmpMac[i];
  }

  return mac;
}

int WiFiClass::begin(const char* ssid)
{
  return begin(ssid, 0, NULL, ENC_TYPE_NONE);
}

int WiFiClass::begin(const char* ssid, uint8_t key_idx, const char* key)
{
  return begin(ssid, key_idx, key, ENC_TYPE_WEP);
}

int WiFiClass::begin(const char* ssid, const char *passphrase)
{
  return begin(ssid, 0, passphrase, ENC_TYPE_TKIP);
}

int WiFiClass::begin(const char* ssid, uint8_t key_idx, const char* key, uint8_t encType)
{
  if (_status == WL_NO_SHIELD) {
    if (!init()) {
      return _status;
    }
  }

  disconnect();

  if (!setMode(0)) {
    _status = WL_CONNECT_FAILED;
    return _status;
  }

  char args[1 + 1 + 32 + 1 + 1 + 63 + 1];
  const char* command = "+WFJAPA";

  if (encType == ENC_TYPE_NONE) {
    sprintf(args, "='%s'", ssid);
  } else if (encType == ENC_TYPE_WEP) {
    command = "+WFJAP";
    sprintf(args, "='%s',1,%d,%s", ssid, key_idx, key);
  } else {
    sprintf(args, "='%s','%s'", ssid, key);
  }

  if (this->AT(command, args) != 0) {
    _status = WL_CONNECT_FAILED;
    return _status;
  }

  _status = WL_IDLE_STATUS;

  for (unsigned long start = millis(); (millis() - start) < _timeout;) {
    _modem.poll(100);

    if (_status == WL_CONNECT_FAILED) {
      return _status;
    } else if (_status != WL_IDLE_STATUS) {
      break;
    }
  }

  if ((uint32_t)_config.localIp != 0) {
    this->AT("+NWDHC", "=0");
  } else {
    this->AT("+NWDHC", "=1");
  }

  if ((uint32_t)_config.localIp != 0) {
    char args[1 + 1 + 1 + 15 + 1 + 15 + 1 + 15 + 1];

    sprintf(
      args, "=%d,%d.%d.%d.%d,%d.%d.%d.%d,%d.%d.%d.%d",
      _interface,
      _config.localIp[0], _config.localIp[1], _config.localIp[2], _config.localIp[3],
      _config.subnet[0], _config.subnet[1], _config.subnet[2], _config.subnet[3],
      _config.gateway[0], _config.gateway[1], _config.gateway[2], _config.gateway[3]
    );
    
    this->AT("+NWIP", args);
  }

  if (_status != WL_CONNECTED) {
    _status = WL_CONNECT_FAILED;
  }

  return _status;
}

uint8_t WiFiClass::beginAP(const char *ssid)
{
  return beginAP(ssid, 1);
}

uint8_t WiFiClass::beginAP(const char *ssid, uint8_t channel)
{
  return beginAP(ssid, NULL, channel);
}

uint8_t WiFiClass::beginAP(const char *ssid, const char* key)
{
  return beginAP(ssid, key, 1);
}

uint8_t WiFiClass::beginAP(const char *ssid, const char* key, uint8_t channel)
{
  if (_status == WL_NO_SHIELD) {
    if (!init()) {
      return _status;
    }
  }

  disconnect();

  if (!setMode(1)) {
    _status = WL_AP_FAILED;
    return _status;
  }

  char args[1 + 1 + 32 + 1 + 5 + 63 + 1 + 1];

  if (key == NULL) {
    sprintf(args, "='%s',0", ssid);
  } else {
    sprintf(args, "='%s',4,2,'%s'", ssid, key);
  }

  if (this->AT("+WFSAP", args, 5000)) {
    _status = WL_AP_FAILED;
    return _status;
  }

  sprintf(args, "=%d", channel);
  if (this->AT("+WFAPCH", args)) {
    _status = WL_AP_FAILED;
    return _status;
  }

  if ((uint32_t)_config.localIp != 0) {
    char args[1 + 1 + 1 + 15 + 1 + 15 + 1 + 15 + 1];

    sprintf(
      args, "=%d,%d.%d.%d.%d,%d.%d.%d.%d,%d.%d.%d.%d",
      _interface,
      _config.localIp[0], _config.localIp[1], _config.localIp[2], _config.localIp[3],
      _config.subnet[0], _config.subnet[1], _config.subnet[2], _config.subnet[3],
      _config.gateway[0], _config.gateway[1], _config.gateway[2], _config.gateway[3]
    );
    
    this->AT("+NWIP", args);
  }

  if (this->AT("+NWDHS", "=1", 5000)) {
    _status = WL_AP_FAILED;
    return _status;
  }

  _status = WL_AP_LISTENING;
  _numConnectedSta = 0;

  return _status;
}

void WiFiClass::disconnect()
{
  if (_interface == 0) {
    this->AT("+WFQAP");
  } else {
    this->AT("+WFTAP");
  }

  _status = WL_DISCONNECTED;
}

void WiFiClass::config(IPAddress local_ip)
{
  IPAddress dns_server(local_ip[0], local_ip[1], local_ip[2], 1);

  config(local_ip, dns_server);
}

void WiFiClass::config(IPAddress local_ip, IPAddress dns_server)
{
  IPAddress gateway(local_ip[0], local_ip[1], local_ip[2], 1);

  config(local_ip, dns_server, gateway);
}

void WiFiClass::config(IPAddress local_ip, IPAddress dns_server, IPAddress gateway)
{
  IPAddress subnet(255, 255, 255, 0);

  config(local_ip, dns_server, gateway, subnet);
}

void WiFiClass::config(IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet)
{
  _config.localIp = local_ip;
  _config.gateway = gateway;
  _config.subnet = subnet;

  setDNS(dns_server);
}

void WiFiClass::setDNS(IPAddress dns_server1)
{
  char args[1 + 15 + 1];

  sprintf(args, "=%d.%d.%d.%d", dns_server1[0], dns_server1[1], dns_server1[2], dns_server1[3]);

  this->AT("+NWDNS", args);
}

IPAddress WiFiClass::localIP()
{
  uint32_t ipAddr = 0;

  getNetworkIpInfo(&_interface, &ipAddr, NULL, NULL);

  return ipAddr;
}

IPAddress WiFiClass::subnetMask()
{
  uint32_t netmask = 0;

  getNetworkIpInfo(&_interface, NULL, &netmask, NULL);

  return netmask;
}

IPAddress WiFiClass::gatewayIP()
{
  uint32_t gw = 0;

  getNetworkIpInfo(&_interface, NULL, NULL, &gw);

  return gw;
}

const char* WiFiClass::SSID()
{
  _ssid[0] = '\0';

  if (this->AT("+WFSTAT") == 0 && _extendedResponse.startsWith("+WFSTAT:")) {
    int ssidIndex = _extendedResponse.indexOf("\nssid=");
    if (ssidIndex != -1) {
      sscanf(_extendedResponse.c_str() + ssidIndex + 1, "ssid=%32[^\n]\n", _ssid);
    }
  }

  return _ssid;
}

uint8_t* WiFiClass::BSSID(uint8_t* bssid)
{
  memset(bssid, 0x00, 6);

  int tmpBssid[6] = { 0, 0, 0, 0, 0, 0 };

  if (this->AT("+WFSTAT") == 0 && _extendedResponse.startsWith("+WFSTAT:")) {
    int bssidIndex = _extendedResponse.indexOf("\nbssid=");
    if (bssidIndex != -1) {
      sscanf(
        _extendedResponse.c_str() + bssidIndex + 1, "bssid=%x:%x:%x:%x:%x:%x\n",
        &tmpBssid[5], &tmpBssid[4], &tmpBssid[3], &tmpBssid[2], &tmpBssid[1], &tmpBssid[0]
      );
    }
  }

  for (int i = 0; i < 6; i++) {
    bssid[i] = tmpBssid[i];
  }

  return bssid;
}

int32_t WiFiClass::RSSI()
{
  int rssi = 0;

  if (this->AT("+WFRSSI") == 0 && _extendedResponse.startsWith("+RSSI:")) {
    sscanf(_extendedResponse.c_str(), "+RSSI:%d", &rssi);
  }

  return rssi;
}

uint8_t WiFiClass::encryptionType()
{
  uint8_t encType = ENC_TYPE_UNKNOWN;

  if (this->AT("+WFSTAT") == 0 && _extendedResponse.startsWith("+WFSTAT:")) {
    if (_extendedResponse.indexOf("key_mgmt=WPA2-AUTO") != -1) { // TODO: verify
      encType = ENC_TYPE_AUTO;
    } else if (_extendedResponse.indexOf("key_mgmt=WPA2-PSK") != -1) {
      encType = ENC_TYPE_CCMP;
    } else if (_extendedResponse.indexOf("key_mgmt=WPA-PSK") != -1) {
      encType = ENC_TYPE_TKIP;
    } else if (_extendedResponse.indexOf("group_cipher=WEP") != -1) {
      encType = ENC_TYPE_WEP;
    } else if (_extendedResponse.indexOf("key_mgmt=NONE") != -1) {
      encType = ENC_TYPE_NONE;
    }

    if (_interface == 1 && encType == ENC_TYPE_UNKNOWN) {
      encType = ENC_TYPE_NONE;
    }
  }

  return encType;
}

int8_t WiFiClass::scanNetworks()
{
  _scanExtendedResponse = "";
  _scanCache.networkItem = 255;

  if (_status == WL_NO_SHIELD) {
    if (!init()) {
      return -1;
    }
  }

  if (this->AT("+WFSCAN", NULL, 5000) != 0 || !_extendedResponse.startsWith("+WFSCAN:")) {
    _status = WL_NO_SSID_AVAIL;
    return -1;
  }

  int numSsid = -1;

  _scanExtendedResponse = _extendedResponse.substring(8);

  for (unsigned int i = 0; i < _scanExtendedResponse.length(); i++) {
    if (_scanExtendedResponse[i] == '\n') {
      numSsid++;
    }
  }

  if (numSsid > 0) {
    _status = WL_SCAN_COMPLETED;
  } else {
    _status = WL_NO_SSID_AVAIL;
  }

  return numSsid;
}

const char* WiFiClass::SSID(uint8_t networkItem)
{
  if (_scanCache.networkItem != networkItem) {
    if (!parseScanNetworksItem(networkItem)) {
      return "";
    }
  }

  return _scanCache.ssid;
}

uint8_t WiFiClass::encryptionType(uint8_t networkItem)
{
  if (_scanCache.networkItem != networkItem) {
    if (!parseScanNetworksItem(networkItem)) {
      return ENC_TYPE_UNKNOWN;
    }
  }

  return _scanCache.encryptionType;
}

uint8_t* WiFiClass::BSSID(uint8_t networkItem, uint8_t* bssid)
{
  memset(bssid, 0x00, 6);

  if (_scanCache.networkItem != networkItem) {
    if (!parseScanNetworksItem(networkItem)) {
      return bssid;
    }
  }

  memcpy(bssid, _scanCache.bssid, 6);

  return bssid;
}

uint8_t WiFiClass::channel(uint8_t networkItem)
{
  if (_scanCache.networkItem != networkItem) {
    if (!parseScanNetworksItem(networkItem)) {
      return 0;
    }
  }

  return _scanCache.channel;
}

int32_t WiFiClass::RSSI(uint8_t networkItem)
{
  if (_scanCache.networkItem != networkItem) {
    if (!parseScanNetworksItem(networkItem)) {
      return 0;
    }
  }

  return _scanCache.rssi;
}

void WiFiClass::end()
{
  _modem.end();
  _socketBuffer.clear();

  _status = WL_NO_SHIELD;
  _numConnectedSta = 0;

  memset(_firmwareVersion, 0x00, sizeof(_firmwareVersion));
  _scanCache.networkItem = 255;
  _scanExtendedResponse = "";

  _config.localIp = (uint32_t)0;
  _config.gateway = (uint32_t)0;
  _config.subnet = (uint32_t)0;

  _lowPowerMode = 0;
  _timeout = WIFI_DEFAULT_TIMEOUT;
}

int WiFiClass::hostByName(const char* aHostname, IPAddress& aResult)
{
  aResult = (uint32_t)0;

  char args[1 + strlen(aHostname) + 1];

  sprintf(args, "=%s", aHostname);

  for (int retry = 0; retry < 30; retry++) {
    if (this->AT("+NWHOST", args, 10000) != 0 || !_extendedResponse.startsWith("+NWHOST:")) {
      return 0;
    }

    int ipAddr[4] = { 0, 0, 0, 0 };

    sscanf(
      _extendedResponse.c_str(), "+NWHOST:%d.%d.%d.%d\n",
      &ipAddr[0], &ipAddr[1], &ipAddr[2], &ipAddr[3]
    );

    aResult = IPAddress(ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);

    if ((uint32_t)aResult == 0) {
      delay(500);
    } else {
      break;
    }
  }

  if ((uint32_t)aResult == 0) {
    return 0;
  }

  return 1;
}

int WiFiClass::ping(const char* hostname)
{
  IPAddress host;

  if (!hostByName(hostname, host)) {
    return WL_PING_UNKNOWN_HOST;
  }

  return ping(host);
}

int WiFiClass::ping(const String &hostname)
{
  return ping(hostname.c_str());
}

int WiFiClass::ping(IPAddress host)
{
  char args[3 + 15 + 2 + 1];

  sprintf(
    args, "=%d,%d.%d.%d.%d,1",
    _interface,
    host[0], host[1], host[2], host[3]
  );

  int result = this->AT("+NWPING", args, 10000);
  if (result == -100) {
    return WL_PING_TIMEOUT;
  } else if (result != 0 || !_extendedResponse.startsWith("+NWPING:")) {
    return WL_PING_ERROR;
  }

  int sentCount = 0;
  int recvCount = 0;
  int avgTime = 0;
  int minTime = 0;
  int maxTime = 0;

  sscanf(
    _extendedResponse.c_str(), "+NWPING:%d,%d,%d,%d,%d\n",
    &sentCount, &recvCount, &avgTime, &minTime, &maxTime
  );

  if (recvCount == 0) {
    return WL_PING_DEST_UNREACHABLE;
  }

  return maxTime;
}

unsigned long WiFiClass::getTime()
{
  time_t t = 0;

  if (this->AT("+TIME", "=?") == 0) {
    struct tm tm;

    memset(&tm, 0x00, sizeof(tm));

    sscanf(
      _extendedResponse.c_str(), "+TIME:%d-%d-%d,%d:%d:%d\n",
      &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
      &tm.tm_hour, &tm.tm_min, &tm.tm_sec
    );

    tm.tm_year -= 1900;
		tm.tm_mon -= 1;
		tm.tm_isdst = -1;

		t = mktime(&tm);
  }

  return t;
}

void WiFiClass::lowPowerMode()
{ 
  _lowPowerMode = 1;
  _irq = 0;

  AT();
}

void WiFiClass::noLowPowerMode()
{
  _lowPowerMode = 0;

  AT();
}

void WiFiClass::setTimeout(unsigned long timeout)
{
  _timeout = timeout;
}

void WiFiClass::debug(Print& p)
{
  _modem.debug(p);
}

void WiFiClass::noDebug()
{
  _modem.noDebug();
}

int WiFiClass::AT(const char* command, const char* args, int timeout)
{
  wakeup();

  int result = _modem.AT(command, args, timeout);

  if (_lowPowerMode) {
    _modem.AT("+SETDPMSLPEXT", NULL, 1000);
  }

  return result;
}

int WiFiClass::ESC(const char* sequence, const char* args, const uint8_t* buffer, int length, int timeout)
{
  wakeup();

  int result = _modem.ESC(sequence, args, buffer, length, timeout);

  if (_lowPowerMode) {
    _modem.AT("+SETDPMSLPEXT", NULL, 1000);
  }

  return result;
}

void WiFiClass::poll(unsigned long timeout)
{
  int sleep = 0;

  if (_irq) {
    sleep = 1;
    _irq = 0;
    wakeup();
  }

  _modem.poll(timeout);

  if (_lowPowerMode && sleep) {
    _modem.AT("+SETDPMSLPEXT", NULL, 1000);
  }
}

void WiFiClass::wakeup()
{
  _run = 0;
  _modem.wakeup();

  // TODO: remove wake timeout, seems only needed for case after WiFi.noLowPowerMode() is called
  for (unsigned long start = millis(); (millis() - start) < 100;) {
    _modem.poll(0);

    if (_run != 0) {
      break;
    }
  }

  _irq = 0;

  _modem.AT("+MCUWUDONE", NULL, 1000);
  _modem.AT("+CLRDPMSLPEXT", NULL, 1000);
}

WiFiSocketBuffer& WiFiClass::socketBuffer()
{
  return _socketBuffer;
}

int WiFiClass::init()
{
  _status = WL_NO_SHIELD;

  _modem.begin(115200);
  _modem.onExtendedResponse(WiFiClass::onExtendedResponseHandler, this);
  _modem.onIrq(WiFiClass::onIrq);

  _modem.wakeup();
  delay(150);

  this->AT("+MCUWUDONE");
  this->AT("+CLRDPMSLPEXT");

  if (this->AT("Z") != 0) {
    end();

    return 0;
  }


  if (this->AT("+WFDIS", "=1") != 0) {
    end();

    return 0;
  }

  if (this->AT("+TRTALL", NULL, 5000) != 0) {
    end();

    return 0;
  }

  if (this->AT("+NWSNTP", "=1") != 0) {
    end();

    return 0;
  }

  _status = WL_IDLE_STATUS;

  memset(_firmwareVersion, 0x00, sizeof(_firmwareVersion));
  _scanCache.networkItem = 255;
  _scanExtendedResponse = "";
  _irq = 0;

  if (this->AT("+DPM", "=1") != 0) {
    return 0;
  }

  _interface = -1;
  for (unsigned long start = millis(); (millis() - start) < 5000;) {
    _modem.poll(100);

    if (_interface != -1) {
      break;
    }
  }

  this->AT("+WFQAP");
  this->AT("+WFTAP");

  return 1;
}

int WiFiClass::setMode(int mode)
{
  char args[3];
  
  _interface = -1;

  if (mode == 0) {
    sprintf(args, "=%d", mode);
    if (this->AT("+WFMODE", args) != 0) {
      return 0;
    }
  
    if (this->AT("+RESTART") != 0) {
      return 0;
    }
  } else {
    if (this->AT("+DEFAP", NULL, 5000) != 0) {
      return 0;
    }
  }

  for (unsigned long start = millis(); (millis() - start) < 5000;) {
    _modem.poll(100);

    if (_interface != -1) {
      return 1;
    }
  }

  return 0;
}

int WiFiClass::parseScanNetworksItem(uint8_t networkItem)
{
  unsigned int index;
  int startIndex = 0;
  int endIndex = 0;
  int newLineCount = 0;

  for (index = 0; index < _scanExtendedResponse.length(); index++) {
    if (_scanExtendedResponse[index] == '\n') {
      newLineCount++;

      if (networkItem == newLineCount) {
        startIndex = index + 1;
      } else if ((networkItem + 1) == newLineCount) {
        endIndex = index;
        break;
      }
    }
  }

  if (index >= _scanExtendedResponse.length()) {
    return 0;
  }

  int bssid[6] = { 0, 0, 0, 0, 0, 0 };
  int frequency = 0;
  int rssi = 0;
  char flags[64 + 1];
  int channel = 0;
  uint8_t encType = ENC_TYPE_UNKNOWN;

  _scanCache.ssid[0] = '\0';

  _scanExtendedResponse.setCharAt(endIndex, '\0');

  sscanf(
    _scanExtendedResponse.c_str() + startIndex,
    "%x:%x:%x:%x:%x:%x\t%d\t%d\t%64s\t%32[^\t\n]",
    &bssid[5], &bssid[4], &bssid[3], &bssid[2], &bssid[1], &bssid[0],
    &frequency,
    &rssi,
    flags,
    _scanCache.ssid
  );

  _scanExtendedResponse.setCharAt(endIndex, '\n');

  // https://en.wikipedia.org/wiki/List_of_WLAN_channels#2.4_GHz_(802.11b/g/n/ax)
  switch (frequency) {
    case 2412: channel = 1; break;
    case 2417: channel = 2; break;
    case 2422: channel = 3; break;
    case 2427: channel = 4; break;
    case 2432: channel = 5; break;
    case 2437: channel = 6; break;
    case 2442: channel = 7; break;
    case 2447: channel = 8; break;
    case 2452: channel = 9; break;
    case 2457: channel = 10; break;
    case 2462: channel = 11; break;
    case 2467: channel = 12; break;
    case 2472: channel = 13; break;
    case 2484: channel = 14; break;
    default:   channel = 0; break;
  }

  if (strstr(flags, "[WPA-AUTO") != NULL) { // TODO: verify
    encType = ENC_TYPE_AUTO;
  } else if (strstr(flags, "[WPA2-PSK") != NULL) {
    encType = ENC_TYPE_CCMP;
  } else if (strstr(flags, "[WPA-PSK") != NULL) {
    encType = ENC_TYPE_TKIP;
  } else if (strstr(flags, "[WEP") != NULL) {
    encType = ENC_TYPE_WEP;
  } else if (strcmp(flags, "[ESS]") == 0) {
    encType = ENC_TYPE_NONE;
  }

  _scanCache.networkItem = networkItem;
  _scanCache.encryptionType = encType;
  for (int i = 0; i < 6; i++) {
    _scanCache.bssid[i] = bssid[i];
  }
  _scanCache.channel = channel;
  _scanCache.rssi = rssi;

  return 1;
}

int WiFiClass::getNetworkIpInfo(int* iface, uint32_t* ipAddr, uint32_t* netmask, uint32_t* gw)
{
  if (this->AT("+NWIP=?") == 0 && _extendedResponse.startsWith("+NWIP:")) {
    int ifaceTmp;
    int ipAddrOctets[4] = {0, 0, 0, 0};
    int netmaskOctets[4] = {0, 0, 0, 0};
    int gwOctets[4] = {0, 0, 0, 0};

    sscanf(
      _extendedResponse.c_str(),
      "+NWIP:%d,%d.%d.%d.%d,%d.%d.%d.%d,%d.%d.%d.%d",
      &ifaceTmp,
      &ipAddrOctets[0], &ipAddrOctets[1], &ipAddrOctets[2], &ipAddrOctets[3],
      &netmaskOctets[0], &netmaskOctets[1], &netmaskOctets[2], &netmaskOctets[3],
      &gwOctets[0], &gwOctets[1], &gwOctets[2], &gwOctets[3]
    );

    if (iface != NULL) {
      *iface = ifaceTmp;
    }

    if (ipAddr != NULL) {
      *ipAddr = IPAddress(ipAddrOctets[0], ipAddrOctets[1], ipAddrOctets[2], ipAddrOctets[3]);
    }

    if (netmask != NULL) {
      *netmask = IPAddress(netmaskOctets[0], netmaskOctets[1], netmaskOctets[2], netmaskOctets[3]);
    }

    if (gw != NULL) {
      *gw = IPAddress(gwOctets[0], gwOctets[1], gwOctets[2], gwOctets[3]);
    }

    return 1;
  }

  return 0;
}

void WiFiClass::onExtendedResponseHandler(void* context, const char* prefix, Stream& s)
{
  ((WiFiClass*)context)->handleExtendedResponse(prefix, s);
}

void WiFiClass::handleExtendedResponse(const char* prefix, Stream& s)
{
  if (strcmp("+TRDTC:", prefix) == 0 || strcmp("+TRDTS:", prefix) == 0 || strcmp("+TRDUS:", prefix) == 0) {
    int commaCount = 0;

    _extendedResponse = "";

    while (1) {
      if (s.available()) {
        char c = s.read();

        _extendedResponse += c;

        if (c == ',') {
          commaCount++;

          if (commaCount == 4) {
            int cid;
            int ipAddrOctets[4] = {0, 0, 0, 0};
            int port;
            int length;

            sscanf(
              _extendedResponse.c_str(),
              "%d,%d.%d.%d.%d,%d,%d,",
              &cid,
              &ipAddrOctets[0], &ipAddrOctets[1], &ipAddrOctets[2], &ipAddrOctets[3],
              &port, &length
            );

            _socketBuffer.receive(cid, IPAddress(ipAddrOctets[0], ipAddrOctets[1], ipAddrOctets[2], ipAddrOctets[3]), port, s, length);

            break;
          }
        }
      }
    }
  } else {
    _extendedResponse = prefix;

    while (1) {
      if (s.available()) {
        char c = s.read();

        _extendedResponse += c;

        if (c == '\n') {
          if (_extendedResponse.endsWith("\r\n")) {
            break;
          }
        }
      }
    }

    if (_extendedResponse.startsWith("+WFJAP:1")) {
      _status = WL_CONNECTED;
    } else if (_extendedResponse.startsWith("+WFJAP:0")) {
      _status = WL_CONNECT_FAILED;
    } else if (_extendedResponse.startsWith("+WFDAP:")) {
      _status = WL_CONNECTION_LOST;
    } else if (_extendedResponse.startsWith("+WFCST:")) {
      _status = WL_AP_CONNECTED;
      _numConnectedSta++;
    } else if (_extendedResponse.startsWith("+WFDST:")) {
      _numConnectedSta--;
      if (_numConnectedSta < 1) {
        _status = WL_AP_LISTENING;
      }
    } else if (_extendedResponse.startsWith("+TRXTC:1")) {
      _socketBuffer.disconnect(1);
    } else if (_extendedResponse.startsWith("+TRCTS:0")) {
      int cid;
      int ipAddrOctets[4] = {0, 0, 0, 0};
      int port;

      sscanf(
        _extendedResponse.c_str(),
        "+TRCTS:%d,%d.%d.%d.%d,%d",
        &cid,
        &ipAddrOctets[0], &ipAddrOctets[1], &ipAddrOctets[2], &ipAddrOctets[3],
        &port
      );

      if (WiFiServer::_inst != NULL) {
        WiFiServer::_inst->connect(cid, IPAddress(ipAddrOctets[0], ipAddrOctets[1], ipAddrOctets[2], ipAddrOctets[3]), port);
      }
    } else if (_extendedResponse.startsWith("+TRXTS:0")) {
      int cid;
      int ipAddrOctets[4] = {0, 0, 0, 0};
      int port;

      sscanf(
        _extendedResponse.c_str(),
        "+TRXTS:%d,%d.%d.%d.%d,%d",
        &cid,
        &ipAddrOctets[0], &ipAddrOctets[1], &ipAddrOctets[2], &ipAddrOctets[3],
        &port
      );

      if (WiFiServer::_inst != NULL) {
        WiFiServer::_inst->disconnect(cid, IPAddress(ipAddrOctets[0], ipAddrOctets[1], ipAddrOctets[2], ipAddrOctets[3]), port);
      }
    } else if (_extendedResponse.startsWith("+INIT:DONE,")) {
      sscanf(
        _extendedResponse.c_str(),
        "+INIT:DONE,%d",
        &_interface
      );
    } else if (_extendedResponse.startsWith("+RUN:") || _extendedResponse.startsWith("+INIT:WAKEUP,")) {
      _run = 1;
    }
  }
}

void WiFiClass::onIrq()
{
  WiFi.handleIrq();
}

void WiFiClass::handleIrq()
{
  _irq = 1;
}

WiFiClass WiFi(SERIAL_PORT_HARDWARE, 5, 2);
