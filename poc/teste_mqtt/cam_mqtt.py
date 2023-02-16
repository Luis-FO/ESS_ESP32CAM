import os

import paho.mqtt.client as mqtt
import cv2
import numpy as np
from multiprocessing import Value
from time import sleep
import json

counter = Value('i', 0)

class mqtt_sub(mqtt.Client):
    
    def __init__(self):
        super().__init__()
        self.state = 0

    def on_message(self, mqttc, obj, msg):
        """"Recebe e processa os dados"""
        #paridade = verify(msg.payload)
        #print("aqui")
        #print(msg.topic+" "+str(msg.qos)+" "+str(msg.payload))
        if msg.topic == 'topic/config':
            content_json = str(msg.payload.decode('utf-8').replace("'", '"'))[:]
            json.loads(content_json)
            print(content_json)

            
    def on_publish(self, mqttc, obj, mid):
        print("mid: "+str(mid))

    def on_subscribe(self, mqttc, obj, mid, granted_qos):
        print("Subscribed: "+str(mid)+" "+str(granted_qos))

    def on_log(self, mqttc, obj, level, string):
        print(string)

    def run(self):
        """ Conecta e se inscreve no tópico imagem
        Broker gratuito online:
        self.connect("mqtt.eclipseprojects.io", 1883, 60)
        Broker local:
        self.connect("<Endereço IPv4 da máquina>", <porta>, 60)
        """
        self.connect("192.168.15.12", 1883, 60)
        self.subscribe("topic/response", 1)# mover para def on_connect e verificar se funciona igual
        self.subscribe("topic/config", 1)
        rc = 0
        while rc == 0:
            rc = self.loop_forever()
        return rc

    # def capture(self):
    #     while True:
    #         if self.state == 1:
    #             sleep(2)
    #             self.mqttc.publish("topic/resp", "imagem", 1)
        
        
def processing(img):
    img_dir = r"C:\Users\LuisF\Desktop\TCC\Sincronia_FreeRTOS_mqtt"
    if not os.path.isdir(img_dir):
        os.mkdir(img_dir)
    counter.value += 1
    count = counter.value       
    cv2.imwrite(os.path.join(img_dir,"img_"+str(count)+".jpeg"), img)
    # ax.imshow(img)
    # plt.show()
    # Armazena a média de intensidades da imagem recebida
    mean = np.mean(img)
    # Insere a média em uma string
    response = f'{mean}'
    return response


def verify(num):
    """test function just for receiving integer number"""
    if num%2 == 0:
        return "Par"
    else:
        return "Impar"


if __name__ == "__main__":

    mqttc = mqtt_sub()
    mqttc.username_pw_set(username="luis", password="DMK178qtS")
    rc = mqttc.run()
    print("rc: "+str(rc))
    