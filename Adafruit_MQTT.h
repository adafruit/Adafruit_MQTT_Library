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
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
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

#include "Arduino.h"
#include <stddef.h>
#include <stdint.h>

// On 8/16-bit targets (AVR) size_t is 16 bits wide, which makes it the *same*
// type as uint16_t; declaring both a uint16_t and a size_t overload there is a
// redefinition error. ADAFRUIT_MQTT_WIDE_SIZE_T is defined only where size_t is
// wider than uint16_t (ESP8266, ESP32, RP2040, SAMD); the wide overloads below
// are compiled only on those targets.
#if defined(__SIZEOF_SIZE_T__)
#if __SIZEOF_SIZE_T__ > 2
#define ADAFRUIT_MQTT_WIDE_SIZE_T 1
#endif
#elif defined(SIZE_MAX) && (SIZE_MAX > 0xFFFFu)
#define ADAFRUIT_MQTT_WIDE_SIZE_T 1
#endif

#if defined(ARDUINO_SAMD_ZERO) || defined(ARDUINO_STM32_FEATHER)
#define strncpy_P(dest, src, len) strncpy((dest), (src), (len))
#define strncasecmp_P(f1, f2, len) strncasecmp((f1), (f2), (len))
#endif

#define ADAFRUIT_MQTT_VERSION_MAJOR 0
#define ADAFRUIT_MQTT_VERSION_MINOR 17
#define ADAFRUIT_MQTT_VERSION_PATCH 0

// Uncomment/comment to turn on/off debug output messages.
// #define MQTT_DEBUG
// Uncomment/comment to turn on/off error output messages.
#define MQTT_ERROR

// Set where debug messages will be printed.
#define DEBUG_PRINTER Serial
// If using something like Zero or Due, change the above to SerialUSB

// Define actual debug output functions when necessary.
#ifdef MQTT_DEBUG
#define DEBUG_PRINT(...)                                                       \
  {                                                                            \
    DEBUG_PRINTER.print(__VA_ARGS__);                                          \
  }
#define DEBUG_PRINTLN(...)                                                     \
  {                                                                            \
    DEBUG_PRINTER.println(__VA_ARGS__);                                        \
  }
#define DEBUG_PRINTBUFFER(buffer, len)                                         \
  {                                                                            \
    printBuffer(buffer, len);                                                  \
  }
#else
#define DEBUG_PRINT(...)                                                       \
  {                                                                            \
  }
#define DEBUG_PRINTLN(...)                                                     \
  {                                                                            \
  }
#define DEBUG_PRINTBUFFER(buffer, len)                                         \
  {                                                                            \
  }
#endif

#ifdef MQTT_ERROR
#define ERROR_PRINT(...)                                                       \
  {                                                                            \
    DEBUG_PRINTER.print(__VA_ARGS__);                                          \
  }
#define ERROR_PRINTLN(...)                                                     \
  {                                                                            \
    DEBUG_PRINTER.println(__VA_ARGS__);                                        \
  }
#define ERROR_PRINTBUFFER(buffer, len)                                         \
  {                                                                            \
    printBuffer(buffer, len);                                                  \
  }
#else
#define ERROR_PRINT(...)                                                       \
  {                                                                            \
  }
#define ERROR_PRINTLN(...)                                                     \
  {                                                                            \
  }
#define ERROR_PRINTBUFFER(buffer, len)                                         \
  {                                                                            \
  }
#endif

// Use 3 (MQTT 3.0) or 4 (MQTT 3.1.1)
#define MQTT_PROTOCOL_LEVEL 4

#define MQTT_CTRL_CONNECT 0x1
#define MQTT_CTRL_CONNECTACK 0x2
#define MQTT_CTRL_PUBLISH 0x3
#define MQTT_CTRL_PUBACK 0x4
#define MQTT_CTRL_PUBREC 0x5
#define MQTT_CTRL_PUBREL 0x6
#define MQTT_CTRL_PUBCOMP 0x7
#define MQTT_CTRL_SUBSCRIBE 0x8
#define MQTT_CTRL_SUBACK 0x9
#define MQTT_CTRL_UNSUBSCRIBE 0xA
#define MQTT_CTRL_UNSUBACK 0xB
#define MQTT_CTRL_PINGREQ 0xC
#define MQTT_CTRL_PINGRESP 0xD
#define MQTT_CTRL_DISCONNECT 0xE

