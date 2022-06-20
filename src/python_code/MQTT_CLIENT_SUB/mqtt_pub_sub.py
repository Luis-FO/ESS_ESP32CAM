import os

import paho.mqtt.client as mqtt
import cv2
import numpy as np
from multiprocessing import Value

counter = Value('i', 0)

class mqtt_sub(mqtt.Client):
        
    def on_message(self, mqttc, obj, msg):
        """"Recebe e processa os dados"""
        #paridade = verify(msg.payload)
        #print("aqui")
        #print(msg.topic+" "+str(msg.qos)+" "+str(msg.payload))
        _ , response = self.convert_to_array(msg.payload)
        self.publish("topic/resp", response, 1)

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
        self.subscribe("topic/img", 1)# mover para def on_connect e verificar se funciona igual
        rc = 0
        while rc == 0:
            rc = self.loop_forever()
        #print('s')
        return rc

    def convert_to_array(self, data):
        nparr = np.frombuffer(data, np.uint8)
        # Decodifica a imagem e converte para grayscale
        img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
        return img, processing(img)


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