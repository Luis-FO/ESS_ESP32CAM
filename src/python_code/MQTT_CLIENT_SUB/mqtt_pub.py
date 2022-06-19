import paho.mqtt.client as mqtt 
from random import randrange, uniform
import time

#mqttBroker ="mqtt.eclipseprojects.io"
mqttBroker = "192.168.15.12"

client = mqtt.Client("PUB")
client.username_pw_set(username="luis", password="DMK178qtS")
client.connect(mqttBroker, port=1883) 

while True:
    randNumber = uniform(20.0, 21.0)
    # client.publish("/topic/qos0", randNumber)
    client.publish("topic/imagem", randNumber)
    print("Just published " + str(randNumber) + " to topic imagem")
    time.sleep(1)