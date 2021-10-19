#ifndef _WIFI_MODEM_H_
#define _WIFI_MODEM_H_

#include <Arduino.h>

class WiFiModem : public Stream {
  public:
    WiFiModem(HardwareSerial& serial);
    virtual ~WiFiModem();

    void begin(unsigned long baudrate);
    void end();

    void onExtendedResponse(void (*handler)(void*, const char*, Stream&), void* context);

    int AT(const char* command = "", const char* args = NULL, int timeout = 1000);
    int ESC(const char* sequence, const char* args, const uint8_t* buffer, int length, int timeout = 1000);

    void poll(unsigned long timeout);

    // from Stream
    virtual int available();
    virtual int read();
    virtual int peek();

    // from Print
    virtual size_t write(uint8_t);
    virtual size_t write(const uint8_t* buffer, size_t size);
    virtual int availableForWrite();
    virtual void flush();

    void debug(Print& p);
    void noDebug();

  private:
    int waitForResponse(int timeout);

  private:
    HardwareSerial* _serial;

    Print* _debug;

    struct {
      void(*handler)(void*, const char*, Stream&);
      void* context;
    } _extendedResponse;
};

#endif
