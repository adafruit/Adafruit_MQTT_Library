#ifndef _ADAFRUIT_MQTT_H_
#define _ADAFRUIT_MQTT_H_

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#define MQTT_PROTOCOL_LEVEL 3

#define MQTT_CTRL_CONNECT 0x1
#define MQTT_CTRL_CONNECTACK 0x2
#define MQTT_CTRL_PUBLISH 0x3

#define MQTT_QOS_1 0x1
#define MQTT_QOS_0 0x0

#define SERVERNAME_SIZE  25
#define USERNAME_SIZE  25
#define KEY_SIZE  41
#define FEEDNAME_SIZE  65

#define CONNECT_TIMEOUT_MS 3000
#define PUBLISH_TIMEOUT_MS 500

#define MQTT_CONN_USERNAMEFLAG 0x80
#define MQTT_CONN_PASSWORDFLAG 0x40
#define MQTT_CONN_WILLRETAIN   0x20
#define MQTT_CONN_WILLQOS   0x08
#define MQTT_CONN_WILLFLAG   0x04
#define MQTT_CONN_CLEANSESSION   0x02
#define MQTT_CONN_KEEPALIVE 15  // in seconds

#define MAXBUFFERSIZE 130

class Adafruit_MQTT {
 public:
  Adafruit_MQTT(char *server, uint16_t port, char *user, char *key);
  uint8_t connectPacket(uint8_t *packet);

  virtual boolean publish(char *topic, char *payload, uint8_t qos) {}
  uint8_t publishPacket(uint8_t *packet, char *topic, char *payload, uint8_t qos);

 protected:
  int8_t errno;
  char servername[SERVERNAME_SIZE];
  uint32_t serverip;
  int16_t portnum;
  char username[USERNAME_SIZE];
  char userkey[KEY_SIZE];

  uint8_t buffer[MAXBUFFERSIZE];
};

class Adafruit_MQTT_Publish {
 public:
  Adafruit_MQTT_Publish(Adafruit_MQTT& mqttserver, char *feed, uint8_t qos = 0);

  //bool publish(char *s);
  //bool publish(double f);
  bool publish(int32_t i);
  bool publish(uint32_t i);
 
private:
  Adafruit_MQTT *mqtt;
  char topic[FEEDNAME_SIZE];
  uint8_t qos;
  int8_t errno;
};

class Adafruit_MQTT_Subscibe {
 public:
  Adafruit_MQTT_Subscibe(Adafruit_MQTT *mqttserver, char *feedname);
  bool setCallback(void (*callback)(char *));

};


#endif /* header guard */
