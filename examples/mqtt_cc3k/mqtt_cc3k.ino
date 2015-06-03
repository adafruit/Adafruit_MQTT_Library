#include <Adafruit_Watchdog.h>
#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include "utility/debug.h"
#include <avr/pgmspace.h>

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_CC3000.h"
#include "mqtt_config.h"

Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT, SPI_CLOCK_DIV2); // you can change this clock speed

Adafruit_MQTT_CC3000 mqtt(&cc3000, AIO_SERVER, AIO_SERVERPORT, AIO_CLIENTNAME, AIO_USERNAME, AIO_KEY);

const char PHOTOCELL_FEED[] PROGMEM = "api/feeds/photocell/data/send.json";
Adafruit_MQTT_Publish photocell = Adafruit_MQTT_Publish(&mqtt, PHOTOCELL_FEED);

const char ONOFF_FEED[] PROGMEM = "api/feeds/onoff/data/receive.json";
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, ONOFF_FEED);


void setup() {
  Serial.begin(115200);

  Serial.println(F("Adafruit MQTT demo"));

  Serial.print(F("Free RAM: ")); Serial.println(getFreeRam(), DEC);

  // Initialise the CC3000 module
  Serial.print(F("\nInit the CC3000..."));
  if (!cc3000.begin()) 
      halt("Failed");

  mqtt.subscribe(&onoffbutton);

  while (! CC3000connect()) {
    Serial.println(F("Retrying WiFi"));
    while(1);
  }

  Serial.println(F("Connected to WiFi!"));
  //////////////////////////////
  Serial.println(F("Connecting to MQTT..."));
  int8_t ret;
  while ((ret = mqtt.connect()) != 0) {
       switch (ret) {
          case 1: Serial.println(F("Wrong protocol")); break;
          case 2: Serial.println(F("ID rejected")); break;
          case 3: Serial.println(F("Server unavail")); break;
          case 4: Serial.println(F("Bad user/pass")); break;
          case 5: Serial.println(F("Not authed")); break;
          case 6: Serial.println(F("Failed to subscribe")); break;
          default: {
            Serial.println(F("Connection failed")); 
            CC3000connect();  // y0w, lets connect to wifi again
            return;           // restart the loop
          }
       }
       Serial.println(F("Retrying MQTT connection"));
       delay(5000); 
  }
  //////////////////////////////

  Serial.println(F("MQTT Connected!"));  
}

uint32_t x=0;

void loop() {
  Adafruit_MQTT_Subscribe *subscription;

  Watchdog.reset();
 
  // Try to ping the MQTT server
  /*
  if (! mqtt.ping(3) ) {
    // MQTT pings failed, lets reconnect
  }
  */
  
  // this is our 'wait for incoming subscription packets' busy subloop
  while (subscription = mqtt.readSubscription(1000)) {
    if (subscription == &onoffbutton) {
      Serial.print(F("Got: ")); 
      Serial.println((char *)onoffbutton.lastread);
    }
  }
  
  // Now we can publish stuff!

  Serial.print(F("\nSending photocell val ")); 
  Serial.print(x);
  Serial.print("...");
  if (! photocell.publish(x++)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
}


