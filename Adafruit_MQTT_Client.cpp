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
#include "Adafruit_MQTT_Client.h"


bool Adafruit_MQTT_Client::connectServer() {
  // Grab server name from flash and copy to buffer for name resolution.
  memset(buffer, 0, sizeof(buffer));
  strcpy_P((char *)buffer, servername);
  DEBUG_PRINT(F("Connecting to: ")); DEBUG_PRINTLN((char *)buffer);
  // Connect and check for success (0 result).
  int r = client->connect((char *)buffer, portnum);
  DEBUG_PRINT(F("Connect result: ")); DEBUG_PRINTLN(r);
  return r != 0;
}

bool Adafruit_MQTT_Client::disconnect() {
  // Stop connection if connected and return success (stop has no indication of
  // failure).
  if (client->connected()) {
    client->stop();
  }
  return true;
}

bool Adafruit_MQTT_Client::connected() {
  // Return true if connected, false if not connected.
  return client->connected();
}

uint16_t Adafruit_MQTT_Client::readPacket(uint8_t *buffer, uint8_t maxlen,
                                          int16_t timeout,
                                          bool checkForValidPubPacket) {
  /* Read data until either the connection is closed, or the idle timeout is reached. */
  uint16_t len = 0;
  int16_t t = timeout;

  while (client->connected() && (timeout >= 0)) {
    //DEBUG_PRINT('.');
    while (client->available()) {
      //DEBUG_PRINT('!');
      char c = client->read();
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
    timeout -= MQTT_CLIENT_READINTERVAL_MS;
    delay(MQTT_CLIENT_READINTERVAL_MS);
  }
  return len;
}

bool Adafruit_MQTT_Client::sendPacket(uint8_t *buffer, uint8_t len) {
  if (client->connected()) {
    uint16_t ret = client->write(buffer, len);
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
