#ifndef _ADAFRUIT_MQTT_H_
#define _ADAFRUIT_MQTT_H_

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#define MQTT_PROTOCOL_LEVEL 3

#define MQTT_CTRL_CONNECT 0x01
#define MQTT_CTRL_CONNECTACK 0x02
#define MQTT_CTRL_PUBLISH 0x03
#define MQTT_CTRL_SUBSCRIBE 0x08
#define MQTT_CTRL_SUBACK 0x09
#define MQTT_CTRL_PINGREQ 0x0C
#define MQTT_CTRL_PINGRESP 0x0D

#define MQTT_QOS_1 0x1
#define MQTT_QOS_0 0x0

#define SERVERNAME_SIZE  25

#define PASSWORD_SIZE  25
#define USERNAME_SIZE  41
#define CLIENTID_SIZE 23

#define FEEDNAME_SIZE  40


#define CONNECT_TIMEOUT_MS 3000
#define PUBLISH_TIMEOUT_MS 500
#define PING_TIMEOUT_MS 500

#define MQTT_CONN_USERNAMEFLAG 0x80
#define MQTT_CONN_PASSWORDFLAG 0x40
#define MQTT_CONN_WILLRETAIN   0x20
#define MQTT_CONN_WILLQOS   0x08
#define MQTT_CONN_WILLFLAG   0x04
#define MQTT_CONN_CLEANSESSION   0x02
#define MQTT_CONN_KEEPALIVE 15  // in seconds

#define MAXBUFFERSIZE (85)
#define MAXSUBSCRIPTIONS 5
#define SUBSCRIPTIONDATALEN 20


//#define DEBUG_MQTT_CONNECT
//#define DEBUG_MQTT_SUBSCRIBE
//#define DEBUG_MQTT_READSUB
//#define DEBUG_MQTT_PUBLISH
//#define DEBUG_MQTT_PACKETREAD

class Adafruit_MQTT_Subscribe;  // forward decl

class Adafruit_MQTT {
 public:
  Adafruit_MQTT(const char *server, uint16_t port, const PROGMEM char *cid, const PROGMEM char *user, const PROGMEM char *pass);

  uint8_t connectPacket(uint8_t *packet);

  virtual boolean publish(const char *topic, char *payload, uint8_t qos) {}
  uint8_t publishPacket(uint8_t *packet, const char *topic, char *payload, uint8_t qos);

  virtual boolean ping(uint8_t t) {}
  uint8_t pingPacket(uint8_t *packet);

  virtual boolean subscribe(Adafruit_MQTT_Subscribe *sub) {}
  uint8_t subscribePacket(uint8_t *packet, const char *topic, uint8_t qos);

  virtual Adafruit_MQTT_Subscribe *readSubscription(int16_t timeout = 0) {};

 protected:
  int8_t errno;
  const char *servername;
  uint32_t serverip;
  int16_t portnum;
  const char *clientid;
  const char *username;
  const char *password;

  Adafruit_MQTT_Subscribe *subscriptions[MAXSUBSCRIPTIONS];
  uint8_t buffer[MAXBUFFERSIZE];
};

class Adafruit_MQTT_Publish {
 public:
  Adafruit_MQTT_Publish(Adafruit_MQTT *mqttserver, const char *feed, uint8_t qos = 0);

  bool publish(char *s);
  //bool publish(double f);
  bool publish(int32_t i);
  bool publish(uint32_t i);
 
private:
  Adafruit_MQTT *mqtt;
  const char *topic;
  uint8_t qos;
};

class Adafruit_MQTT_Subscribe {
 public:
  Adafruit_MQTT_Subscribe(Adafruit_MQTT *mqttserver, const char *feedname, uint8_t q=0);

  bool setCallback(void (*callback)(char *));

  const char *topic;
  uint8_t qos;

  uint8_t * lastread[SUBSCRIPTIONDATALEN];
 private:
  Adafruit_MQTT *mqtt;
};


#endif /* header guard */
