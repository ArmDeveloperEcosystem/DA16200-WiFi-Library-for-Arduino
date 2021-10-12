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
  _cid = 1;
  WiFi._socketBuffer.begin(_cid);
  WiFi._socketBuffer.clear(_cid);
  WiFi._socketBuffer.connect(_cid);

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

  sprintf(args, "%d%d,0,0,", _cid, size);

  if (WiFi._modem.ESC("S", args, buf, size) != 0) {
    setWriteError();
    return 0;
  }

  return size;
}

int WiFiClient::available()
{
  WiFi._modem.poll(0);

  return WiFi._socketBuffer.available(_cid);
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

  return WiFi._socketBuffer.read(_cid, buf, size);
}

int WiFiClient::peek()
{
  if (available()) {
    return  WiFi._socketBuffer.peek(_cid);
  }

  return -1;
}

void WiFiClient::flush()
{
}

void WiFiClient::stop()
{
  if (_inst == this) {
    if (WiFi._socketBuffer.connected(_cid)) {
      WiFi._modem.AT("+TRTRM", "=1");
    }

    _inst = NULL;
    WiFi._socketBuffer.clear(_cid);
    WiFi._socketBuffer.disconnect(_cid);
  }
}

uint8_t WiFiClient::connected()
{
  WiFi._modem.poll(0);

  return WiFi._socketBuffer.available(_cid) || WiFi._socketBuffer.connected(_cid);
}

WiFiClient::operator bool()
{
  return (_inst == this);
}
