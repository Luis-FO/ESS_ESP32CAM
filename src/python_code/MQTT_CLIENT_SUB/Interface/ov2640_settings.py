import json

"""
Camera_Settings modela a configuração da câmera
Ela recebe como parâmetro a fonte de suas informações
"""
"""" Framesizes
FRAMESIZE_96X96":0,  96x96 
"FRAMESIZE_QQVGA":1,    160x120
"FRAMESIZE_QCIF":2,     176x144
"FRAMESIZE_HQVGA":3,    240x176
"FRAMESIZE_240X240":4,  240x240
"FRAMESIZE_QVGA":5,     320x240
"FRAMESIZE_CIF":6,      400x296
"FRAMESIZE_HVGA":7,     480x320
"FRAMESIZE_VGA":8,      640x480
"FRAMESIZE_SVGA":9,     800x600
"FRAMESIZE_XGA":10,     1024x768
"FRAMESIZE_HD":11,      1280x720
"FRAMESIZE_SXGA":12,    1280x1024
"FRAMESIZE_UXGA":13,    1600x1200
"""


class Camera_Settings:
    """
        Classe auxiliar que se responsabiliza pela leitura de formulários
    """
    frame_sizes = {"FRAMESIZE_96X96":0,    
        "FRAMESIZE_QQVGA":1,    
        "FRAMESIZE_QCIF":2,     
        "FRAMESIZE_HQVGA":3,    
        "FRAMESIZE_240X240":4,  
        "FRAMESIZE_QVGA":5,     
        "FRAMESIZE_CIF":6,      
        "FRAMESIZE_HVGA":7,     
        "FRAMESIZE_VGA":8,      
        "FRAMESIZE_SVGA":9,     
        "FRAMESIZE_XGA":10,     
        "FRAMESIZE_HD":11,      
        "FRAMESIZE_SXGA":12,   
        "FRAMESIZE_UXGA":13}
    
    def __init__(self, brightness = 0, vflip = False) -> None:
        # Conectando widgets as variáveis
        
        self._quality = 10 # 0 to 63
        self._brightness = 0     # -2 to 2

        self._exposure_ctrl = 0  # 0 = disable , 1 = enable

        self._aec_value = 0    # 0 to 1200

        self._vflip = 0          # 0 = disable , 1 = enable

        # *init_status sensor_t sensor)
        # *reset sensor_t sensor) // Reset the configuration of the sensor, and return ESP_OK if reset is successful
        # self._pixformat  pixformat_t pixformat)
        self._framesize = self.frame_sizes["FRAMESIZE_CIF"]
        # self._contrast
        # self._saturation
        # self._sharpness
        # self._denoise
        # self._gainceiling
        # self._colorbar  
        # self._whitebal
        # self._hmirror
        # self._aec2
        # self._awb_gain
        self._gain_ctrl = 0
        self._agc_gain = 0 # 0-30
        # self._special_effect
        # self._wb_mode
        #self._ae_level
        # self._dcw  
        # self._bpc  
        # self._wpc  
        # self._raw_gma  
        # self._lenc  

    @property
    def framesize(self):
        return self._framesize
    
    @framesize.setter
    def framesize(self, value):
        self._framesize = self.frame_sizes[value]
    
    def to_dict(self):
        return {'vflip': self._vflip, \
                'exposure_ctrl': self._exposure_ctrl, \
                'aec_value': self._aec_value, \
                'agc_gain': self._agc_gain, \
                'framesize': self._framesize}
    
    def to_json(self):
        return json.dumps(self.to_dict())
    
    
    def __str__(self) -> str:
        atributos = self.to_dict()
        texto = ''
        for key, value in atributos.items():
            texto+= f'{key}: {value}\n'
        return texto

if __name__ == "__main__":

    cam_settings = Camera_Settings()
    #print(cam_settings.to_json())
    print(cam_settings)

    # print(cam_settings)