#define MQTT_QOS_1 0x1
#define MQTT_QOS_0 0x0

#define CONNECT_TIMEOUT_MS 6000
#define PUBLISH_TIMEOUT_MS 500
#define PING_TIMEOUT_MS 500
#define SUBACK_TIMEOUT_MS 500
// Minimum time to read the rest of a packet once its first byte arrives
#define PACKET_BODY_TIMEOUT_MS 5000

// Adjust as necessary, in seconds.  Default to 5 minutes.
#define MQTT_CONN_KEEPALIVE 300

// Largest full packet we're able to send.
// Need to be able to store at least ~90 chars for a connect packet with full
// 23 char client ID.
// Future TODO: This should be replaced by the ability to dynamically allocate a
// buffer as needed.
#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32) ||            \
    defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_SAMD)
#define MAXBUFFERSIZE (512)
#else
#define MAXBUFFERSIZE (150)
#endif

#define MQTT_CONN_USERNAMEFLAG 0x80
#define MQTT_CONN_PASSWORDFLAG 0x40
#define MQTT_CONN_WILLRETAIN 0x20
#define MQTT_CONN_WILLQOS_1 0x08
#define MQTT_CONN_WILLQOS_2 0x18
#define MQTT_CONN_WILLFLAG 0x04
#define MQTT_CONN_CLEANSESSION 0x02

// how much data we save in a subscription object
// and how many subscriptions we want to be able to track.
// NOTE: SUBSCRIPTIONDATALEN is only the *default* subscription payload size.
// Raising the MQTT buffer via the maxBufferSz constructor argument does not
// change it -- pass datalen_max to each Adafruit_MQTT_Subscribe as well.
#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega328P__)
#define MAXSUBSCRIPTIONS 5
#define SUBSCRIPTIONDATALEN 20
#else
#define MAXSUBSCRIPTIONS 15
#define SUBSCRIPTIONDATALEN MAXBUFFERSIZE
#endif

class AdafruitIO_MQTT; // forward decl

// Function pointer that returns an int
typedef void (*SubscribeCallbackUInt32Type)(uint32_t);
// returns a double
typedef void (*SubscribeCallbackDoubleType)(double);
// returns a chunk of raw data
typedef void (*SubscribeCallbackBufferType)(char *str, uint16_t len);
// returns an io data wrapper instance
typedef void (AdafruitIO_MQTT::*SubscribeCallbackIOType)(char *str,
                                                         uint16_t len);
#if defined(ADAFRUIT_MQTT_WIDE_SIZE_T)
// returns a chunk of raw data, with a size_t length so payloads larger than
// 64KB can report their true size
typedef void (*SubscribeCallbackBufferLargeType)(char *str, size_t len);
// returns an io data wrapper instance, with a size_t length
typedef void (AdafruitIO_MQTT::*SubscribeCallbackIOLargeType)(char *str,
                                                              size_t len);
#endif

extern void printBuffer(uint8_t *buffer, size_t len);

class Adafruit_MQTT_Subscribe; // forward decl

class Adafruit_MQTT {
public:
  Adafruit_MQTT(const char *server, uint16_t port, const char *cid,
                const char *user, const char *pass,
                size_t maxBufferSz = MAXBUFFERSIZE);

  Adafruit_MQTT(const char *server, uint16_t port, const char *user = "",
                const char *pass = "", size_t maxBufferSz = MAXBUFFERSIZE);
  virtual ~Adafruit_MQTT();

  // Allocate a zeroed buffer of expectedSz bytes, returns NULL if the
  // allocation fails
  static uint8_t *allocateMqttBuffer(size_t expectedSz);

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
  int8_t connect(const char *user, const char *pass);

  // Return a printable string version of the error code returned by
  // connect(). This returns a __FlashStringHelper*, which points to a
  // string stored in flash, but can be directly passed to e.g.
  // Serial.println without any further processing.
  const __FlashStringHelper *connectErrorString(int8_t code);

  // Sends MQTT disconnect packet and calls disconnectServer()
  bool disconnect();

  // Return true if connected to the MQTT server, otherwise false.
  virtual bool connected() = 0; // Subclasses need to fill this in!

  // Size of the packet buffer, in bytes, as actually allocated. Returns 0 if
  // the buffer allocation failed.
  size_t bufferSize() const { return maxBufferSize; }

