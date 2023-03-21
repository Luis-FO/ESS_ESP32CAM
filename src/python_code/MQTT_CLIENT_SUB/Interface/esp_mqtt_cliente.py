from PIL import Image, ImageTk
import paho.mqtt.client as mqtt
import numpy as np
import cv2


class ESP_MQTT_Client():

    def __init__(self, username = 'luis', password = 'DMK178qtS') -> None:

        # Cliente MQTT
        self.cliente = mqtt.Client()
        self.cliente.username_pw_set(username="luis", password= password)
        self.cliente.on_message = self.msg

    def msg(self, mqttc, obj, msg):
        # print('Mensagem Recebida')
        return self.convert_to_array(msg.payload) 

    def convert_to_array(self, data):
        nparr = np.frombuffer(data, np.uint8)
        # Decodifica a imagem e converte para grayscale
        return cv2.imdecode(nparr, cv2.IMREAD_COLOR)
    
    def get_image_from_esp(self):
        # (topico, dado, qos(de 0 até 2)
        self.cliente.publish('topic/get_image', 'get_image', 1)

    def set_cam_config(self, configs):
        """Configs receive a json object"""
        # (topico, dado, qos(de 0 até 2)
        self.cliente.publish('topic/cam_config', configs, 1)

    def run(self):
        """ Conecta e se inscreve no tópico imagem
        Broker gratuito online:
        self.connect("mqtt.eclipseprojects.io", 1883, 60)
        Broker local:
        self.connect("<Endereço IPv4 da máquina>", <porta>, 60)
        """
        self.cliente.connect("192.168.15.12", 1883, 60)
        self.cliente.subscribe("topic/img", 1)# mover para def on_connect e verificar se funciona igual
        self.cliente.loop_start()
    


if __name__ == "__main__":
    #settings = Camera_Settings()
    cliente = ESP_MQTT_Client()
    cliente.run()


