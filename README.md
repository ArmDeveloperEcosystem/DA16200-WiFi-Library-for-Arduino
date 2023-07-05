# DA16200 Wi-Fi Library for Arduino

Wi-Fi enable your Arduino sketches using [Dialog's DA16200 Module](https://www.dialog-semiconductor.com/products/wi-fi/da16200-modules).

Includes support for the [SparkFun's "Qwiic WiFi Shield - DA16200"](https://www.sparkfun.com/products/18567). **Requires firmware version `3.1.2.0` or later.**

Currently compatible with `samd` and `mbed_nano` based Arduino boards, such as the [Arduino Zero](https://store-usa.arduino.cc/products/arduino-zero), [SparkFun RedBoard Turbo - SAMD21 Development Board](https://www.sparkfun.com/products/14812), and [Arduino Nano 33 BLE](https://store-usa.arduino.cc/collections/nano-family/products/arduino-nano-33-ble). Please open a pull request if you've succesfully used this library with another architecture.

## Usage

### Include library in Arduino sketch

`#include <DA16200_WiFi.h>`

### API

See [API.md](API.md) for more details. This library aims to be compatible with with the official Arduino [WiFi](https://www.arduino.cc/en/Reference/WiFi) and [WiFi101](https://www.arduino.cc/en/Reference/WiFi101) libraries where possible.

### Examples

See [`examples`](examples/) folder. Examples are based on the official Arduino [WiFi](https://www.arduino.cc/en/Reference/WiFi) and [WiFi101](https://www.arduino.cc/en/Reference/WiFi101) library examples.

### ⚠️ Known Limitations due to DA16200 AT command firmware ⚠️

 * No support for provisioning mode
 * Only one active TCP client (`WiFiClient`), TCP server (`WiFiServer`), UDP socket (`WiFiUDP`) at a time
 * No TLS socket support
 * No support for disconnecting individual clients connected to a TCP server (`WiFiServer`)
 * No support for UDP multicast sockets (`WiFiUDP`)
 * No flow control when receiving large amounts of data on sockets

## License

[LGPL 2.1](LICENSE)
