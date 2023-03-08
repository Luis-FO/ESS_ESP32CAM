from tkinter import *
from tkinter import ttk
from PIL import Image, ImageTk
import paho.mqtt.client as mqtt
import numpy as np
import cv2
from ov2640_settings import Camera_Settings

# set_exposure_ctrl()
# set_aec_value()
# sensor_t * s = esp_camera_sensor_get()

# s->set_brightness(s, 0);     // -2 to 2
# s->set_contrast(s, 0);       // -2 to 2
# s->set_saturation(s, 0);     // -2 to 2
# s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
# s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
# s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
# s->set_wb_mode(s, 0);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
# s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
# s->set_aec2(s, 0);           // 0 = disable , 1 = enable
# s->set_ae_level(s, 0);       // -2 to 2
# s->set_aec_value(s, 300);    // 0 to 1200
# s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
# s->set_agc_gain(s, 0);       // 0 to 30
# s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
# s->set_bpc(s, 0);            // 0 = disable , 1 = enable
# s->set_wpc(s, 1);            // 0 = disable , 1 = enable
# s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
# s->set_lenc(s, 1);           // 0 = disable , 1 = enable
# s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
# s->set_vflip(s, 0);          // 0 = disable , 1 = enable
# s->set_dcw(s, 1);            // 0 = disable , 1 = enable
# s->set_colorbar(s, 0);       // 0 = disable , 1 = enable

#convert array to PiL Image    


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
        
        # Barra deslizante para configuração da exposição
        self.label_exposure = ttk.Label(self.Frame_painel, text = "Exposição(Tempo?)")
        self.label_exposure.grid()

        self.aec_value = IntVar()
        self.exposure_value_slider = ttk.Scale(self.Frame_painel, variable = self.aec_value ,from_= 0, to = 1200, orient = HORIZONTAL)
        self.exposure_value_slider.grid()

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
        self.image = ImageTk.PhotoImage(Image.open(r"src\python_code\MQTT_CLIENT_SUB\Imagens\g1.jpg"))  

        self.label_image = Label(self.Frame_image, image = self.image)
        self.label_image.grid(column=0, row=0)

    def set_configs(self):
        self.read_form()
        self.publica("topic/cam_config",self.cam_conf.to_json())

    def read_form(self):
        self.cam_conf.vflip = int(self.vflip_state.get())
        self.cam_conf._aec_value = int(self.aec_value.get())
        self.cam_conf._exposure_ctrl = 0


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


