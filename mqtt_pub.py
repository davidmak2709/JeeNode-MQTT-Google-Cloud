import paho.mqtt.client as mqtt
import ssl, jwt, datetime, json
from time import sleep
import socket

ca_certs = 'roots.pem'
public_crt = 'rsa_public.pem'
private_key_file = 'rsa_private.pem'

mqtt_url = "mqtt.googleapis.com"
mqtt_port = 8883
mqtt_topic = "/projects/corded-velocity-239915/topics/test_topic"
project_id   = "corded-velocity-239915"
cloud_region = "europe-west1"
registry_id  = "test_registry"
device_id    = "rpi"

connflag = False

def create_jwt(project_id, private_key_file, algorithm):
    token = {
            'iat': datetime.datetime.utcnow(),
            'exp': datetime.datetime.utcnow() + datetime.timedelta(minutes=60),
            'aud': project_id
    }

    with open(private_key_file, 'r') as f:
        private_key = f.read()

    print('Creating JWT using {} from private key file {}'.format(
            algorithm, private_key_file))

    return jwt.encode(token, private_key, algorithm=algorithm)

def error_str(rc):
    """Convert a Paho error to a human readable string."""
    return "Some error occurred. {}: {}".format(rc, mqtt.error_string(rc))

def on_disconnect(unused_client, unused_userdata, rc):
    """Paho callback for when a device disconnects."""
    print("on_disconnect", error_str(rc))

def on_connect(client, userdata, flags, response_code):
    global connflag
    connflag = True
    print("Connected with status: {0}".format(response_code))

def on_publish(client, userdata, mid):
    print("Published")

def datetime_handler(x):
    if isinstance(x, datetime.datetime):
        return x.isoformat()
    raise TypeError("Unknown type")

if __name__ == "__main__":

    client = mqtt.Client("projects/{}/locations/{}/registries/{}/devices/{}".format(
                         project_id,
                         cloud_region,
                         registry_id,
                         device_id))

    client.username_pw_set(username='unused',
                           password=create_jwt(project_id,
                                               private_key_file,
                                               algorithm="RS256"))

    client.tls_set(ca_certs=ca_certs, tls_version=ssl.PROTOCOL_TLSv1_2)

    client.on_connect = on_connect
    client.on_publish = on_publish
    client.on_disconnect = on_disconnect

    print("Connecting to Google IoT Broker...")
    client.connect(mqtt_url, mqtt_port, keepalive=60)

    socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    socket.bind(('10.42.0.1', 5001))
    
    while True:
        data, addr = socket.recvfrom(1024)
        data = data.decode("utf-8") 
        print ("received message:", data)
        
        client.loop()
        if connflag == True and data:
            print("Publishing...")
            data = data.replace("\x00","")
            raw_payload = {'timestamp': datetime.datetime.now(), 'light_data': int(data)}
            payload = json.dumps(raw_payload, default=datetime_handler)
            res = client.publish('/devices/{}/events'.format(device_id), payload, qos=1)
            data = None
            sleep(1)
        else:
            print("Waiting for connection...")
