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
#include "Adafruit_MQTT.h"


void printBuffer(uint8_t *buffer, uint8_t len) {
  for (uint8_t i=0; i<len; i++) {
    if (isprint(buffer[i]))
      DEBUG_PRINTER.write(buffer[i]);
    else  
      DEBUG_PRINTER.print(" ");
    DEBUG_PRINTER.print(F(" [0x"));
    if (buffer[i] < 0x10)
      DEBUG_PRINTER.print("0");
    DEBUG_PRINTER.print(buffer[i],HEX);
    DEBUG_PRINTER.print("], ");
    if (i % 8 == 7) DEBUG_PRINTER.println();
  }
  DEBUG_PRINTER.println();
}

/* Not used now, but might be useful in the future
static uint8_t *stringprint(uint8_t *p, char *s) {
  uint16_t len = strlen(s);
  p[0] = len >> 8; p++;
  p[0] = len & 0xFF; p++;
  memcpy(p, s, len);
  return p+len;
}
*/

static uint8_t *stringprint_P(uint8_t *p, const char *s, uint16_t maxlen=0) {
  // If maxlen is specified (has a non-zero value) then use it as the maximum
  // length of the source string to write to the buffer.  Otherwise write
  // the entire source string.
  uint16_t len = strlen_P(s);
  if (maxlen > 0 && len > maxlen) {
    len = maxlen;
  }
  /*
  for (uint8_t i=0; i<len; i++) {
    Serial.write(pgm_read_byte(s+i));
  }
  */
  p[0] = len >> 8; p++;
  p[0] = len & 0xFF; p++;
  strncpy_P((char *)p, s, len);
  return p+len;
}


// Adafruit_MQTT Definition ////////////////////////////////////////////////////

Adafruit_MQTT::Adafruit_MQTT(const char *server, uint16_t port, const char *cid,
                             const char *user, const char *pass) {
  servername = server;
  portnum = port;
  clientid = cid;
  username = user;
  password = pass;

  for (uint8_t i=0; i<MAXSUBSCRIPTIONS; i++) {
    subscriptions[i] = 0;
  }
}

Adafruit_MQTT::Adafruit_MQTT(const __FlashStringHelper *server, uint16_t port, const __FlashStringHelper *cid,
                             const __FlashStringHelper *user, const __FlashStringHelper *pass) {
  servername = (const char *)server;
  portnum = port;
  clientid = (const char *)cid;
  username = (const char *)user;
  password = (const char *)pass;

  for (uint8_t i=0; i<MAXSUBSCRIPTIONS; i++) {
    subscriptions[i] = 0;
  }
}


/*
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
}
*/

int8_t Adafruit_MQTT::connect() {
  // Connect to the server.
  if (!connectServer())
    return -1;
  
  // Construct and send connect packet.
  uint8_t len = connectPacket(buffer);
  if (!sendPacket(buffer, len))
    return -1;

  // Read connect response packet and verify it  
  len = readPacket(buffer, 4, CONNECT_TIMEOUT_MS);
  if (len != 4)
    return -1;
  if ((buffer[0] != (MQTT_CTRL_CONNECTACK << 4)) || (buffer[1] != 2))
    return -1;
  if (buffer[3] != 0)
    return buffer[3];

  // Setup subscriptions once connected.
  for (uint8_t i=0; i<MAXSUBSCRIPTIONS; i++) {
    // Ignore subscriptions that aren't defined.
    if (subscriptions[i] == 0) continue;

    // Construct and send subscription packet.
    uint8_t len = subscribePacket(buffer, subscriptions[i]->topic, subscriptions[i]->qos);
    if (!sendPacket(buffer, len))
      return -1;
    
    // Get SUBACK
    len = readPacket(buffer, 5, CONNECT_TIMEOUT_MS);
    DEBUG_PRINT(F("SUBACK:\t"));
    DEBUG_PRINTBUFFER(buffer, len);
    if ((len != 5) || (buffer[0] != (MQTT_CTRL_SUBACK << 4))) {
      return 6;  // failure to subscribe
    }
  }

  return 0;
}

const __FlashStringHelper* Adafruit_MQTT::connectErrorString(int8_t code) {
   switch (code) {
      case 1: return F("Wrong protocol");
      case 2: return F("ID rejected");
      case 3: return F("Server unavail");
      case 4: return F("Bad user/pass");
      case 5: return F("Not authed");
      case 6: return F("Failed to subscribe");
      case -1: return F("Connection failed");
      default: return F("Unknown error");
   }
}