  // Set MQTT last will topic, payload, QOS, and retain. This needs
  // to be called before connect() because it is sent as part of the
  // connect control packet.
  bool will(const char *topic, const char *payload, uint8_t qos = 0,
            uint8_t retain = 0);

  // Sets the KeepAlive Interval, in seconds.
  bool setKeepAliveInterval(uint16_t keepAlive);

  // Publish a message to a topic using the specified QoS level.  Returns true
  // if the message was published, false otherwise.
  bool publish(const char *topic, const char *payload, uint8_t qos = 0,
               bool retain = false);
  bool publish(const char *topic, uint8_t *payload, size_t bLen,
               uint8_t qos = 0, bool retain = false);

  // Add a subscription to receive messages for a topic.  Returns true if the
  // subscription could be added or was already present, false otherwise.
  // Must be called before connect(), subscribing after the connection
  // is made is not currently supported.
  bool subscribe(Adafruit_MQTT_Subscribe *sub);

  // Unsubscribe from a previously subscribed MQTT topic.
  bool unsubscribe(Adafruit_MQTT_Subscribe *sub);

  // Check if any subscriptions have new messages.  Will return a reference to
  // an Adafruit_MQTT_Subscribe object which has a new message.  Should be
  // called in the sketch's loop function to ensure new messages are recevied.
  // Note that subscribe should be called first for each topic that receives
  // messages!
  Adafruit_MQTT_Subscribe *readSubscription(int16_t timeout = 0);

  // Handle any data coming in for subscriptions
  Adafruit_MQTT_Subscribe *handleSubscriptionPacket(size_t len);

  // Execute a subscription packet's associated callback and mark as "read"
  void processSubscriptionPacket(Adafruit_MQTT_Subscribe *sub);

  void processPackets(int16_t timeout);

  // Ping the server to ensure the connection is still alive.
  bool ping(uint8_t n = 1);

protected:
  // Interface that subclasses need to implement:

  // Connect to the server and return true if successful, false otherwise.
  virtual bool connectServer() = 0;

  // Disconnect from the MQTT server.  Returns true if disconnected, false
  // otherwise.
  virtual bool disconnectServer() = 0; // Subclasses need to fill this in!

  // Send data to the server specified by the buffer and length of data.
  //
  // NOTE: subclasses MUST implement this uint16_t form -- it is what keeps
  // subclasses written against older versions of this library compiling.
  // Subclasses that want to send payloads larger than 64KB should *also*
  // override the size_t form below; the library only ever calls the size_t
  // form internally.
  virtual bool sendPacket(uint8_t *buffer, uint16_t len) = 0;

#if defined(ADAFRUIT_MQTT_WIDE_SIZE_T)
  // Wide form of sendPacket().  The default implementation forwards to the
  // uint16_t form so subclasses that have not been updated keep working; it
  // fails rather than truncating when len does not fit in a uint16_t.
  virtual bool sendPacket(uint8_t *buffer, size_t len) {
    if (len > 0xFFFFUL) {
      ERROR_PRINTLN(F("Packet too large for this sendPacket() implementation"));
      return false;
    }
    return sendPacket(buffer, (uint16_t)len);
  }
#endif

  // Read MQTT packet from the server.  Will read up to maxlen bytes and store
  // the data in the provided buffer.  Waits up to the specified timeout (in
  // milliseconds) for data to be available.
  //
  // NOTE: as with sendPacket(), subclasses MUST implement this uint16_t form.
  virtual uint16_t readPacket(uint8_t *buffer, uint16_t maxlen,
                              int16_t timeout) = 0;

#if defined(ADAFRUIT_MQTT_WIDE_SIZE_T)
  // Wide form of readPacket().  The default implementation clamps maxlen and
  // forwards to the uint16_t form; subclasses that support buffers larger than
  // 64KB should override this one instead.
  virtual size_t readPacket(uint8_t *buffer, size_t maxlen, int16_t timeout) {
    if (maxlen > 0xFFFFUL) {
      maxlen = 0xFFFFUL;
    }
    return readPacket(buffer, (uint16_t)maxlen, timeout);
  }
#endif

