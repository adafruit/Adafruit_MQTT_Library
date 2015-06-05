#include <Adafruit_SleepyDog.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_CC3000.h"


bool Adafruit_MQTT_CC3000::connectServer(void) {
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

bool Adafruit_MQTT_CC3000::disconnect(void) {
  return (mqttclient.close() == 0); 
}

uint16_t Adafruit_MQTT_CC3000::readPacket(uint8_t *buffer, uint8_t maxlen, 
                                          int16_t timeout, 
                                          bool checkForValidPubPacket) {
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

bool Adafruit_MQTT_CC3000::sendPacket(uint8_t *buffer, uint8_t len) {
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