bool Adafruit_MQTT::publish(const char *topic, const char *data, uint8_t qos) {
  // Construct and send publish packet.
  uint8_t len = publishPacket(buffer, topic, data, qos);
  if (!sendPacket(buffer, len))
    return false;
  
  // If QOS level is high enough verify the response packet.
  if (qos > 0) {
    len = readPacket(buffer, 4, PUBLISH_TIMEOUT_MS);
    DEBUG_PRINT(F("Publish QOS1+ reply:\t"));
    DEBUG_PRINTBUFFER(buffer, len);
    //TODO: Verify response packet?
  }

  return true;
}

bool Adafruit_MQTT::subscribe(Adafruit_MQTT_Subscribe *sub) {
  uint8_t i;
  // see if we are already subscribed
  for (i=0; i<MAXSUBSCRIPTIONS; i++) {
    if (subscriptions[i] == sub) {
      DEBUG_PRINTLN(F("Already subscribed"));
      return true;
    }
  }
  if (i==MAXSUBSCRIPTIONS) { // add to subscriptionlist
    for (i=0; i<MAXSUBSCRIPTIONS; i++) {
      if (subscriptions[i] == 0) {
        DEBUG_PRINT(F("Added sub ")); DEBUG_PRINTLN(i);
        subscriptions[i] = sub;
        return true;
      }
    }
  }

  DEBUG_PRINTLN(F("no more subscription space :("));
  return false;
}

Adafruit_MQTT_Subscribe *Adafruit_MQTT::readSubscription(int16_t timeout) {
  uint8_t i, topiclen, datalen;

  // Check if data is available to read.
  uint16_t len = readPacket(buffer, MAXBUFFERSIZE, timeout, true); // return one full packet
  if (!len)
    return NULL;  // No data available, just quit.
  DEBUG_PRINTBUFFER(buffer, len);

  // Parse out length of packet.
  topiclen = buffer[3];
  DEBUG_PRINT(F("Looking for subscription len ")); DEBUG_PRINTLN(topiclen);

  // Find subscription associated with this packet.
  for (i=0; i<MAXSUBSCRIPTIONS; i++) {
    if (subscriptions[i]) {
      // Skip this subscription if its name length isn't the same as the
      // received topic name.
      if (strlen_P(subscriptions[i]->topic) != topiclen)
        continue;
      // Stop if the subscription topic matches the received topic. Be careful
      // to make comparison case insensitive.
      if (strncasecmp_P((char*)buffer+4, subscriptions[i]->topic, topiclen) == 0) {
        DEBUG_PRINT(F("Found sub #")); DEBUG_PRINTLN(i);
        break;        
      }
    }
  }
  if (i==MAXSUBSCRIPTIONS) return NULL; // matching sub not found ???

  // zero out the old data
  memset(subscriptions[i]->lastread, 0, SUBSCRIPTIONDATALEN);

  datalen = len - topiclen - 4;
  if (datalen > SUBSCRIPTIONDATALEN) {
    datalen = SUBSCRIPTIONDATALEN-1; // cut it off
  }
  // extract out just the data, into the subscription object itself
  memcpy(subscriptions[i]->lastread, buffer+4+topiclen, datalen);
  subscriptions[i]->datalen = datalen;
  DEBUG_PRINT(F("Data len: ")); DEBUG_PRINTLN(datalen);
  DEBUG_PRINT(F("Data: ")); DEBUG_PRINTLN((char *)subscriptions[i]->lastread);

  // return the valid matching subscription
  return subscriptions[i];
}

bool Adafruit_MQTT::ping(uint8_t times) {
  while (times) {
    // Construct and send ping packet.
    uint8_t len = pingPacket(buffer);
    if (!sendPacket(buffer, len))
      return false;
    
    // Process ping reply.
    len = readPacket(buffer, 2, PING_TIMEOUT_MS);
    if (buffer[0] == (MQTT_CTRL_PINGRESP << 4))
      return true;  
  }
  return false;
}

// Packet Generation Functions /////////////////////////////////////////////////

