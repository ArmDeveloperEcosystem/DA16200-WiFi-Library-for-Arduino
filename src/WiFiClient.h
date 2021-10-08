#ifndef _WIFI_CLIENT_H_
#define _WIFI_CLIENT_H_

#include <Client.h>
#include <RingBuffer.h>

class WiFiClient : public Client {
  public:
    WiFiClient();
    virtual ~WiFiClient();

    virtual int connect(IPAddress ip, uint16_t port);
    virtual int connect(const char* host, uint16_t port);
    virtual size_t write(uint8_t);
    virtual size_t write(const uint8_t* buf, size_t size);
    virtual int available();
    virtual int read();
    virtual int read(uint8_t* buf, size_t size);
    virtual int peek();
    virtual void flush();
    virtual void stop();
    virtual uint8_t connected();
    virtual operator bool();

    using Print::write;

  protected:
    friend class WiFiClass;

    int receive(IPAddress ip, uint16_t port, Stream& s, int length);

    static WiFiClient* _inst;

  private:
    RingBufferN<8096> _rxBuffer;
};

#endif
