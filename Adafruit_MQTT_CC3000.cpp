#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_CC3000.h"
#include <Adafruit_Watchdog.h>

static void printBuffer(uint8_t *buffer, uint8_t len) {
  for (uint8_t i=0; i<len; i++) {
    if (isprint(buffer[i]))
      Serial.write(buffer[i]);
    else  
      Serial.print(" ");
    Serial.print(F(" [0x"));
    if (buffer[i] < 0x10)
      Serial.print("0");
    Serial.print(buffer[i],HEX);
    Serial.print("], ");
    if (i % 8 == 7) Serial.println();
  }
  Serial.println();
}

Adafruit_MQTT_CC3000::Adafruit_MQTT_CC3000(Adafruit_CC3000 *cc3k, const char *server, uint16_t port, const char *cid, const char *user, const char *pass) : Adafruit_MQTT(server, port, cid, user, pass),  cc3000(cc3k)
{
  // nothin doin
}

int8_t Adafruit_MQTT_CC3000::connect(void) {
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
      if (!dnsretries) return -1;
      delay(500);
    }
    
    serverip = ip;
    cc3000->printIPdotsRev(serverip);  
    Serial.println();
  }

  Watchdog.reset();

  // connect to server
#ifdef DEBUG_MQTT_CONNECT  
  Serial.println(F("Connecting to TCP"));
#endif
  mqttclient = cc3000->connectTCP(serverip, portnum);
  uint8_t len = connectPacket(buffer);

#ifdef DEBUG_MQTT_CONNECT  
  Serial.println(F("MQTT connection packet:"));  printBuffer(buffer, len);
#endif
    
  if (mqttclient.connected()) {
    uint16_t ret = mqttclient.write(buffer, len);
#ifdef DEBUG_MQTT_CONNECT  
    Serial.print("returned: "); Serial.println(ret);
#endif
    if (ret != len)  return -1;
  } else {
#ifdef DEBUG_MQTT_CONNECT  
    Serial.println(F("Connection failed"));
#endif
    return -1;
  }
  
  len = readPacket(buffer, 4, CONNECT_TIMEOUT_MS);

  if (len != 4)  return -1;

  if ((buffer[0] != (MQTT_CTRL_CONNECTACK << 4)) || (buffer[1] != 2)) {
    return -1;
  }
  if (buffer[3] != 0) return buffer[3];

  
  /**************** subscription time! */
  for (uint8_t i=0; i<MAXSUBSCRIPTIONS; i++) {
    if (subscriptions[i] == 0) continue;
#ifdef DEBUG_MQTT_CONNECT
    Serial.print(F("Subscribing..."));
#endif
    uint8_t len = subscribePacket(buffer, subscriptions[i]->topic, subscriptions[i]->qos);
    
#ifdef DEBUG_MQTT_CONNECT
    Serial.println(F("MQTT subscription packet:"));  printBuffer(buffer, len);
#endif

    if (mqttclient.connected()) {
      uint16_t ret = mqttclient.write(buffer, len);
#ifdef DEBUG_MQTT_CONNECT  
      Serial.print("returned: "); Serial.println(ret);
#endif
      if (ret != len)  return -1;
    } else {
#ifdef DEBUG_MQTT_CONNECT  
      Serial.println(F("Connection failed"));    
#endif
      return -1;
    }
    
    // Get SUBACK
    len = readPacket(buffer, 5, CONNECT_TIMEOUT_MS);
#ifdef DEBUG_MQTT_CONNECT  
    Serial.print(F("SUBACK:\t"));  printBuffer(buffer, len);
#endif
    if ((len != 5) || (buffer[0] != (MQTT_CTRL_SUBACK << 4))) {
      return 6;  // failure to subscribe
    }
  }

  return 0;
}