// The current MQTT spec is 3.1.1 and available here:
//   http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718028
// However this connect packet and code follows the MQTT 3.1 spec here (some
// small differences in the protocol):
//   http://public.dhe.ibm.com/software/dw/webservices/ws-mqtt/mqtt-v3r1.html#connect
uint8_t Adafruit_MQTT::connectPacket(uint8_t *packet) {
  uint8_t *p = packet;
  uint16_t len;

  // fixed header, connection messsage no flags
  p[0] = (MQTT_CTRL_CONNECT << 4) | 0x0;
  p+=2;
  // fill in packet[1] last

  p = stringprint_P(p, PSTR("MQIsdp"));

  p[0] = MQTT_PROTOCOL_LEVEL;
  p++;

  p[0] = MQTT_CONN_CLEANSESSION;
  if (pgm_read_byte(username) != 0)
    p[0] |= MQTT_CONN_USERNAMEFLAG;
  if (pgm_read_byte(password) != 0)
    p[0] |= MQTT_CONN_PASSWORDFLAG;
  p++;
  // TODO: add WILL support?

  p[0] = MQTT_CONN_KEEPALIVE >> 8;
  p++;
  p[0] = MQTT_CONN_KEEPALIVE & 0xFF;
  p++;

  p = stringprint_P(p, clientid, 23);  // Limit client ID to first 23 characters.

  if (pgm_read_byte(username) != 0) {
    p = stringprint_P(p, username);
  }
  if (pgm_read_byte(password) != 0) {
    p = stringprint_P(p, password);
  }

  len = p - packet;

  packet[1] = len-2;  // don't include the 2 bytes of fixed header data
  DEBUG_PRINTLN(F("MQTT connect packet:"));
  DEBUG_PRINTBUFFER(buffer, len);
  return len;
}

uint8_t Adafruit_MQTT::publishPacket(uint8_t *packet, const char *topic, 
                                     const char *data, uint8_t qos) {
  uint8_t *p = packet;
  uint16_t len;

  p[0] = MQTT_CTRL_PUBLISH << 4 | qos << 1;
  // fill in packet[1] last
  p+=2;

  p = stringprint_P(p, topic);

  memcpy(p, data, strlen(data));
  p+=strlen(data);
  len = p - packet;
  packet[1] = len-2; // don't include the 2 bytes of fixed header data
  DEBUG_PRINTLN(F("MQTT publish packet:"));
  DEBUG_PRINTBUFFER(buffer, len);
  return len;
}

uint8_t Adafruit_MQTT::subscribePacket(uint8_t *packet, const char *topic, 
                                       uint8_t qos) {
  uint8_t *p = packet;
  uint16_t len;

  p[0] = MQTT_CTRL_SUBSCRIBE << 4 | MQTT_QOS_1 << 1;
  // fill in packet[1] last
  p+=2;

  // put in a message id,
  p[0] = 0xAD;
  p[1] = 0xAF;
  p+=2;

  p = stringprint_P(p, topic);

  p[0] = qos;
  p++;

  len = p - packet;
  packet[1] = len-2; // don't include the 2 bytes of fixed header data
  DEBUG_PRINTLN(F("MQTT subscription packet:"));
  DEBUG_PRINTBUFFER(buffer, len);
  return len;
}

uint8_t Adafruit_MQTT::pingPacket(uint8_t *packet) {
  packet[0] = MQTT_CTRL_PINGREQ << 4;
  packet[1] = 0;
  DEBUG_PRINTLN(F("MQTT ping packet:"));
  DEBUG_PRINTBUFFER(buffer, 2);
  return 2;
}


// Adafruit_MQTT_Publish Definition ////////////////////////////////////////////

Adafruit_MQTT_Publish::Adafruit_MQTT_Publish(Adafruit_MQTT *mqttserver, 
                                             const char *feed, uint8_t q) {
  mqtt = mqttserver;
  topic = feed;
  qos = q;
}

Adafruit_MQTT_Publish::Adafruit_MQTT_Publish(Adafruit_MQTT *mqttserver,
                                             const __FlashStringHelper *feed, uint8_t q) {
  mqtt = mqttserver;
  topic = (const char *)feed;
  qos = q;
}

bool Adafruit_MQTT_Publish::publish(int32_t i) {
  char payload[18];
  itoa(i, payload, 10);
  return mqtt->publish(topic, payload, qos);
}

bool Adafruit_MQTT_Publish::publish(double f, uint8_t precision) {
  char payload[40];  // Need to technically hold float max, 39 digits and minus sign.
  dtostrf(f, 0, precision, payload);
  return mqtt->publish(topic, payload, qos);
}

bool Adafruit_MQTT_Publish::publish(uint32_t i) {
  char payload[18];
  itoa(i, payload, 10);
  return mqtt->publish(topic, payload, qos);
}

bool Adafruit_MQTT_Publish::publish(const char *payload) {
  return mqtt->publish(topic, payload, qos);
}


// Adafruit_MQTT_Subscribe Definition //////////////////////////////////////////

Adafruit_MQTT_Subscribe::Adafruit_MQTT_Subscribe(Adafruit_MQTT *mqttserver, 
                                                 const char *feed, uint8_t q) {
  mqtt = mqttserver;
  topic = feed;
  qos = q;
}

Adafruit_MQTT_Subscribe::Adafruit_MQTT_Subscribe(Adafruit_MQTT *mqttserver,
                                                 const __FlashStringHelper *feed, uint8_t q) {
  mqtt = mqttserver;
  topic = (const char *)feed;
  qos = q;
}
