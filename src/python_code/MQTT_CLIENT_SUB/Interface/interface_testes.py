from tkinter import *
from tkinter import ttk
from PIL import Image, ImageTk
import paho.mqtt.client as mqtt
import numpy as np

class InterfaceComando():

    def __init__(self) -> None:
        
        # Cliente MQTT
        self.cliente = mqtt.Client()
        self.cliente.username_pw_set(username="luis", password="DMK178qtS")
        self.cliente.on_message = self.mensagem
        self.root = Tk()
        self.root.resizable()

        # Ttk Frame widget is a container, used to group other widgets together
        self.Frame_painel = ttk.Frame(self.root, padding=10)

        self.frm_img_camera.grid()
        self.Frame_painel.grid()

        # Cria uma label com texto e posicionada dentro do frame "frm"
        self.Frame_ = ttk.Frame(self.root, padding=10)
        self.img1 = ImageTk.PhotoImage(Image.open(r"src\python_code\MQTT_CLIENT_SUB\Imagens\g1.jpg")) 

        self.num_img = 0
        self.label_image = ttk.Label(self.frm_img_camera, image = self.img1)
        self.label_image.grid(column=0, row=0)
        
        # Botão que aciona o método "destroy" da intancia root (Nossa janela)
        ttk.Button(self.Frame_painel, text="Captura (Not Working)", command=self.captura).grid(column=0, row=0)
        ttk.Button(self.Frame_painel, text="Quit", command=self.root.destroy).grid(column=1, row=0)
    
    def run(self):
        self.run_mqtt()
        self.root.mainloop()

    def mensagem(self, mqttc, obj, msg):
        img = self.convert_to_array(msg.payload)
        self.exibe(img)
    
    def run_mqtt(self):
        """ Conecta e se inscreve no tópico imagem
        Broker gratuito online:
        self.connect("mqtt.eclipseprojects.io", 1883, 60)
        Broker local:
        self.connect("<Endereço IPv4 da máquina>", <porta>, 60)
        """
        self.cliente.connect("192.168.15.12", 1883, 60)
        self.cliente.subscribe("topic/img", 1) # mover para def on_connect e verificar se funciona igual
        self.cliente.loop_start()

    def captura(self):
        # self.img1 = ImageTk.PhotoImage(Image.open("src\python_code\MQTT_CLIENT_SUB\Imagens\Gato3.jpg"))
        # self.label_image.config(image = self.img1)
        self.cliente.publish("topic/captura", '1',1)
        print("Captura")

    def exibe(self, img):
        # self.img1 = ImageTk.PhotoImage(Image.open("src\python_code\MQTT_CLIENT_SUB\Imagens\Gato3.jpg"))
        # self.label_image.config(image = self.img1)
        print(img)

    def convert_to_array(self, data):
        # nparr = np.frombuffer(data, np.uint8)
        img = data
        # Decodifica a imagem e converte para grayscale
        # img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
        return img

if __name__ == "__main__":

    interface = InterfaceComando()
    interface.run()