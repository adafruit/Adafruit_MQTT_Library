/****************************************************************
  Adafruit MQTT Library, Adafruit IO SSL/TLS Example for AirLift

  Must use the latest version of nina-fw from:
    https://github.com/adafruit/nina-fw

  Works great with Adafruit AirLift ESP32 Co-Processors!
    --> https://www.adafruit.com/product/4201
    --> https://www.adafruit.com/product/4116

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Brent Rubell for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 *******************************************************************/

#include <WiFiNINA.h>
#include <SPI.h>

// For AirLift Breakout/Wing/Shield: Configure the following to match the ESP32 Pins!
#if !defined(SPIWIFI_SS)
  // Don't change the names of these #define's! they match the variant ones
  #define SPIWIFI SPI
  #define SPIWIFI_SS 11  // Chip select pin
  #define SPIWIFI_ACK 10 // a.k.a BUSY or READY pin
  #define ESP32_RESETN 9 // Reset pin
  #define ESP32_GPIO0 -1 // Not connected
  #define SET_PINS 1     // Pins were set using this IFNDEF
#endif

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* WiFi Access Point *********************************/

#define WLAN_SSID "WLAN_SSID"
#define WLAN_PASS "WIFI_PASSWORD"
int keyIndex = 0; // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;
/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER "io.adafruit.com"
// Using port 8883 for MQTTS
#define AIO_SERVERPORT 8883
// Adafruit IO Account Configuration
// (to obtain these values, visit https://io.adafruit.com and click on Active Key)
#define AIO_USERNAME "YOUR_ADAFRUIT_IO_USERNAME"
#define AIO_KEY "YOUR_ADAFRUIT_IO_KEY"

/************ Global State (you don't need to change this!) ******************/

// WiFiSSLClient for SSL/TLS support
WiFiSSLClient client;

// Setup the MQTT client class by WLAN_PASSing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
/****************************** Feeds ***************************************/

// Setup a feed called 'test' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish test = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/test");

/*************************** Sketch Code ************************************/

void setup()
{
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // if the AirLift's pins were defined above...
  #ifdef SET_PINS
    WiFi.setPins(SPIWIFI_SS, SPIWIFI_ACK, ESP32_RESETN, ESP32_GPIO0, &SPIWIFI);
  #endif

  // check for the wifi module
  while (WiFi.status() == WL_NO_MODULE)
  {
    Serial.println("Communication with WiFi module failed!");
    delay(1000);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0")
  {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(WLAN_SSID);
  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
  do
  {
    status = WiFi.begin(WLAN_SSID, WLAN_PASS);
    delay(100); // wait until connected
  } while (status != WL_CONNECTED);
  Serial.println("Connected to wifi");
  printWiFiStatus();
}

uint32_t x = 0;

void loop()
{
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // Now we can publish stuff!
  Serial.print(F("\nSending val "));
  Serial.print(x);
  Serial.print(F(" to test feed..."));
  if (!test.publish(x++))
  {
    Serial.println(F("Failed"));
  }
  else
  {
    Serial.println(F("OK!"));
  }

  // wait a couple seconds to avoid rate limit
  delay(2000);
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect()
{
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected())
  {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0)
  { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000); // wait 5 seconds
    retries--;
    if (retries == 0)
    {
      // basically die and wait for WDT to reset me
      while (1)
        ;
    }
  }

  Serial.println("MQTT Connected!");
}

void printWiFiStatus()
{
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}