#ifndef _ADAFRUIT_MQTT_CC3000_H_
#define _ADAFRUIT_MQTT_CC3000_H_

#include <Adafruit_CC3000.h>
#include "Adafruit_MQTT.h"

// delay in ms between calls of available()
#define MQTT_CC3000_INTERAVAILDELAY 10  


class Adafruit_MQTT_CC3000 : public Adafruit_MQTT {
 public:
  Adafruit_MQTT_CC3000(Adafruit_CC3000 *cc3k, const char *server, uint16_t port, 
                       const char *cid, const char *user, const char *pass):
    Adafruit_MQTT(server, port, cid, user, pass),
    cc3000(cc3k)
  {}

  bool connectServer();
  bool disconnect();
  uint16_t readPacket(uint8_t *buffer, uint8_t maxlen, int16_t timeout, 
                      bool checkForValidPubPacket = false);
  bool sendPacket(uint8_t *buffer, uint8_t len);
 
 private:
  uint32_t serverip;
  Adafruit_CC3000 *cc3000;
  Adafruit_CC3000_Client mqttclient;
};


#endif
