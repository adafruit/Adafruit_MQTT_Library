# Adafruit MQTT Library example with support

This example illustrates Publish an arbitrary data packet using the Adafruit MQTT library to an MQTT feed which can then be parsed by the included python subscriber client.

---

### Installing and configuring Mosquitto broker (minimal working setup):

####_Installing on Raspberry Pi/Linux:_

```bash
sudo apt-get install mosquitto
cd /etc/mosquitto/
#See "Both" Setup Instructions Below
```

####_Installing On a Mac:_
```bash
brew install mosquitto
cd /usr/local/etc/mosquitto
#See "Both" Setup Instructions Below
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

---

### Using Example Python Subscriber:

Install dependencies if haven't already
```bash
cd ../Adafruit_MQTT_Library/examples/mqtt_arbitrary_buffer/python_subscriber
pip install -r requirements.txt
```

Run python script with default values and watch your parsed data print out.
```bash
python subscriber.py
```

Use help to see a list of modifiable options
```bash
python subscriber.py -h
```
