#include "WiFiUdp.h"

#include "WiFi.h"

WiFiClass::WiFiClass(HardwareSerial& serial, int resetPin) :
  _modem(serial, resetPin),
  _status(WL_NO_SHIELD)
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

  if (_modem.AT("+WFSTAT") == 0) {
    if (_extendedResponse.indexOf("bssid=") != -1) {
      _status = WL_CONNECTED;
    } else {
      _status = WL_DISCONNECTED;
    }
  }

  return _status;
}

const char* WiFiClass::firmwareVersion()
{
  if (_status == WL_NO_SHIELD) {
    if (!init()) {
      return "";
    }
  }

  if (_modem.AT("+VER") != 0) {
    return "";
  }

  memset(_firmwareVersion, 0x00, sizeof(_firmwareVersion));

  // TODO: validate prefix
  strncpy(_firmwareVersion, _extendedResponse.c_str() + 5, sizeof(_firmwareVersion) - 1);

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

  if (_modem.AT("+WFMAC", "=?") != 0) {
    return mac;
  }

  int tmpMac[6] = { 0, 0, 0, 0, 0, 0};

  // TODO: validate prefix
  sscanf(_extendedResponse.c_str(), "+WFMAC:%x:%x:%x:%x:%x:%x",
         &tmpMac[5], &tmpMac[4], &tmpMac[3], &tmpMac[2], &tmpMac[1], &tmpMac[0]);

  for (int i = 0; i < 6; i++) {
    mac[i] = tmpMac[i];
  }

  return mac;
}

int WiFiClass::begin(const char* ssid, const char *passphrase)
{
  if (_status == WL_NO_SHIELD) {
    if (!init()) {
      return _status;
    }
  }

  disconnect();

  if (_modem.AT("+WFDIS", "=1") != 0) {
    _status = WL_CONNECT_FAILED;
    return _status;
  }

  char args[1 + 32 + 1 + 63 + 1];

  sprintf(args, "=%s,'%s'", ssid, passphrase);

  if (_modem.AT("+WFJAPA", args) != 0) {
    _status = WL_CONNECT_FAILED;
    return _status;
  }

  _status = WL_IDLE_STATUS;

  for (unsigned long start = millis(); (millis() - start) < 30000;) {
    _modem.poll(100);

    if (_status != WL_IDLE_STATUS) {
      break;
    }
  }

  if (_status != WL_CONNECTED) {
    _status = WL_CONNECT_FAILED;
  }

  return _status;
}

int WiFiClass::disconnect()
{
  _modem.AT("+WFQAP");

  _status = WL_DISCONNECTED;
}

IPAddress WiFiClass::localIP()
{
  uint32_t ipAddr = 0;

  getNetworkIpInfo(NULL, &ipAddr, NULL, NULL);

  return ipAddr;
}

IPAddress WiFiClass::subnetMask()
{
  uint32_t netmask = 0;

  getNetworkIpInfo(NULL, NULL, &netmask, NULL);

  return netmask;
}

IPAddress WiFiClass::gatewayIP()
{
  uint32_t gw = 0;

  getNetworkIpInfo(NULL, NULL, NULL, &gw);

  return gw;
}

const char* WiFiClass::SSID()
{
  _ssid[0] = '\0';

  if (_modem.AT("+WFSTAT") == 0) {
    // TODO: validate prefix
    int ssidIndex = _extendedResponse.indexOf("\nssid=");
    if (ssidIndex != -1) {
      sscanf(_extendedResponse.c_str() + ssidIndex + 1, "ssid=%32s\n", _ssid);
    }
  }

  return _ssid;
}