uint16_t Adafruit_MQTT_CC3000::readPacket(uint8_t *buffer, uint8_t maxlen, int16_t timeout, boolean checkForValidPubPacket) {
  /* Read data until either the connection is closed, or the idle timeout is reached. */
  uint16_t len = 0;
  int16_t t = timeout;

  while (mqttclient.connected() && (timeout >= 0)) {
    //Serial.print('.');
    while (mqttclient.available()) {
      //Serial.print('!');
      char c = mqttclient.read();
      timeout = t;  // reset the timeout
      buffer[len] = c;
      //Serial.print((uint8_t)c,HEX);
      len++;
      if (len == maxlen) {  // we read all we want, bail
#ifdef DEBUG_MQTT_PACKETREAD
	Serial.print(F("Read packet:\t"));  printBuffer(buffer, len);
#endif
	return len;
      }

      // special case where we just one one publication packet at a time
      if (checkForValidPubPacket) {
	if ((buffer[0] == (MQTT_CTRL_PUBLISH << 4)) && (buffer[1] == len-2)) {
	  // oooh a valid publish packet!
#ifdef DEBUG_MQTT_PACKETREAD
	  Serial.print(F("PUBLISH packet:\t"));  printBuffer(buffer, len);
#endif
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

boolean Adafruit_MQTT_CC3000::ping(uint8_t times) {
  while (times) {
    uint8_t len = pingPacket(buffer);

    Serial.print(F("Sending:\t")); printBuffer(buffer, len);

    if (mqttclient.connected()) {
      uint16_t ret = mqttclient.write(buffer, len);
      //Serial.print("returned: "); Serial.println(ret);
      if (ret != len)  return false;
    } else {
      Serial.println(F("Connection failed"));    
      return false;
    }
    
    // process ping reply
    len = readPacket(buffer, 2, PING_TIMEOUT_MS);

    if (buffer[0] == (MQTT_CTRL_PINGRESP << 4))
      return true;  
  }
  return false;
}

int32_t Adafruit_MQTT_CC3000::close(void) {
  return mqttclient.close(); 
}

boolean Adafruit_MQTT_CC3000::publish(const char *topic, char *data, uint8_t qos) {
  uint8_t len = publishPacket(buffer, topic, data, qos);

#ifdef DEBUG_MQTT_PUBLISH
  Serial.println(F("MQTT publish packet:"));  printBuffer(buffer, len);
#endif

  if (mqttclient.connected()) {
    uint16_t ret = mqttclient.write(buffer, len);
#ifdef DEBUG_MQTT_PUBLISH
    Serial.print("returned: "); Serial.println(ret);
#endif
    if (ret != len)  return false;
  } else {
#ifdef DEBUG_MQTT_PUBLISH
    Serial.println(F("Connection failed"));    
#endif
    return false;
  }
  
  if (qos > 0) {
    len = readPacket(buffer, 4, PUBLISH_TIMEOUT_MS);

#ifdef DEBUG_MQTT_PUBLISH
    Serial.print(F("Reply:\t"));  printBuffer(buffer, len);
#endif

    return true;
  } else {
    return true;
  }
}


boolean Adafruit_MQTT_CC3000::subscribe(Adafruit_MQTT_Subscribe *sub) {
  uint8_t i;
  // see if we are already subscribed
  for (i=0; i<MAXSUBSCRIPTIONS; i++) {
    if (subscriptions[i] == sub) {
#ifdef DEBUG_MQTT_SUBSCRIBE 
      Serial.println(F("Already subscribed"));
#endif
      break;
    }
  }
  if (i==MAXSUBSCRIPTIONS) { // add to subscriptionlist
      for (i=0; i<MAXSUBSCRIPTIONS; i++) {
	if (subscriptions[i] == 0) {
#ifdef DEBUG_MQTT_SUBSCRIBE
	  Serial.print(F("Added sub ")); Serial.println(i);
#endif
	  subscriptions[i] = sub;
	  break;
	}
      } 
  }
  if (i==MAXSUBSCRIPTIONS) {
#ifdef DEBUG_MQTT_SUBSCRIBE
    Serial.println(F("no more space :("));
#endif
    return false;
  }
}

Adafruit_MQTT_Subscribe *Adafruit_MQTT_CC3000::readSubscription(int16_t timeout) {
  uint8_t i, topiclen, datalen;

#ifdef DEBUG_MQTT_READSUB
  Serial.println(F("reading..."));
#endif

  uint16_t len = readPacket(buffer, MAXBUFFERSIZE, timeout, true); // return one full packet

#ifdef DEBUG_MQTT_READSUB
  printBuffer(buffer, len);
#endif

  if (!len) return NULL;

  topiclen = buffer[3];
#ifdef DEBUG_MQTT_READSUB
   Serial.print(F("Looking for subscription len ")); Serial.println(topiclen);
#endif

  // figure out what subscription this is!
  for (i=0; i<MAXSUBSCRIPTIONS; i++) {
    if (subscriptions[i]) {
      //Serial.print(i);
      boolean flag = true;
      // TODO: REPLACE WITH MEMCMP?
      for (uint8_t k=0; k<topiclen; k++) {
	if ( buffer[4+k] != pgm_read_byte(subscriptions[i]->topic+k) ) 
	  flag = false;
      }
      if (flag) {
#ifdef DEBUG_MQTT_READSUB
	Serial.println((char *)buffer+4);
	Serial.print(F("Found sub #")); Serial.println(i);
#endif
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

#ifdef DEBUG_MQTT_READSUB
  Serial.print(F("Data len: ")); Serial.println(datalen);
  Serial.print("Data: "); Serial.println((char *)subscriptions[i]->lastread);
#endif

  // return the valid matching subscription
  return subscriptions[i];
}
