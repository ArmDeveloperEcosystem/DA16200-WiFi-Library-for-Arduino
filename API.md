# DA16200 Wi-Fi Library for Arduino - API

This library aims to be compatible with with the official Arduino [WiFi](https://www.arduino.cc/en/Reference/WiFi) and [WiFi101](https://www.arduino.cc/en/Reference/WiFi101) libraries where possible.

The links below refer to the official Arduino library documentation as the same API and behaviour is intended.

## `WiFi`

* [`WiFi.begin(...)`](https://www.arduino.cc/en/Reference/WiFi101Begin)
* [`WiFi.end()`](https://www.arduino.cc/en/Reference/WiFi101End)
* [`WiFi.beginAP(...)`](https://www.arduino.cc/en/Reference/WiFi101BeginAP)
  * Supports open and WPA/WPA2 encryption types, no WEP support
* [`WiFi.disconnect()`](https://www.arduino.cc/en/Reference/WiFi101Disconnect)
* [`WiFi.config(...)`](https://www.arduino.cc/en/Reference/WiFi101Config)
* [`WiFi.setDNS(...)`](https://www.arduino.cc/en/Reference/WiFi101SetDns)
* [`WiFi.SSID(...)`](https://www.arduino.cc/en/Reference/WiFi101SSID)
* [`WiFi.BSSID(...)`](https://www.arduino.cc/en/Reference/WiFi101BSSID)
* [`WiFi.RSSI(...)`](https://www.arduino.cc/en/Reference/WiFi101RSSI)
* [`WiFi.encryptionType(...)`](https://www.arduino.cc/en/Reference/WiFi101EncryptionType)
* [`WiFi.scanNetworks()`](https://www.arduino.cc/en/Reference/WiFi101ScanNetworks)
* [`WiFi.ping(...)`](https://www.arduino.cc/en/Reference/WiFi101Ping)
* [`WiFi.macAddress(...)`](https://www.arduino.cc/en/Reference/WiFi101MACAddress)
* [`WiFi.lowPowerMode()`](https://www.arduino.cc/en/Reference/WiFi101LowPowerMode)
* [`WiFi.noLowPowerMode()`](https://www.arduino.cc/en/Reference/WiFi101NoLowPowerMode)
* [`WiFi.localIP()`](https://www.arduino.cc/en/Reference/WiFi101LocalIP)
* [`WiFi.subnetMask()`](https://www.arduino.cc/en/Reference/WiFi101SubnetMask)
* [`WiFi.gatewayIP()`](https://www.arduino.cc/en/Reference/WiFi101GatewayIP)
* [`WiFi.getTime()`](https://www.arduino.cc/en/Reference/WiFi101GetTime)

## `WiFiClient`

* [`WiFiClient()`](https://www.arduino.cc/en/Reference/WiFi101Client)
* [`wifiClient.connected()`](https://www.arduino.cc/en/Reference/WiFi101ClientConnected)
* [`wifiClient.connect(...)`](https://www.arduino.cc/en/Reference/WiFi101ClientConnect)
* [`wifiClient.write(...)`](https://www.arduino.cc/en/Reference/WiFi101ClientWrite)
* [`wifiClient.print(...)`](https://www.arduino.cc/en/Reference/WiFi101ClientPrint)
* [`wifiClient.println(...)`](https://www.arduino.cc/en/Reference/WiFi101ClientPrintln)
* [`wifiClient.available()`](https://www.arduino.cc/en/Reference/WiFi101ClientAvailable)
* [`wifiClient.read(...)`](https://www.arduino.cc/en/Reference/WiFi101ClientRead)
* [`wifiClient.flush()`](https://www.arduino.cc/en/Reference/WiFi101ClientFlush)
* [`wifiClient.stop()`](https://www.arduino.cc/en/Reference/WiFi101ClientStop)

## `WiFiServer`

* [`WiFiServer(...)`](https://www.arduino.cc/en/Reference/WiFi101Server)
* [`wifiServer.begin()`](https://www.arduino.cc/en/Reference/WiFi101ServerBegin)
* [`wifiServer.write(...)`](https://www.arduino.cc/en/Reference/WiFi101ServerWrite)
* [`wifiServer.print(...)`](https://www.arduino.cc/en/Reference/WiFi101ServerPrint)
* [`wifiServer.println(...)`](https://www.arduino.cc/en/Reference/WiFi101ServerPrintln)
* [`wifiServer.available()`](https://www.arduino.cc/en/Reference/WiFi101ServerAvailable)

## `WiFiUDP`

* [`WiFiUDP(...)`](https://www.arduino.cc/en/Reference/WiFi101UDPConstructor)
* [`wifiUdp.begin(...)`](https://www.arduino.cc/en/Reference/WiFi101UDPBegin)
* [`wifiUdp.beginPacket(...)`](https://www.arduino.cc/en/Reference/WiFi101UDPBeginPacket)
* [`wifiUdp.write(...)`](https://www.arduino.cc/en/Reference/WiFi101UDPWrite)
* [`wifiUdp.endPacket()`](https://www.arduino.cc/en/Reference/WiFi101UDPEndPacket)
* [`wifiUdp.available()`](https://www.arduino.cc/en/Reference/WiFi101UDPAvailable)
* [`wifiUdp.parsePacket()`](https://www.arduino.cc/en/Reference/WiFi101UDPParsePacket)
* [`wifiUdp.peek()`](https://www.arduino.cc/en/Reference/WiFi101UDPPeek)
* [`wifiUdp.read(...)`](https://www.arduino.cc/en/Reference/WiFi101UDPRead)
* [`wifiUdp.flush()`](https://www.arduino.cc/en/Reference/WiFi101UDPFlush)
* [`wifiUdp.stop()`](https://www.arduino.cc/en/Reference/WiFi101UDPStop)
* [`wifiUdp.remoteIP()`](https://www.arduino.cc/en/Reference/WiFi101UDPRemoteIP)
* [`wifiUdp.remotePort()`](https://www.arduino.cc/en/Reference/WiFi101UDPRemotePort)