uint8_t* WiFiClass::BSSID(uint8_t* bssid)
{
  memset(bssid, 0x00, 6);

  int tmpBssid[6] = { 0, 0, 0, 0, 0, 0 };

  if (_modem.AT("+WFSTAT") == 0) {
    // TODO: validate prefix
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

  if (_modem.AT("+WFRSSI") == 0) {
    // TODO: validate prefix
    sscanf(_extendedResponse.c_str(), "+RSSI:%d", &rssi);
  }

  return rssi;
}

uint8_t WiFiClass::encryptionType()
{
  uint8_t encType = ENC_TYPE_UNKNOWN;

  if (_modem.AT("+WFSTAT") == 0) {
    // TODO: validate prefix
    if (_extendedResponse.indexOf("key_mgmt=WPA2-AUTO") != -1) { // TODO: verify
      encType = ENC_TYPE_AUTO;
    } else if (_extendedResponse.indexOf("key_mgmt=WPA2-PSK") != -1) {
      encType = ENC_TYPE_CCMP;
    } else if (_extendedResponse.indexOf("key_mgmt=WPA-PSK") != -1) {
      encType = ENC_TYPE_TKIP;
    } else if (_extendedResponse.indexOf("key_mgmt=WEP") != -1) { // TODO: verify
      encType = ENC_TYPE_WEP;
    } else if (_extendedResponse.indexOf("key_mgmt=OPEN") != -1) { // TODO: verify
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

  if (_modem.AT("+WFSCAN", NULL, 5000) != 0) {
    _status = WL_NO_SSID_AVAIL;
    return -1;
  }

  int numSsid = -1;

  // TODO: validate prefix
  _scanExtendedResponse = _extendedResponse.substring(8);

  for (int i = 0; i < _scanExtendedResponse.length(); i++) {
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

  _status = WL_NO_SHIELD;

  memset(_firmwareVersion, 0x00, sizeof(_firmwareVersion));
  _scanCache.networkItem = 255;
  _scanExtendedResponse = "";
}

int WiFiClass::hostByName(const char* aHostname, IPAddress& aResult)
{
  aResult = (uint32_t)0;

  char args[1 + strlen(aHostname) + 1];

  sprintf(args, "=%s", aHostname);

  for (int retry = 0; retry < 10 && (uint32_t)aResult == 0; retry++) {
    if (_modem.AT("+NWHOST", args, 10000) != 0) {
      return 0;
    }

    int ipAddr[4] = { 0, 0, 0, 0 };

    // TODO: validate prefix
    sscanf(
      _extendedResponse.c_str(), "+NWHOST:%d.%d.%d.%d\n",
      &ipAddr[0], &ipAddr[1], &ipAddr[2], &ipAddr[3]
    );

    aResult = IPAddress(ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
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

  sprintf(args, "=0,%d.%d.%d.%d,1", host[0], host[1], host[2], host[3]);

  int result = _modem.AT("+NWPING", args, 10000);
  if (result == -100) {
    return WL_PING_TIMEOUT;
  } else if (result != 0) {
    return WL_PING_ERROR;
  }

  int sentCount = 0;
  int recvCount = 0;
  int avgTime = 0;
  int minTime = 0;
  int maxTime = 0;

  // TODO: validate prefix
  sscanf(
    _extendedResponse.c_str(), "+NWPING:%d,%d,%d,%d,%d\n",
    &sentCount, &recvCount, &avgTime, &minTime, &maxTime
  );

  if (recvCount == 0) {
    return WL_PING_DEST_UNREACHABLE;
  }

  return maxTime;
}

int WiFiClass::init()
{
  _status = WL_NO_SHIELD;

  _modem.begin(115200);
  _modem.onExtendedResponse(WiFiClass::onExtendedResponseHandler, this);

  if (_modem.AT("Z") != 0) {
    end();

    return 0;
  }

  if (_modem.AT("+WFMODE", "=0") != 0) {
    end();

    return 0;
  }

  if (_modem.AT("+TRTALL") != 0) {
    end();

    return 0;
  }

  if (_modem.AT("+NWSNTP", "=1") != 0) {
    end();

    return 0;
  }

  _status = WL_IDLE_STATUS;

  memset(_firmwareVersion, 0x00, sizeof(_firmwareVersion));
  _scanCache.networkItem = 255;
  _scanExtendedResponse = "";

  return 1;
}

int WiFiClass::parseScanNetworksItem(uint8_t networkItem)
{
  int index;
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
  int extra;

  _scanCache.ssid[0] = '\0';

  _scanExtendedResponse.setCharAt(endIndex, '\0');

  sscanf(
    _scanExtendedResponse.c_str() + startIndex,
    "%x:%x:%x:%x:%x:%x\t%d\t%d\t%64s\t%32s",
    &bssid[5], &bssid[4], &bssid[3], &bssid[2], &bssid[1], &bssid[0],
    &frequency,
    &rssi,
    flags,
    _scanCache.ssid,
    &extra
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

  int flagsLength = strlen(flags);

  if (strnstr(flags, "[WPA-AUTO", flagsLength) != NULL) { // TODO: verify
    encType = ENC_TYPE_AUTO;
  } else if (strnstr(flags, "[WPA2-PSK", flagsLength) != NULL) {
    encType = ENC_TYPE_CCMP;
  } else if (strnstr(flags, "[WPA-PSK", flagsLength) != NULL) {
    encType = ENC_TYPE_TKIP;
  } else if (strnstr(flags, "[WEP", flagsLength) != NULL) { // TODO: verify
    encType = ENC_TYPE_WEP;
  } else if (strnstr(flags, "[OPEN", flagsLength) != NULL) { // TODO: verify
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
  if (_modem.AT("+NWIP=?") == 0) {
    int ifaceTmp;
    int ipAddrOctets[4] = {0, 0, 0, 0};
    int netmaskOctets[4] = {0, 0, 0, 0};
    int gwOctets[4] = {0, 0, 0, 0};

    // TODO: validate prefix
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
  // TODO: check prefix
  if (strcmp("+TRDTC:", prefix) == 0 || strcmp("+TRDUS:", prefix) == 0) {
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

            int read = 0;

            if (cid == 2 && WiFiUDP::_inst != NULL) {
              read = WiFiUDP::_inst->receive(IPAddress(ipAddrOctets[0], ipAddrOctets[1], ipAddrOctets[2], ipAddrOctets[3]), port, s, length);
            }

            length -= read;

            while (length) {
              if (s.available()) {
                s.read();

                length--;
              }
            }

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
    }
  }
}

WiFiClass WiFi(SERIAL_PORT_HARDWARE, 7);
