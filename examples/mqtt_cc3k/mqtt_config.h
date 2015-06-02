#define IDLE_TIMEOUT_MS 5000

/*************************** CC3000 Pins ***********************************/

// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   2  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  8
#define ADAFRUIT_CC3000_CS    10
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       ""        // cannot be longer than 32 characters!
#define WLAN_PASS       ""
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2

/************************* Adafruit.io Setup *********************************/

const char AIO_SERVER[] PROGMEM =  "io.adafruit.com";
#define AIO_SERVERPORT 1883
const char AIO_CLIENTNAME[] PROGMEM = "huzzah";
const char AIO_KEY[] PROGMEM = "";
const char AIO_PASSWORD[] PROGMEM = "";



const char DBG_SERVER[] PROGMEM =  "apt.adafruit.com";
#define DBG_SERVERPORT 1883
const char DBG_CLIENTNAME[] PROGMEM = "huzzah";
const char DBG_KEY[] PROGMEM = "";
const char DBG_PASSWORD[] PROGMEM = "";


#define halt(s) { Serial.println(F( s )); while(1);  }


#define println_P(s) {   for (uint8_t i=0; i<strlen_P(s); i++) { Serial.write(pgm_read_byte(s+i)); } }
boolean CC3000connect(void);

