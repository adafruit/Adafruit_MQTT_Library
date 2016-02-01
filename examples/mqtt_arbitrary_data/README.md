# Adafruit MQTT Library Arbitrary Data Publish Example

This example illustrates publishing an arbitrary data packet using the Adafruit MQTT library to an MQTT feed which can then be parsed by the included python subscriber client. Possible usage cases include adding metadata (collection time, sensor info etc) to a datapoint.

![alt-text](https://raw.githubusercontent.com/stuthedew/Adafruit_MQTT_Library/Arbitrary_data_publish/examples/mqtt_arbitrary_data/python_subscriber/mqtt_figure.png "Arbitrary data flow diagram")

My motivation for this was wanting to be able to include metadata to a post.
Specifically, I was playing around with a [Teviso RD3024 radiation sensor](http://www.teviso.com/en/products/radiation-sensor-rd3024.htm), and a salvaged Americium radiation source from a smoke detector, at varying distances from the sensor. I wanted a way to associate the collection time, and distance between the source and sensor with the actual radiation reading itself.


---

## Installing and configuring Mosquitto broker (minimal working setup):

####_Installing on Raspberry Pi/Linux:_

```bash
sudo apt-get install mosquitto
cd /etc/mosquitto/
#See "Configuring Mosquitto Broker below"
```

####_Installing On a Mac:_
```bash
brew install mosquitto
cd /usr/local/etc/mosquitto
#See "Configuring Mosquitto Broker below"
```

---

####Configuring Mosquitto broker
```bash
sudo nano mosquitto.conf
```
Now we have to enable a password file to correctly interface with the Adafruit MQTT library. Scroll about two thirds of the way down until you see:

```bash
# -----------------------------------------------------------------
# Default authentication and topic access control
# -----------------------------------------------------------------
```

You should see `#password_file` about a paragraph after that.
Change

```bash
#password_file
```

To

```bash
password_file pwfile
```

Now `ctrl-x` to save and exit.

You're almost done! We just have to create and populate the password file we just configured. The default user info is:
* **Arduino Subscriber:**
    * Username: TestUser
    * Password: TestUser

* **Python Subscriber:**
    * Username: TestPy
    * Password: TestPy

```bash
touch pwfile #create the password file
mosquitto_passwd pwfile TestUser #Enter and confirm password when prompted
mosquitto_passwd pwfile TestPy #Enter and confirm password when prompted
```

####Running Mosquitto broker
Now run Mosquitto broker to allow Arduino publisher and Python subscriber to communicate

```bash
mosquitto
```

---

## Using Example Python Subscriber:

####Installing Python subscriber
Install dependencies if you haven't already
```bash
cd ../Adafruit_MQTT_Library/examples/mqtt_arbitrary_buffer/python_subscriber
pip install -r requirements.txt
```


####Installing Python subscriber
Run python script with default values and watch your parsed data print out.
```bash
python subscriber.py #Add -h flag to see modifiable options
```

Assuming that the Mosquitto broker is running in the background and the Adafruit_MQTT client (Arduino) is publishing, you should see the example data print out every 10 seconds.

```bash
MQTT:  Connection successful
Connection successful
Subscribed to /feeds/arb_packet
Received char Array: "Hello!", val1: -4533, val2: 73102, val3: 3354...
Received char Array: "Hello!", val1: -4533, val2: 83611, val3: 3354...
Received char Array: "Hello!", val1: -4533, val2: 94115, val3: 3354...
```
