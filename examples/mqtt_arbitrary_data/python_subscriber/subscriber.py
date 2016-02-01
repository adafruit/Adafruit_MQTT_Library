'''MQTT subscriber for Adafruit MQTT library mqtt_arbitrary_buffer example'''
import paho.mqtt.client as mqtt
import argparse
import struct
import array
import sys

return_str =[
    "Connection successful",
    "incorrect protocol version",
    "invalid client identifier",
    "server unavailable",
    "bad username or password",
    "not authorised"
    ]

args = None

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, rc):
    """callback function on connect. Subscribes or exits depending on outcome"""
    print("MQTT: "),
    print(return_str[rc])
    if(rc > 1):
        print("Connection refused - " + return_str[rc])
        sys.exit(rc)
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    else:
        print(return_str[rc])
        client.subscribe(args.topic)
        print("Subscribed to {}".format(args.topic))

def on_disconnect(client, userdata, rc):
    """Callback for disconnect"""
    if rc != 0:
        print("Unexpected disconnection.")
        client.reconnect()

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    try:
        pMsg = parseMsg(msg.payload)
        print("Received char Array: \"{}\", val1: {}, val2: {}, val3: {}...".format(pMsg[0], pMsg[1], pMsg[2], pMsg[3]))

    except Exception as err:
        print err



def argBegin():
    parser = argparse.ArgumentParser(description='MQTT subscriber for Adafruit MQTT library mqtt_arbitrary_buffer example')
    parser.add_argument("--host", default="localhost", help='mqtt host to connect to. Defaults to localhost.')
    parser.add_argument("-p", "--port", default=1883, help='network port to connect to. Defaults to 1883.')
    parser.add_argument("-t", "--topic", nargs='*', default="/feeds/arb_packet", help="mqtt topic to subscribe to. May be repeated multiple times.")
    parser.add_argument("-u", "--username", default="testPy", help="provide a username (requires MQTT 3.1 broker)")
    parser.add_argument("-P", "--password", default="testPy", help="provide a password (requires MQTT 3.1 broker)")
    parser.add_argument("-k", "--keepalive", default=60, help="keep alive in seconds for this client. Defaults to 60.")

    return parser.parse_args()


def parseMsg(payload):
    """Parses C struct from MQTT publisher Adafruit MQTT client to Python list"""

    arr = array.array('B', payload) #convert python list to C-like array of unsigned char (B)

    parsedStruct = struct.Struct('< 10s h L H') #define struct template (see below)
    '''
        Format of Struct from Adafruit MQTT client, Arduino, etc:

        Adafruit MQTT client == Little endian (<)

             Var NAME   | C TYPE (python symbol)    | size of member x bytes
             -------------------------------------------------------------------
              "charAry" |  uchar (s)               |   10s x 1 = 10 bytes
              "val1"    |  int16 / short (h)       |   1h  x 2 =  2 bytes
              "val2"    |  unsigned long (L)       |   1L  x 4 =  4 bytes
              "val3"    |  uint16/unsigned short(H)|   1H  x 2 =  2 bytes
              ------------------------------------------------------------------
                                    Total packet size =         | 18 bytes |

        See Section 7.3.2 of Python struct module documentation for complete format list
        https://docs.python.org/2/library/struct.html
    '''

    charAry, val1, val2, val3 = parsedStruct.unpack_from(arr)  #convert byte array to formatted struct
    charAry = charAry.rstrip(' \t\r\n\0') #remove trailing white space from buffer
    return charAry, val1, val2, val3







def main():
    """Wait for incoming message published by Adafruit MQTT client"""
    global args
    args = argBegin()
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    client.username_pw_set(args.username, args.password)
    client.connect(args.host, args.port, args.keepalive)

    # Blocking call that processes network traffic, dispatches callbacks and
    # handles reconnecting.
    # Other loop*() functions are available that give a threaded interface and a
    # manual interface.
    client.loop_forever()

if __name__ == '__main__':
    sys.exit(main())
