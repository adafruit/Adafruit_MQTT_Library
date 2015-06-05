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

#define SERVERNAME_SIZE  25

#define PASSWORD_SIZE  50  // Need to be able to store an AIO key which is 41 chars.
#define USERNAME_SIZE  41
#define CLIENTID_SIZE  23

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

#define MAXBUFFERSIZE (105)  // Need to be able to store at least ~90 chars
                             // for a connect packet with full 23 char client ID.
#define MAXSUBSCRIPTIONS 5
#define SUBSCRIPTIONDATALEN 20


extern void printBuffer(uint8_t *buffer, uint8_t len);

class Adafruit_MQTT_Subscribe;  // forward decl

class Adafruit_MQTT {
 public:
  Adafruit_MQTT(const char *server, uint16_t port, const PROGMEM char *cid, 
                const PROGMEM char *user, const PROGMEM char *pass);
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
  int8_t connect();

  // Disconnect from the MQTT server.  Returns true if disconnected, false
  // otherwise.
  virtual bool disconnect() = 0;  // Subclasses need to fill this in!

  // Publish a message to a topic using the specified QoS level.  Returns true
  // if the message was published, false otherwise.
  bool publish(const char *topic, char *payload, uint8_t qos);
  
  // Add a subscription to receive messages for a topic.  Returns true if the
  // subscription could be added, false otherwise.
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
  uint8_t buffer[MAXBUFFERSIZE];

 private:
  Adafruit_MQTT_Subscribe *subscriptions[MAXSUBSCRIPTIONS];

  // Functions to generate MQTT packets.
  uint8_t connectPacket(uint8_t *packet);
  uint8_t publishPacket(uint8_t *packet, const char *topic, char *payload, uint8_t qos);
  uint8_t subscribePacket(uint8_t *packet, const char *topic, uint8_t qos);
  uint8_t pingPacket(uint8_t *packet);
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


#endif
