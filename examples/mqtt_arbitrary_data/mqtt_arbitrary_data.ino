/***************************************************
  Adafruit MQTT Library Arbitrary Data Example

  Must use ESP8266 Arduino from:
    https://github.com/esp8266/Arduino

  Works great with Adafruit's Huzzah ESP board & Feather
  ----> https://www.adafruit.com/product/2471
  ----> https://www.adafruit.com/products/2821

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Stuart Feichtinger
  Modifed from the mqtt_esp8266 example written by Tony DiCola for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "...your SSID..."
#define WLAN_PASS       "...your password..."

/************************* Adafruit.io Setup *********************************/

#define ARB_SERVER      "...host computer ip address..."
#define ARB_SERVERPORT  1883                   // use 8883 for SSL
#define ARB_USERNAME    "TestUser"
#define ARB_PW         "TestUser"


/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, ARB_SERVER, ARB_SERVERPORT, ARB_USERNAME, ARB_PW);

/****************************** Feeds ***************************************/

// Setup a feed called 'arb_packet' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
#define ARB_FEED "/feeds/arb_packet"
Adafruit_MQTT_Publish ap = Adafruit_MQTT_Publish(&mqtt, ARB_FEED);


// Arbitrary Payload
// Union allows for easier interaction of members in struct form with easy publishing
// of "raw" bytes
typedef union{
    //Customize struct with whatever variables/types you like.

    struct __attribute__((__packed__)){  // packed to eliminate padding for easier parsing.
        char charAry[10];
        int16_t val1;
        unsigned long val2;
        uint16_t val3;
    }s;

    uint8_t raw[sizeof(s)];                    // For publishing

    /*
        // Alternate Option with anonymous struct, but manual byte count:

        struct __attribute__((__packed__)){  // packed to eliminate padding for easier parsing.
            char charAry[10];       // 10 x 1 byte  =   10 bytes
            int16_t val1;           // 1 x  2 bytes =   2 bytes
            unsigned long val2;     // 1 x  4 bytes =   4 bytes
            uint16_t val3;          // 1 x  2 bytes =   2 bytes
                                            -------------------
                                            TOTAL =    18 bytes
        };
        uint8_t raw[18];                    // For publishing
*/

} packet_t;

/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

void setup() {
  Serial.begin(115200);
  delay(10);

  Serial.println(F("Adafruit MQTT demo"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print(F("Connecting to "));
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println();

  Serial.println(F("WiFi connected"));
  Serial.println(F("IP address: ")); Serial.println(WiFi.localIP());

}

packet_t arbPac;

const char strVal[] PROGMEM = "Hello!";

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  //Update arbitrary packet values
  strcpy_P(arbPac.s.charAry, strVal);
  arbPac.s.val1 = -4533;
  arbPac.s.val2 = millis();
  arbPac.s.val3 = 3354;

  /*
   // Alternate Union with anonymous struct
   // (see union declaration above)

     strcpy_P(arbPac.charAry, strVal);
     arbPac.val1 = -4533;
     arbPac.val2 = millis();
     arbPac.val3 = 3354;
  */

  if (! ap.publish(arbPac.raw, sizeof(packet_t)))
        Serial.println(F("Publish Failed."));
      else {
        Serial.println(F("Publish Success!"));
        delay(500);
      }

      delay(10000);


  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  /*
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
  */
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print(F("Connecting to MQTT... "));

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println(F("Retrying MQTT connection in 5 seconds..."));
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println(F("MQTT Connected!"));
}
