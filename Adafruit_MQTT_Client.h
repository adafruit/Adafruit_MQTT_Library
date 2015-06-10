#ifndef _ADAFRUIT_MQTT_CLIENT_H_
#define _ADAFRUIT_MQTT_CLIENT_H_

#include "Client.h"
#include "Adafruit_MQTT.h"

// How long to delay waiting for new data to be available in readPacket.
#define MQTT_CLIENT_READINTERVAL_MS 10  

// MQTT client implementation for a generic Arduino Client interface.  Can work
// with almost all Arduino network hardware like ethernet shield, wifi shield,
// and even other platforms like ESP8266.

class Adafruit_MQTT_Client : public Adafruit_MQTT {
 public:
  Adafruit_MQTT_Client(Client *client, const char *server, uint16_t port,
                       const char *cid, const char *user,
                       const char *pass):
    Adafruit_MQTT(server, port, cid, user, pass),
    client(client)
  {}

  bool connectServer();
  bool disconnect();
  uint16_t readPacket(uint8_t *buffer, uint8_t maxlen, int16_t timeout, 
                      bool checkForValidPubPacket = false);
  bool sendPacket(uint8_t *buffer, uint8_t len);
 
 private:
  Client* client;
};


#endif
