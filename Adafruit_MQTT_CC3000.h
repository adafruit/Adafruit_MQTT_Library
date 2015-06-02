#ifndef _ADAFRUIT_MQTT_CC3000_H_
#define _ADAFRUIT_MQTT_CC3000_H_


#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_CC3000.h"
#include <Adafruit_CC3000.h>

class Adafruit_MQTT_CC3000 : public Adafruit_MQTT {
 public:
  Adafruit_MQTT_CC3000(Adafruit_CC3000 *cc3k, char *server, uint16_t port, char *cid, char *user, char *pass);
  int8_t connect(void);
  uint16_t readPacket(uint8_t *buffer, uint8_t maxlen, uint16_t timeout);
  int32_t close(void);

  boolean publish(char *topic, char *payload, uint8_t qos);
  boolean ping(void);

 private:
  Adafruit_CC3000 *cc3000;
  Adafruit_CC3000_Client mqttclient;
};


#endif
