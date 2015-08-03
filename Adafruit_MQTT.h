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
#ifndef _ADAFRUIT_MQTT_H_
#define _ADAFRUIT_MQTT_H_

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

// Uncomment/comment to turn on/off debug output messages.
//#define MQTT_DEBUG

// Set where debug messages will be printed.
#define DEBUG_PRINTER Serial

// Define actual debug output functions when necessary.
#ifdef MQTT_DEBUG
  #define DEBUG_PRINT(...) { DEBUG_PRINTER.print(__VA_ARGS__); }
  #define DEBUG_PRINTLN(...) { DEBUG_PRINTER.println(__VA_ARGS__); }
  #define DEBUG_PRINTBUFFER(buffer, len) { printBuffer(buffer, len); }
#else
  #define DEBUG_PRINT(...) {}
  #define DEBUG_PRINTLN(...) {}
  #define DEBUG_PRINTBUFFER(buffer, len) {}
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

#define CONNECT_TIMEOUT_MS 3000
#define PUBLISH_TIMEOUT_MS 500
#define PING_TIMEOUT_MS 500

// Adjust as necessary, in seconds.  Default to 5 minutes.
#define MQTT_CONN_KEEPALIVE 300

// Largest full packet we're able to send.
// Need to be able to store at least ~90 chars for a connect packet with full
// 23 char client ID.
#define MAXBUFFERSIZE (125)

#define MQTT_CONN_USERNAMEFLAG 0x80
#define MQTT_CONN_PASSWORDFLAG 0x40
#define MQTT_CONN_WILLRETAIN   0x20
#define MQTT_CONN_WILLQOS   0x08
#define MQTT_CONN_WILLFLAG   0x04
#define MQTT_CONN_CLEANSESSION   0x02

// how many subscriptions we want to be able to
// track
#define MAXSUBSCRIPTIONS 5

// how much data we save in a subscription object
// eg max-subscription-payload-size
#define SUBSCRIPTIONDATALEN 20


extern void printBuffer(uint8_t *buffer, uint8_t len);

class Adafruit_MQTT_Subscribe;  // forward decl

class Adafruit_MQTT {
 public:
  Adafruit_MQTT(const char *server, uint16_t port, const char *cid,
                const char *user, const char *pass);
  Adafruit_MQTT(const __FlashStringHelper *server, uint16_t port, const __FlashStringHelper *cid,
                const __FlashStringHelper *user, const __FlashStringHelper *pass);
  virtual ~Adafruit_MQTT() {}

  // Connect to the MQTT server.  Returns 0 on success, otherwise an error code
  // that indicates something went wrong:
  //   -1 = Error connecting to server
  //    1 = Wrong protocol
  //    2 = ID rejected
  //    3 = Server unavailable
  //    4 = Bad username or password
  //    5 = Not authenticated
  //    6 = Failed to subscribe
  // Use connectErrorString() to get a printable string version of the
  // error.
  int8_t connect();

  // Return a printable string version of the error code returned by
  // connect(). This returns a __FlashStringHelper*, which points to a
  // string stored in flash, but can be directly passed to e.g.
  // Serial.println without any further processing.
  const __FlashStringHelper* connectErrorString(int8_t code);

  // Disconnect from the MQTT server.  Returns true if disconnected, false
  // otherwise.
  virtual bool disconnect() = 0;  // Subclasses need to fill this in!

  // Return true if connected to the MQTT server, otherwise false.
  virtual bool connected() = 0;  // Subclasses need to fill this in!

  // Publish a message to a topic using the specified QoS level.  Returns true
  // if the message was published, false otherwise.
  // The topic must be stored in PROGMEM. It can either be a
  // char*, or a __FlashStringHelper* (the result of the F() macro).
  bool publish(const char *topic, const char *payload, uint8_t qos = 0);
  bool publish(const __FlashStringHelper *topic, const char *payload, uint8_t qos = 0) {
    return publish((const char *)topic, payload, qos);
  }

  // Add a subscription to receive messages for a topic.  Returns true if the
  // subscription could be added or was already present, false otherwise.
  // Must be called before connect(), subscribing after the connection
  // is made is not currently supported.
  bool subscribe(Adafruit_MQTT_Subscribe *sub);

  // Check if any subscriptions have new messages.  Will return a reference to
  // an Adafruit_MQTT_Subscribe object which has a new message.  Should be called
  // in the sketch's loop function to ensure new messages are recevied.  Note
  // that subscribe should be called first for each topic that receives messages!
  Adafruit_MQTT_Subscribe *readSubscription(int16_t timeout=0);

  // Ping the server to ensure the connection is still alive.  Returns true if
  // successful, otherwise false.
  bool ping(uint8_t t);

 protected:
  // Interface that subclasses need to implement:

  // Connect to the server and return true if successful, false otherwise.
  virtual bool connectServer() = 0;

  // Send data to the server specified by the buffer and length of data.
  virtual bool sendPacket(uint8_t *buffer, uint8_t len) = 0;

  // Read MQTT packet from the server.  Will read up to maxlen bytes and store
  // the data in the provided buffer.  Waits up to the specified timeout (in
  // milliseconds) for data to be available.  If checkForValidPubPacket is true
  // then the received data is verified to make sure it's a complete packet.
  virtual uint16_t readPacket(uint8_t *buffer, uint8_t maxlen, int16_t timeout,
                              bool checkForValidPubPacket = false) = 0;

  // Shared state that subclasses can use:
  const char *servername;
  int16_t portnum;
  const char *clientid;
  const char *username;
  const char *password;
  uint8_t buffer[MAXBUFFERSIZE];  // one buffer, used for all incoming/outgoing

 private:
  Adafruit_MQTT_Subscribe *subscriptions[MAXSUBSCRIPTIONS];

  // Functions to generate MQTT packets.
  uint8_t connectPacket(uint8_t *packet);
  uint8_t publishPacket(uint8_t *packet, const char *topic, const char *payload, uint8_t qos);
  uint8_t subscribePacket(uint8_t *packet, const char *topic, uint8_t qos);
  uint8_t pingPacket(uint8_t *packet);
};


class Adafruit_MQTT_Publish {
 public:
  Adafruit_MQTT_Publish(Adafruit_MQTT *mqttserver, const char *feed, uint8_t qos = 0);
  Adafruit_MQTT_Publish(Adafruit_MQTT *mqttserver, const __FlashStringHelper *feed, uint8_t qos = 0);

  bool publish(const char *s);
  bool publish(double f, uint8_t precision=2);  // Precision controls the minimum number of digits after decimal.
                                                // This might be ignored and a higher precision value sent.
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
  Adafruit_MQTT_Subscribe(Adafruit_MQTT *mqttserver, const __FlashStringHelper *feedname, uint8_t q=0);

  bool setCallback(void (*callback)(char *));

  const char *topic;
  uint8_t qos;

  uint8_t lastread[SUBSCRIPTIONDATALEN];
  // Number valid bytes in lastread. Limited to SUBSCRIPTIONDATALEN-1 to
  // ensure nul terminating lastread.
  uint8_t datalen;
 private:
  Adafruit_MQTT *mqtt;
};


#endif
