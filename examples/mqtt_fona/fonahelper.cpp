#include <Adafruit_SleepyDog.h>
#include "Adafruit_FONA.h"

#define halt(s) { Serial.println(F( s )); while(1);  }

extern Adafruit_FONA fona;
extern SoftwareSerial fonaSS;

boolean FONAconnect(const __FlashStringHelper *apn, const __FlashStringHelper *username, const __FlashStringHelper *password) {
  Watchdog.enable(8000);
  Watchdog.reset();

  Serial.println(F("Initializing FONA....(May take 3 seconds)"));
  
  fonaSS.begin(4800); // if you're using software serial
  
  if (! fona.begin(fonaSS)) {           // can also try fona.begin(Serial1) 
    Serial.println(F("Couldn't find FONA"));
    return false;
  }
  fonaSS.println("AT+CMEE=2");
  Serial.println(F("FONA is OK"));
  Watchdog.reset();
  Serial.println(F("Checking for network..."));
  while (fona.getNetworkStatus() != 1) {
   delay(500);
  }
  Watchdog.reset();
  
  fona.setGPRSNetworkSettings(apn, username, password);

  Serial.println(F("Disabling GPRS"));
  fona.enableGPRS(false);
  
  Watchdog.reset();
  Serial.println(F("Enabling GPRS"));
  if (!fona.enableGPRS(true)) {
    Serial.println(F("Failed to turn GPRS on"));  
    return false;
  }
  Watchdog.reset();

  return true;
}

