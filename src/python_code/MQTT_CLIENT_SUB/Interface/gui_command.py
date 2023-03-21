from tkinter import *
from tkinter import ttk
from PIL import Image, ImageTk
import paho.mqtt.client as mqtt
import numpy as np
import cv2
from ov2640_settings import Camera_Settings


class gui_ctrl:

    def __init__(self) -> None:
        # Configuração da câmera
        self.cam_conf = Camera_Settings()

        # Cliente MQTT
        self.cliente = mqtt.Client()
        self.cliente.username_pw_set(username="luis", password="DMK178qtS")
        self.cliente.on_message = self.mensagem
        
        # Janela
        self.root = Tk()
        self.root.title("Configurações da câmera")
        #self.root.geometry('600x600')
        self.root.resizable()

        # Painel de comandos
        self.Frame_painel = ttk.Frame(self.root, padding=10)
        self.Frame_painel.grid(row = 0, column = 0)
        
        # Flip check-button
        self.vflip_state = BooleanVar()
        self.vflip_checkbt = ttk.Checkbutton(self.Frame_painel, text = 'vflip', variable = self.vflip_state, onvalue=1, offvalue=0)
        self.vflip_checkbt.grid()
        
        # Exposure value
        self.label_exposure = ttk.Label(self.Frame_painel, text = "Exposição(Tempo?)")
        self.label_exposure.grid()

        self.aec_value = IntVar()
        self.exposure_value_slider = ttk.Scale(self.Frame_painel, variable = self.aec_value ,from_= 0, to = 1200, orient = HORIZONTAL)
        self.exposure_value_slider.grid()

        # Brilho
        self.label_gain = ttk.Label(self.Frame_painel, text = "Ganho")
        self.label_gain.grid()
        
        #Ganho
        self.gain = IntVar()
        self.gain_slider = ttk.Scale(self.Frame_painel, variable = self.gain ,from_= 0, to = 30, orient = HORIZONTAL)
        self.gain_slider.grid()

        # Framesize
        self._combobox = ttk.Combobox(self.Frame_painel)
        self._combobox['values'] = list(Camera_Settings.frame_sizes.keys())
        self._combobox.current(self.cam_conf.framesize)
        self._combobox.grid()

        ################################# Botões #####################################
        # Botão para Envio de Configurações 
        self.set_config = ttk.Button(self.Frame_painel, text = "Enviar Configuração", command=self.set_configs)
        self.set_config.grid()

        # Botão de captura
        self.capture = ttk.Button(self.Frame_painel, text="Captura", command=self.get_image_from_esp)
        self.capture.grid()

        
        # Botão "Quit"
        ttk.Button(self.Frame_painel, text="Quit", command=self.root.destroy).grid()
        
        ############################### Imagem #######################################
        # Imagem da câmera
        self.Frame_image = ttk.Frame(self.root, padding=1)
        self.Frame_image.grid(row = 0, column = 1)
        
        # Cria uma label com texto e posicionada dentro do frame "frm"
        self.image = ImageTk.PhotoImage(Image.open(r"src\python_code\MQTT_CLIENT_SUB\Imagens\florianopolis_horizontal_marca2015_PNG.png"))  

        self.label_image = Label(self.Frame_image, image = self.image)
        self.label_image.grid(column=0, row=0)

    def set_configs(self):
        self.read_form()
        self.publica("topic/cam_config",self.cam_conf.to_json())

    def read_form(self):
        self.cam_conf._vflip = int(self.vflip_state.get())
        self.cam_conf._aec_value = int(self.aec_value.get())
        self.cam_conf._exposure_ctrl = 0
        self.cam_conf._agc_gain = int(self.gain.get())
        self.cam_conf.framesize = self._combobox.get()
        print(self.cam_conf)

    def display_image(self, new_image):

        PIL_image = Image.fromarray(np.uint8(new_image)).convert('RGB')
        self.image = ImageTk.PhotoImage(PIL_image)
        # label1 = ttk.Label(image=tk_image )
        #self.img1 = ImageTk.PhotoImage(Image.open("src\python_code\MQTT_CLIENT_SUB\Imagens\Gato3.jpg"))
        self.label_image.config(image = self.image)
    
    def mensagem(self, mqttc, obj, msg):
        # print('Mensagem Recebida')
        img = self.convert_to_array(msg.payload)
        self.display_image(new_image = img)

    def convert_to_array(self, data):
        nparr = np.frombuffer(data, np.uint8)
        # Decodifica a imagem e converte para grayscale
        img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
        return img
    
    def get_image_from_esp(self):
        self.publica("topic/get_image", 'get_image')

    def run(self):
        self.run_client()
        self.root.mainloop()
    
    def run_client(self):
        """ Conecta e se inscreve no tópico imagem
        Broker gratuito online:
        self.connect("mqtt.eclipseprojects.io", 1883, 60)
        Broker local:
        self.connect("<Endereço IPv4 da máquina>", <porta>, 60)
        """
        self.cliente.connect("192.168.15.12", 1883, 60)
        self.cliente.subscribe("topic/img", 1)# mover para def on_connect e verificar se funciona igual
        self.cliente.loop_start()
    
    def publica(self, topic, data):
        self.cliente.publish(topic, data, 1)

if __name__ == "__main__":
    #settings = Camera_Settings()
    interface = gui_ctrl()
    interface.run()


