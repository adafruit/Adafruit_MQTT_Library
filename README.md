# Adafruit MQTT Library [![Build Status](https://travis-ci.org/adafruit/Adafruit_MQTT_Library.svg?branch=master)](https://travis-ci.org/adafruit/Adafruit_MQTT_Library)

Arduino library for MQTT support, including access to Adafruit IO.  Works with
the Adafruit CC3000, FONA, Arduino Yun, ESP8266 Arduino platforms, and anything that supports
Arduino's Client interface (like Ethernet shield).

See included examples for how to use the library to access an MQTT service to
publish and subscribe to feeds.  Note that this does not support the full MQTT
spec but is intended to support enough for QoS 0 and 1 publishing.

Depends on the following other libraries depending on the target platform:

   - [Adafruit SleepyDog](https://github.com/adafruit/Adafruit_SleepyDog), watchdog
   library used by FONA and CC3000 code for reliability.

   - [Adafruit CC3000](https://github.com/adafruit/Adafruit_CC3000_Library), required
   for the CC3000 hardware.

   - [Adafruit FONA](https://github.com/adafruit/Adafruit_FONA_Library), required for
   the FONA hardware.

Future todos:

   - QoS > 0 for publication

   - Subscription callbacks

   - example with way more timeout/reconnection protection

   - Wills

   - remove watchdog
