/***************************************************
  mqtt_airlift_subscribe_time_keepalive

  Example demonstrating MQTT subscription and manual
  KeepAlive handling for AirLift (ESP32 / NINA-FW)
  devices using the Adafruit_MQTT_Library.

  This example:
    * Connects an AirLift device to WiFi
    * Subscribes to io.adafruit.com/time/ISO-8601
    * Manually manages MQTT KeepAlive intervals
    * Sends periodic MQTT PINGs to maintain the
      MQTT session (workaround for NINA-FW behavior)

  Written by Bruce Conrad.
  MIT license, all text above must be included in
  any redistribution.
 ****************************************************/

#include <SPI.h>
#include <WiFiNINA.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* WiFi Setup *********************************/
// Update for your WiFi network.
#define WIFI_SSID   "YOUR_WIFI_SSID"
#define WIFI_PASS   "YOUR_WIFI_PASSWORD"

/************************* Adafruit IO Setup ***************************/
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883      // use 8883 for MQTTS
#define AIO_USERNAME    "YOUR_ADAFRUIT_IO_USERNAME"
#define AIO_KEY         "YOUR_ADAFRUIT_IO_KEY"

/************************* AirLift Pin Setup ***************************/
// These are the pins you provided and known-good.
#define SPIWIFI       SPI
#define SPIWIFI_SS    13
#define SPIWIFI_IRQ   11
#define SPIWIFI_RESET 12
#define ESP32_GPIO0   -1

/*************************** Timing / Params ***************************/
#define AIRLIFT_KEEPALIVE   30
#define MAX_MQTT_RETRIES    5
#define SUBSCRIBE_WAIT_MS   1500
#define PING_PERIOD_MS      ((AIRLIFT_KEEPALIVE - 10) * 1000)

/*************************** MQTT Objects ******************************/
WiFiClient client;

Adafruit_MQTT_Client mqtt(
  &client,
  AIO_SERVER,
  AIO_SERVERPORT,
  AIO_USERNAME,
  AIO_KEY
);

Adafruit_MQTT_Subscribe time_subscription =
  Adafruit_MQTT_Subscribe(&mqtt, "time/ISO-8601");

/*************************** State Vars *******************************/
unsigned long ping_clk = 0;
long ping_cnt = 0;

/*************************** Function Prototypes ***********************/
void initWiFi();
void MQTT_connect();
void MQTT_ping();
void printCurrentNet();
void printWiFiData();
void printMacAddress(byte mac[]);

/*************************** Setup *************************************/
void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("MQTT AirLift Subscribe + KeepAlive Example");

  initWiFi();

  mqtt.setKeepAliveInterval(AIRLIFT_KEEPALIVE);
  mqtt.subscribe(&time_subscription);

  MQTT_connect();
}

/*************************** Main Loop *********************************/
void loop() {

  MQTT_ping();

  Adafruit_MQTT_Subscribe *subscription =
    mqtt.readSubscription(SUBSCRIBE_WAIT_MS);

  if (subscription == &time_subscription) {
    Serial.print("TIME FEED: ");
    Serial.println((char *)time_subscription.lastread);
  }
}

/*************************** MQTT Connect ******************************/
void MQTT_connect() {

  unsigned long start = millis();
  long attempts = 0;

  while (attempts < MAX_MQTT_RETRIES) {
    attempts++;

    Serial.print("MQTT Connect Attempt #");
    Serial.print(attempts);
    Serial.print(" ... ");

    int8_t ret = mqtt.connect();

    if (ret == 0) {
      Serial.print("Connected in ");
      Serial.print(millis() - start);
      Serial.println(" ms");
      ping_clk = millis();
      return;
    }

    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying in 5 seconds...");
    mqtt.disconnect();
    delay(5000);
  }

  Serial.println("ERROR: Maximum MQTT connect attempts exceeded.");
  while (1) delay(10);
}

/*************************** MQTT PING Logic ***************************/
void MQTT_ping() {
  if ((millis() - ping_clk) < PING_PERIOD_MS) return;

  ping_cnt++;
  unsigned long start = millis();

  Serial.print("PING #");
  Serial.print(ping_cnt);
  Serial.print(" ... ");

  for (int i = 0; i < MAX_MQTT_RETRIES; i++) {

    if (mqtt.ping()) {
      Serial.print("OK (");
      Serial.print(millis() - start);
      Serial.println(" ms)");
      ping_clk = millis();
      return;
    }

    Serial.print("fail ");
    delay(1000);
  }

  Serial.println("\nERROR: No MQTT PING response. Halting.");
  while (1) delay(10);
}

/*************************** WiFi Init *********************************/
void initWiFi() {

  // IMPORTANT: passes the *SPI object* (SPIWIFI), not a pin
  WiFi.setPins(SPIWIFI_SS, SPIWIFI_IRQ, SPIWIFI_RESET, ESP32_GPIO0, &SPIWIFI);

  Serial.println("Starting AirLift WiFi...");

  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("ERROR: Communication with AirLift failed.");
    while (1) delay(10);
  }

  Serial.print("NINA-FW Version: ");
  Serial.println(WiFi.firmwareVersion());

  int status = WL_IDLE_STATUS;

  while (status != WL_CONNECTED) {
    Serial.print("Connecting to SSID: ");
    Serial.println(WIFI_SSID);

    status = WiFi.begin(WIFI_SSID, WIFI_PASS);

    while (WiFi.localIP()[0] == 0) {
      delay(50);
    }
  }

  Serial.println("WiFi connected.");
  printCurrentNet();
  printWiFiData();
}

/*************************** Helpers ***********************************/
void printCurrentNet() {
  Serial.print("SSID: ");
  Serial.print(WiFi.SSID());

  byte bssid[6];
  WiFi.BSSID(bssid);

  Serial.print("  BSSID: ");
  printMacAddress(bssid);

  Serial.print("  RSSI: ");
  Serial.print(WiFi.RSSI());

  Serial.print("  Encryption: ");
  Serial.println(WiFi.encryptionType(), HEX);
}

void printWiFiData() {
  Serial.print("IP: ");
  Serial.print(WiFi.localIP());

  Serial.print("  Gateway: ");
  Serial.print(WiFi.gatewayIP());

  Serial.print("  Subnet: ");
  Serial.print(WiFi.subnetMask());

  byte mac[6];
  WiFi.macAddress(mac);

  Serial.print("  MAC: ");
  printMacAddress(mac);
}

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) Serial.print("0");
    Serial.print(mac[i], HEX);
    if (i > 0) Serial.print(":");
  }
  Serial.print(" ");
}
