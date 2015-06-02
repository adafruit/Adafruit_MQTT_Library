#include "Adafruit_MQTT.h"


Adafruit_MQTT::Adafruit_MQTT(char *server, uint16_t port, char *cid, char *user, char *pass) {
  strncpy(servername, server, SERVERNAME_SIZE);
  servername[SERVERNAME_SIZE-1] = 0;
  portnum = port;
  serverip = 0;

  strncpy(clientid, cid, CLIENTID_SIZE);
  clientid[CLIENTID_SIZE-1] = 0;

  strncpy(username, user, USERNAME_SIZE);
  username[USERNAME_SIZE-1] = 0;

  strncpy(password, pass, PASSWORD_SIZE);
  password[PASSWORD_SIZE-1] = 0;

  errno = 0;
}

uint8_t Adafruit_MQTT::pingPacket(uint8_t *packet) {
  packet[0] = MQTT_CTRL_PINGREQ << 4;
  packet[1] = 0;
  return 2;
}

static uint8_t *stringprint(uint8_t *p, char *s) {
  uint16_t len = strlen(s);
  p[0] = len >> 8; p++;
  p[0] = len & 0xFF; p++;
  memcpy(p, s, len);
  return p+len;
}

// http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718028
uint8_t Adafruit_MQTT::connectPacket(uint8_t *packet) {
  uint8_t *p = packet;
  uint16_t len;

  // fixed header, connection messsage no flags
  p[0] = (MQTT_CTRL_CONNECT << 4) | 0x0;
  p+=2;
  // fill in packet[1] last

  p = stringprint(p, "MQIsdp");

  p[0] = MQTT_PROTOCOL_LEVEL;
  p++;

  p[0] = MQTT_CONN_CLEANSESSION;
  if (username[0] != 0) 
    p[0] |= MQTT_CONN_USERNAMEFLAG;
  if (password[0] != 0) 
    p[0] |= MQTT_CONN_PASSWORDFLAG;
  p++;
  // TODO: add WILL support?

  p[0] = MQTT_CONN_KEEPALIVE >> 8;
  p++;
  p[0] = MQTT_CONN_KEEPALIVE & 0xFF;
  p++;

  p = stringprint(p, clientid);

  if (username[0] != 0) {
    p = stringprint(p, username);
  }
  if (password[0] != 0) {
    p = stringprint(p, password);
  }

  len = p - packet;

  packet[1] = len-2;  // don't include the 2 bytes of fixed header data
  
  return len;
}

uint8_t Adafruit_MQTT::publishPacket(uint8_t *packet, char *topic, char *data, uint8_t qos) {
  uint8_t *p = packet;
  uint16_t len;

  p[0] = MQTT_CTRL_PUBLISH << 4 | qos << 1;
  // fill in packet[1] last
  p+=2;

  p = stringprint(p, topic);

  memcpy(p, data, strlen(data));
  p+=strlen(data);
  len = p - packet;
  packet[1] = len-2; // don't include the 2 bytes of fixed header data
  return len;
}


Adafruit_MQTT_Publish::Adafruit_MQTT_Publish(Adafruit_MQTT &mqttserver, char *feed, uint8_t q) {
  mqtt = &mqttserver;
  strncpy(topic, feed, FEEDNAME_SIZE);
  topic[FEEDNAME_SIZE-1] = 0; 
  qos = q;

  errno = 0;
}


bool Adafruit_MQTT_Publish::publish(int32_t i) {
  char payload[18];
  itoa(i, payload, 10);
  return mqtt->publish(topic, payload, qos);
}

bool Adafruit_MQTT_Publish::publish(uint32_t i) {
  char payload[18];
  itoa(i, payload, 10);
  return mqtt->publish(topic, payload, qos);
}
