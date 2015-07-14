// The MIT License (MIT)
//
// Copyright (c) 2015 Adafruit Industries
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#ifndef _ADAFRUIT_MQTT_CC3000_H_
#define _ADAFRUIT_MQTT_CC3000_H_

#include <Adafruit_SleepyDog.h>
#include <Adafruit_CC3000.h>
#include "Adafruit_MQTT.h"


// delay in ms between calls of available()
#define MQTT_CC3000_INTERAVAILDELAY 10


// CC3000-specific version of the Adafruit_MQTT class.
// Note that this is defined as a header-only class to prevent issues with using
// the library on non-CC3000 platforms (since Arduino will include all .cpp files
// in the compilation of the library).
class Adafruit_MQTT_CC3000 : public Adafruit_MQTT {
 public:
  Adafruit_MQTT_CC3000(Adafruit_CC3000 *cc3k, const char *server, uint16_t port,
                       const char *cid, const char *user, const char *pass):
    Adafruit_MQTT(server, port, cid, user, pass),
    cc3000(cc3k)
  {}

  bool connectServer() {
    uint32_t ip = 0;

    Watchdog.reset();

    // look up IP address
    if (serverip == 0) {
      // Try looking up the website's IP address using CC3K's built in getHostByName
      strcpy_P((char *)buffer, servername);
      Serial.print((char *)buffer); Serial.print(F(" -> "));
      uint8_t dnsretries = 5;

      Watchdog.reset();
      while (ip == 0) {
        if (! cc3000->getHostByName((char *)buffer, &ip)) {
          Serial.println(F("Couldn't resolve!"));
          dnsretries--;
          Watchdog.reset();
        }
        //Serial.println("OK"); Serial.println(ip, HEX);
        if (!dnsretries) return false;
        delay(500);
      }

      serverip = ip;
      cc3000->printIPdotsRev(serverip);
      Serial.println();
    }

    Watchdog.reset();

    // connect to server
    DEBUG_PRINTLN(F("Connecting to TCP"));
    mqttclient = cc3000->connectTCP(serverip, portnum);

    return mqttclient.connected();
  }

  bool disconnect() {
    if (connected()) {
      return (mqttclient.close() == 0);
    }
    else {
      return true;
    }
  }

  bool connected() {
    return mqttclient.connected();
  }

  uint16_t readPacket(uint8_t *buffer, uint8_t maxlen, int16_t timeout,
                      bool checkForValidPubPacket = false) {
    /* Read data until either the connection is closed, or the idle timeout is reached. */
    uint16_t len = 0;
    int16_t t = timeout;

    while (mqttclient.connected() && (timeout >= 0)) {
      //DEBUG_PRINT('.');
      while (mqttclient.available()) {
        //DEBUG_PRINT('!');
        char c = mqttclient.read();
        timeout = t;  // reset the timeout
        buffer[len] = c;
        //DEBUG_PRINTLN((uint8_t)c, HEX);
        len++;
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
      timeout -= MQTT_CC3000_INTERAVAILDELAY;
      delay(MQTT_CC3000_INTERAVAILDELAY);
    }
    return len;
  }

  bool sendPacket(uint8_t *buffer, uint8_t len) {
    if (mqttclient.connected()) {
      uint16_t ret = mqttclient.write(buffer, len);
      DEBUG_PRINT(F("sendPacket returned: ")); DEBUG_PRINTLN(ret);
      if (ret != len) {
        DEBUG_PRINTLN("Failed to send complete packet.")
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
  Adafruit_CC3000 *cc3000;
  Adafruit_CC3000_Client mqttclient;
};


#endif