  // Read a full packet, keeping note of the correct length
  size_t readFullPacket(uint8_t *buffer, size_t maxsize, uint16_t timeout);
  // Properly process packets until you get to one you want
  size_t processPacketsUntil(uint8_t *buffer, uint8_t waitforpackettype,
                             uint16_t timeout);

  // Shared state that subclasses can use:
  const char *servername;
  int16_t portnum;
  const char *clientid;
  const char *username;
  const char *password;
  const char *will_topic;
  const char *will_payload;
  uint8_t will_qos;
  uint8_t will_retain;
  uint16_t keepAliveInterval; // MQTT KeepAlive time interval, in seconds
  uint8_t *buffer;      // one buffer, used for all incoming/outgoing payloads
  size_t maxBufferSize; // size of buffer, in bytes, 0 if allocation failed
  uint16_t packet_id_counter;

private:
  Adafruit_MQTT_Subscribe *subscriptions[MAXSUBSCRIPTIONS];

  void flushIncoming(uint16_t timeout);

  // Functions to generate MQTT packets.
  uint8_t connectPacket(uint8_t *packet);
  uint8_t disconnectPacket(uint8_t *packet);
  size_t publishPacket(uint8_t *packet, const char *topic, uint8_t *payload,
                       size_t bLen, uint8_t qos, size_t maxPacketLen = 0,
                       bool retain = false);
  uint8_t subscribePacket(uint8_t *packet, const char *topic, uint8_t qos);
  uint8_t unsubscribePacket(uint8_t *packet, const char *topic);
  uint8_t pingPacket(uint8_t *packet);
  uint8_t pubackPacket(uint8_t *packet, uint16_t packetid);
};

class Adafruit_MQTT_Publish {
public:
  Adafruit_MQTT_Publish(Adafruit_MQTT *mqttserver, const char *feed,
                        uint8_t qos = 0);
  bool publish(const char *s, bool retain = false);
  bool publish(
      double f,
      uint8_t precision =
          2, // Precision controls the minimum number of digits after decimal.
             // This might be ignored and a higher precision value sent.
      bool retain = false);
  bool publish(int32_t i, bool retain = false);
  bool publish(uint32_t i, bool retain = false);
  bool publish(uint8_t *b, size_t bLen, bool retain = false);

private:
  Adafruit_MQTT *mqtt;
  const char *topic;
  uint8_t qos;
};

class Adafruit_MQTT_Subscribe {
public:
  Adafruit_MQTT_Subscribe(Adafruit_MQTT *mqttserver, const char *feedname,
                          uint8_t q = 0,
                          size_t datalen_max = SUBSCRIPTIONDATALEN);
  ~Adafruit_MQTT_Subscribe();

  // Each subscription owns its payload buffer and must not be copied.
  Adafruit_MQTT_Subscribe(const Adafruit_MQTT_Subscribe &) = delete;
  Adafruit_MQTT_Subscribe &operator=(const Adafruit_MQTT_Subscribe &) = delete;

  void setCallback(SubscribeCallbackUInt32Type callb);
  void setCallback(SubscribeCallbackDoubleType callb);
  void setCallback(SubscribeCallbackBufferType callb);
  void setCallback(AdafruitIO_MQTT *io, SubscribeCallbackIOType callb);
#if defined(ADAFRUIT_MQTT_WIDE_SIZE_T)
  void setCallback(SubscribeCallbackBufferLargeType callb);
  void setCallback(AdafruitIO_MQTT *io, SubscribeCallbackIOLargeType callb);
#endif
  void removeCallback(void);

  const char *topic;
  uint8_t qos;

  uint8_t *lastread;
  size_t lastread_max; // size of lastread, in bytes, 0 if allocation failed
  // Number of valid bytes in lastread. Limited to lastread_max-1 to ensure
  // NUL-terminated lastread.
  size_t datalen;

  SubscribeCallbackUInt32Type callback_uint32t;
  SubscribeCallbackDoubleType callback_double;
  SubscribeCallbackBufferType callback_buffer;
  SubscribeCallbackIOType callback_io;
#if defined(ADAFRUIT_MQTT_WIDE_SIZE_T)
  SubscribeCallbackBufferLargeType callback_buffer_large;
  SubscribeCallbackIOLargeType callback_io_large;
#endif

  AdafruitIO_MQTT *io_mqtt;

  bool new_message;

private:
  Adafruit_MQTT *mqtt;
};

#endif
