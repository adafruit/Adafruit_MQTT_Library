#include "Adafruit_MQTT.h"


Adafruit_MQTT::Adafruit_MQTT(char *server, uint16_t port, char *user, char *key) {
  strncpy(servername, server, SERVERNAME_SIZE);
  servername[SERVERNAME_SIZE-1] = 0;
  portnum = port;
  serverip = 0;
  strncpy(username, user, USERNAME_SIZE);
  username[USERNAME_SIZE-1] = 0;
  strncpy(userkey, key, KEY_SIZE);
  userkey[KEY_SIZE-1] = 0; 

  errno = 0;
}

// http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718028
uint8_t Adafruit_MQTT::connectPacket(uint8_t *packet) {
  uint8_t *p = packet;

  // fixed header, connection messsage no flags
  p[0] = (MQTT_CTRL_CONNECT << 4) | 0x0;
  p+=2;
  // fill in packet[1] last

  p[0] = 0;
  p[1] = 6; // (strlen(MQIdsp)
  p+=2;
  memcpy(p,"MQIdsp", 6);
  p+=6;

  p[0] = MQTT_PROTOCOL_LEVEL;
  p++;

  p[0] = MQTT_CONN_CLEANSESSION;
  if (username[0] != 0) 
    p[0] |= MQTT_CONN_USERNAMEFLAG;
  if (userkey[0] != 0) 
    p[0] |= MQTT_CONN_PASSWORDFLAG;
  p++;
  // TODO: add WILL support?

  p[0] = MQTT_CONN_KEEPALIVE >> 8;
  p++;
  p[0] = MQTT_CONN_KEEPALIVE & 0xFF;
  p++;

  if (username[0] != 0) {
    uint16_t len = strlen(username);
    p[0] = len >> 8; p++;
    p[0] = len & 0xFF; p++;
    memcpy(p, username, len);
    p+=len;
  }
  if (userkey[0] != 0) {
    uint16_t len = strlen(userkey);
    p[0] = len >> 8; p++;
    p[0] = len & 0xFF; p++;
    memcpy(p, userkey, len);
    p+=len;
  }

  uint8_t totallen = p - packet;

  // add two empty bytes at the end (?)
  p[0] = 0;
  p[1] = 0;
  p+=2;


  packet[1] = totallen;
  

  return totallen+2;
}


Adafruit_MQTT_Publish::Adafruit_MQTT_Publish(Adafruit_MQTT *mqttserver, char *feed) {
  mqtt = mqttserver;
  strncpy(feedname, feed, FEEDNAME_SIZE);
  feedname[FEEDNAME_SIZE-1] = 0; 

  errno = 0;
}


