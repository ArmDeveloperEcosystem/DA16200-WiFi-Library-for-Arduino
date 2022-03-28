/*
 * Copyright (c) 2022 Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1
 * 
 */

#ifndef _WIFI_H_
#define _WIFI_H_

#include <Arduino.h>

#include "utility/WiFiModem.h"
#include "utility/WiFiSocketBuffer.h"

typedef enum {
  WL_NO_SHIELD = 255,
  WL_NO_MODULE = WL_NO_SHIELD,
  WL_IDLE_STATUS = 0,
  WL_NO_SSID_AVAIL,
  WL_SCAN_COMPLETED,
  WL_CONNECTED,
  WL_CONNECT_FAILED,
  WL_CONNECTION_LOST,
  WL_DISCONNECTED,
  WL_AP_LISTENING,
  WL_AP_CONNECTED,
  WL_AP_FAILED
} wl_status_t;

enum wl_enc_type {
  ENC_TYPE_WEP  = 5,
  ENC_TYPE_TKIP = 2,
  ENC_TYPE_CCMP = 4,
  ENC_TYPE_NONE = 7,
  ENC_TYPE_AUTO = 8,

  ENC_TYPE_UNKNOWN = 255
};

typedef enum {
  WL_PING_DEST_UNREACHABLE = -1,
  WL_PING_TIMEOUT = -2,
  WL_PING_UNKNOWN_HOST = -3,
  WL_PING_ERROR = -4
} wl_ping_result_t;

#define WIFI_FIRMWARE_LATEST_VERSION "3.1.2.0"

class WiFiClass {
  public:
    WiFiClass(HardwareSerial& _serial, int rtcWakePin, int wakeUpPin);
    virtual ~WiFiClass();

    uint8_t status();
    uint8_t reasonCode();
    const char* firmwareVersion();
    uint8_t* macAddress(uint8_t* mac);

    int begin(const char* ssid);
    int begin(const char* ssid, uint8_t key_idx, const char* key);
    int begin(const char* ssid, const char *passphrase);

    uint8_t beginAP(const char *ssid);
    uint8_t beginAP(const char *ssid, uint8_t channel);
    uint8_t beginAP(const char *ssid, const char* key);
    uint8_t beginAP(const char *ssid, const char* key, uint8_t channel);

    void disconnect();

    void config(IPAddress local_ip);
    void config(IPAddress local_ip, IPAddress dns_server);
    void config(IPAddress local_ip, IPAddress dns_server, IPAddress gateway);
    void config(IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet);
    void setDNS(IPAddress dns_server1);

    IPAddress localIP();
    IPAddress subnetMask();
    IPAddress gatewayIP();
    const char* SSID();
    uint8_t* BSSID(uint8_t* bssid);
    int32_t RSSI();
    uint8_t encryptionType();

    int8_t scanNetworks();
    const char* SSID(uint8_t networkItem);
    uint8_t encryptionType(uint8_t networkItem);
    uint8_t* BSSID(uint8_t networkItem, uint8_t* bssid);
    uint8_t channel(uint8_t networkItem);
    int32_t RSSI(uint8_t networkItem);

    void end();

    int hostByName(const char* aHostname, IPAddress& aResult);

    int ping(const char* hostname);
    int ping(const String &hostname);
    int ping(IPAddress host);

    unsigned long getTime();

    void lowPowerMode();
    void noLowPowerMode();

    void setTimeout(unsigned long timeout);

    void debug(Print& p);
    void noDebug();

  protected:
    friend class WiFiClient;
    friend class WiFiServer;
    friend class WiFiUDP;

    int AT(const char* command = "", const char* args = NULL, int timeout = 2000);
    int ESC(const char* sequence, const char* args, const uint8_t* buffer, int length, int timeout = 1000);

    void poll(unsigned long timeout);

    void wakeup();

    WiFiSocketBuffer& socketBuffer();

  private:
    int begin(const char* ssid, uint8_t key_idx, const char* key, uint8_t encType);

    int init();

    int setMode(int mode);

    int parseScanNetworksItem(uint8_t networkItem);
    int getNetworkIpInfo(int* iface, uint32_t* ipAddr, uint32_t* netmask, uint32_t* gw);

    static void onExtendedResponseHandler(void* context, const char* prefix, Stream& s);
    void handleExtendedResponse(const char* prefix, Stream& s);
    static void onIrq();
    void handleIrq();

  private:
    WiFiModem _modem;
    WiFiSocketBuffer _socketBuffer;
    String _extendedResponse;
    volatile int _irq;

    wl_status_t _status;
    int _interface;
    int _numConnectedSta;
    char _ssid[32 + 1];
    char _firmwareVersion[sizeof(WIFI_FIRMWARE_LATEST_VERSION)];
    String _scanExtendedResponse;
    struct {
      uint8_t networkItem;
      char ssid[32 + 1];
      uint8_t encryptionType;
      uint8_t bssid[6];
      uint8_t channel;
      int32_t rssi;
    } _scanCache;

    struct {
      IPAddress localIp;
      IPAddress gateway;
      IPAddress subnet;
    } _config;

    int _lowPowerMode;
    unsigned long _timeout;

    int _run;
};

extern WiFiClass WiFi;

#endif
