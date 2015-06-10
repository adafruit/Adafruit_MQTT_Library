#ifndef _ADAFRUIT_MQTT_FONA_H_
#define _ADAFRUIT_MQTT_FONA_H_

#include <Adafruit_FONA.h>
#include "Adafruit_MQTT.h"

#define MQTT_FONA_INTERAVAILDELAY 100
#define MQTT_FONA_QUERYDELAY 500

class Adafruit_MQTT_FONA : public Adafruit_MQTT {
 public:
  Adafruit_MQTT_FONA(Adafruit_FONA *f, const char *server, uint16_t port, 
                       const char *cid, const char *user, const char *pass):
    Adafruit_MQTT(server, port, cid, user, pass),
    fona(f)
  {}

  bool connectServer() {
    char server[40];
    strncpy_P(server, servername, 40);
    Watchdog.reset();

    // connect to server
    DEBUG_PRINTLN(F("Connecting to TCP"));
    return fona->TCPconnect(server, portnum);
  }

  bool disconnect() {
    return fona->TCPclose();
  }

  uint16_t readPacket(uint8_t *buffer, uint8_t maxlen, int16_t timeout, 
                      bool checkForValidPubPacket = false) {
    uint8_t *buffp = buffer;
    DEBUG_PRINTLN(F("Reading a packet.."));

    if (!fona->TCPconnected()) return 0;


    /* Read data until either the connection is closed, or the idle timeout is reached. */
    uint16_t len = 0;
    int16_t t = timeout;
    uint16_t avail;

    while (fona->TCPconnected() && (timeout >= 0)) {
      DEBUG_PRINT('.');
      while (avail = fona->TCPavailable()) {
        DEBUG_PRINT('!');
        
        if (len + avail > maxlen) {
    // oof we cant read more of the available data in this buffer
    return len;
        }

        // try to read the data into the end of the pointer
        if (! fona->TCPread(buffp, avail)) return len;
        
        // read it! advance pointer
        buffp += avail;
        len += avail;
        timeout = t;  // reset the timeout

        //DEBUG_PRINTLN((uint8_t)c, HEX);

        if (len == maxlen) {  // we read all we want, bail
          DEBUG_PRINT(F("Read packet:\t"));
          DEBUG_PRINTBUFFER(buffer, len);
    return len;
        }

        // special case where we just one one publication packet at a time
        if (checkForValidPubPacket) {
          if ((buffer[0] == (MQTT_CTRL_PUBLISH << 4)) && (buffer[1] == len-2)) {
            // oooh a valid publish packet!
            DEBUG_PRINT(F("Read PUBLISH packet:\t"));
            DEBUG_PRINTBUFFER(buffer, len);
            return len;
          }
        }
        
      }
      Watchdog.reset();
      timeout -= MQTT_FONA_INTERAVAILDELAY;
      timeout -= MQTT_FONA_QUERYDELAY; // this is how long it takes to query the FONA for avail()
      delay(MQTT_FONA_INTERAVAILDELAY);
    }
    
    return len;
  }

  bool sendPacket(uint8_t *buffer, uint8_t len) {
    DEBUG_PRINTLN(F("Writing packet"));
    if (fona->TCPconnected()) {
      boolean ret = fona->TCPsend((char *)buffer, len);
      //DEBUG_PRINT(F("sendPacket returned: ")); DEBUG_PRINTLN(ret);
      if (!ret) {
        DEBUG_PRINTLN("Failed to send packet.")
        return false;
      }
    } else {
      DEBUG_PRINTLN(F("Connection failed!"));
      return false;
    }
    return true;    
  }
 
 private:
  uint32_t serverip;
  Adafruit_FONA *fona;
};

#endif
