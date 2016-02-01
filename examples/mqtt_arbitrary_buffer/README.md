# Adafruit MQTT Library example with support

This example illustrates Publish an arbitrary data packet using the Adafruit MQTT library to an MQTT feed which can then be parsed by the included python subscriber client.


## Installing and running Mosquitto
####On a Raspberry Pi:
```bash
sudo apt-get install mosquitto
```

####On a Mac:
```bash
brew install mosquitto
```


## Using Example Python Subscriber:

Install dependents if haven't already
```bash
cd ../Adafruit_MQTT_Library/examples/mqtt_arbitrary_buffer/python_subscriber
pip install -r requirements.txt
```

Run python script with default values
```bash
python subscriber.py
```

Use help to see a list of modifiable options
```bash
python subscriber.py -h
```
