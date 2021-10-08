#include "WiFi.h"

#include "WiFiClient.h"

WiFiClient* WiFiClient::_inst = NULL;

WiFiClient::WiFiClient()
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
  if (_inst != NULL) {
    return 0;
  }

  char args[1 + 15 + 1 + 5 + 1 + 5 + 1];

  sprintf(args, "=%d.%d.%d.%d,%d,%d", ip[0], ip[1], ip[2], ip[3], port, 0);

  if (WiFi._modem.AT("+TRTC", args, 10000) != 0) {
    return 0;
  }

  _inst = this;
  _rxBuffer.clear();

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
  char args[1 + 4 + 5 + 1];

  if (size > 2048) {
    size = 2048;
  }

  sprintf(args, "1%d,0,0,", size);

  if (WiFi._modem.ESC("S", args, buf, size) != 0) {
    setWriteError();
    return 0;
  }

  return size;
}

int WiFiClient::available()
{
  if (!_rxBuffer.available()) {
    WiFi._modem.poll(20);
  }

  return _rxBuffer.available();
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
  int avail = available();
  if (size > avail) {
    size = avail;
  }

  for (int i = 0; i < size; i++) {
    *buf++ = _rxBuffer.read_char();
  }

  return size;
}

int WiFiClient::peek()
{
  if (available()) {
    return _rxBuffer.peek();
  }

  return -1;
}

void WiFiClient::flush()
{
}

void WiFiClient::stop()
{
  if (_inst == this) {
    WiFi._modem.AT("+TRTRM", "=1");

    _inst = NULL;
    _rxBuffer.clear();
  }
}

uint8_t WiFiClient::connected()
{
  return _rxBuffer.available() || (_inst == this);
}

WiFiClient::operator bool()
{
  return (_inst == this);
}

int WiFiClient::receive(IPAddress ip, uint16_t port, Stream& s, int length)
{
  int read = 0;

  while (read < length) {
    if (_rxBuffer.isFull()) {
      // TODO: ensure no RX buffer overflow ... close sock if overflow?
      Serial.println("*** WiFiClient::receive => _rxBuffer.isFull() ***");
      while (1);
    }

    if (s.available()) {
      _rxBuffer.store_char(s.read());
      read++;
    }
  }

  return read;
}
