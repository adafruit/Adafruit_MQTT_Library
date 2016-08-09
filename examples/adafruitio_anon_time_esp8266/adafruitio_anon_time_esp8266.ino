/***************************************************
  Adafruit MQTT Library ESP8266 Adafruit IO Anonymous Time Query

  Must use the latest version of ESP8266 Arduino from:
    https://github.com/esp8266/Arduino

  Works great with Adafruit's Huzzah ESP board & Feather
  ----> https://www.adafruit.com/product/2471
  ----> https://www.adafruit.com/products/2821

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* WiFi Access Point *********************************/
#define WLAN_SSID       "network"
#define WLAN_PASS       "password"

/************************* Adafruit.io Setup *********************************/
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  8883

WiFiClientSecure client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT);

Adafruit_MQTT_Subscribe timefeed = Adafruit_MQTT_Subscribe(&mqtt, "time/seconds");

// set timezone offset from UTC
int timeZone = -4; // UTC - 4 eastern daylight time (nyc)
int interval = 4; // trigger every X hours

int last_min = -1;

void timecallback(uint32_t current) {
  // adjust to local time zone
  current += (timeZone * 60 * 60);
  int curr_hour = (current / 60 / 60) % 24;
  int curr_min  = (current / 60 ) % 60;
  int curr_sec  = (current) % 60;

  Serial.print("Time: "); 
  Serial.print(curr_hour); Serial.print(':'); 
  Serial.print(curr_min); Serial.print(':'); 
  Serial.println(curr_sec);
  
  // only trigger on minute change
  if(curr_min != last_min) {
    last_min = curr_min;
    
    Serial.println("This will print out every minute!");
  }
}

void setup() {

  Serial.begin(115200);
  delay(10);

  Serial.print(F("\nAdafruit IO anonymous Time Demo"));

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println(F(" WiFi connected."));

  timefeed.setCallback(timecallback);
  mqtt.subscribe(&timefeed);

}

void loop() {

  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // wait 10 seconds for subscription messages
  // since we have no other tasks in this example.
  mqtt.processPackets(10000);

  // keep the connection alive
  mqtt.ping();

}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }

  Serial.println("MQTT Connected!");
}