#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* WiFi Access Point *********************************/
#define WLAN_SSID       "network"
#define WLAN_PASS       "password"

/************************* Adafruit.io Setup *********************************/
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  8883
#define AIO_USERNAME    "user"
#define AIO_KEY         "key"

WiFiClientSecure client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);

Adafruit_MQTT_Subscribe timefeed = Adafruit_MQTT_Subscribe(&mqtt, "time/seconds");

// set timezone offset from UTC
int timeZone = -4; // UTC - 4 eastern daylight time (nyc)
int interval = 4; // trigger every X hours
int hour = 0; // current hour

void timecallback(uint32_t current) {

  // stash previous hour
  int previous = hour;

  // adjust to local time zone
  current += (timeZone * 60 * 60);

  // calculate current hour
  hour = (current / 60 / 60) % 24;

  // only trigger on interval
  if((hour != previous) && (hour % interval) == 0) {
    Serial.println("Run your code here");
  }

}

void setup() {

  Serial.begin(115200);
  delay(10);

  Serial.print(F("Adafruit IO Time Demo"));

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

  int8_t code;

  // connect to adafruit io if not connected
  if(! mqtt.connected()) {

    Serial.print("Connecting to Adafruit IO... ");
    if((code = mqtt.connect()) != 0) {
      Serial.println(mqtt.connectErrorString(code));
      while(1);
    }
    Serial.println("connected!");

  }

  mqtt.processPackets(1000);
  mqtt.ping();

}
