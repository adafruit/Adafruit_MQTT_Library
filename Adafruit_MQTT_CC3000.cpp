#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_CC3000.h"
#include <Adafruit_Watchdog.h>

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
  Serial.println(F("Connecting to TCP"));
  mqttclient = cc3000->connectTCP(serverip, portnum);
  uint8_t len = connectPacket(buffer);
  Serial.println(F("MQTT connection packet:"));
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
    
  if (mqttclient.connected()) {
    uint16_t ret = mqttclient.write(buffer, len);
    //Serial.print("returned: "); Serial.println(ret);
    if (ret != len)  return -1;
  } else {
    Serial.println(F("Connection failed"));    
    return -1;
  }
  
  len = readPacket(buffer, 4, CONNECT_TIMEOUT_MS);

  if (len != 4)  return -1;

  if ((buffer[0] == (MQTT_CTRL_CONNECTACK << 4)) && (buffer[1] == 2)) {
    // good packet structure
    return buffer[3];
  }

  return -1;
}

uint16_t Adafruit_MQTT_CC3000::readPacket(uint8_t *buffer, uint8_t maxlen, int16_t timeout) {
  /* Read data until either the connection is closed, or the idle timeout is reached. */
  uint16_t len = 0;
  int16_t t = timeout;

  while (mqttclient.connected() && (timeout > 0)) {
    Serial.print('.');
    while (mqttclient.available()) {
      Serial.print('!');
      char c = mqttclient.read();
      timeout = t;  // reset the timeout
      buffer[len] = c;
      //Serial.print((uint8_t)c,HEX);
      len++;
      if (len == maxlen) {  // we read all we want, bail

	Serial.print(F("Read packet:\t"));
	for (uint8_t i=0; i<len; i++) {
	  if (isprint(buffer[i])) 
	    Serial.write(buffer[i]);  
	  else 
	    Serial.write(' ');
	  Serial.print(" [0x"); Serial.print(buffer[i], HEX); Serial.print("], ");
	}
	Serial.println();
	
	return len;
      }
    }
    Watchdog.reset();
    timeout-=10;
    delay(10);
  }
  return len;
}

boolean Adafruit_MQTT_CC3000::ping(uint8_t times) {
  while (times) {
    uint8_t len = pingPacket(buffer);
    Serial.print("Sending...\t");
    for (uint8_t i=0; i<len; i++) {
       Serial.print(" [0x"); Serial.print(buffer[i], HEX); Serial.print("], ");
    }
    Serial.println();
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
  Serial.println("MQTT publish packet:");
  for (uint8_t i=0; i<len; i++) {
    if (isprint(buffer[i]))
      Serial.write(buffer[i]);
    else  
      Serial.print(" ");
    Serial.print(" [0x");
    if (buffer[i] < 0x10)
      Serial.print("0");
    Serial.print(buffer[i],HEX);
    Serial.print("], ");
    if (i % 8 == 7) Serial.println();
  }
  Serial.println();
    
  if (mqttclient.connected()) {
    uint16_t ret = mqttclient.write(buffer, len);
    //Serial.print("returned: "); Serial.println(ret);
    if (ret != len)  return false;
  } else {
    Serial.println(F("Connection failed"));    
    return false;
  }
  
  if (qos > 0) {
    Serial.println(F("Reply:"));
    len = readPacket(buffer, 4, PUBLISH_TIMEOUT_MS);
    for (uint8_t i=0; i<len; i++) {
      Serial.write(buffer[i]); Serial.print(" [0x"); Serial.print(buffer[i], HEX); Serial.print("], ");
    }
    Serial.println();

    return true;
  } else {
    return true;
  }
}
