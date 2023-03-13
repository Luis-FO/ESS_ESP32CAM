import json

"""
Camera_Settings modela a configuração da câmera
Ela recebe como parâmetro a fonte de suas informações
"""
class Camera_Settings:
    """
        Classe auxiliar que se responsabiliza pela leitura de formulários
    """

    def __init__(self, brightness = 0, vflip = False) -> None:
        # Conectando widgets as variáveis

        # set_exposure_ctrl()
        # set_aec_value()
        # sensor_t * s = esp_camera_sensor_get()

        self._brightness = 0;     # -2 to 2
        # self._contrast;       # -2 to 2
        # self._saturation;     # -2 to 2
        # self._special_effect; # 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
        # self._whitebal;       # 0 = disable , 1 = enable
        # self._awb_gain;       # 0 = disable , 1 = enable
        # self._wb_mode;        # 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
        self._exposure_ctrl = 1;  # 0 = disable , 1 = enable
        # self._aec2;           # 0 = disable , 1 = enable
        # self._ae_level;       # -2 to 2
        self._aec_value = 0;    # 0 to 1200
        # self._gain_ctrl;      # 0 = disable , 1 = enable
        # self._agc_gain;       # 0 to 30
        # self._gainceiling;  # 0 to 6
        # self._bpc;            # 0 = disable , 1 = enable
        # self._wpc;            # 0 = disable , 1 = enable
        # self._raw_gma;        # 0 = disable , 1 = enable
        # self._lenc;           # 0 = disable , 1 = enable
        # self._hmirror;        # 0 = disable , 1 = enable
        self._vflip = 0;          # 0 = disable , 1 = enable
        # self._dcw;            # 0 = disable , 1 = enable
        # self._colorbar;       # 0 = disable , 1 = enable

        # self._aec2 = 0           # 0 = disable , 1 = enable
        # self._ae_level = 0       # -2 to 2

        # assossiate a function to a atribute
        # self._vflip = vflip        # 0 = disable , 1 = enable

    @property
    def vflip(self):
        return self._vflip
    
    @vflip.setter
    def vflip(self, value):
        """value: 0 = disable, 1 = enable"""
        self._vflip = value

    def to_dict(self):
        return {'vflip': self._vflip, \
                'exposure_ctrl': self._brightness, \
                'aec_value': self._aec_value, \
                'brightness': self._brightness }
    
    def to_json(self):
        return json.dumps(self.to_dict())
    
    
    def __str__(self) -> str:
        return f'Brightness: {self._brightness}\n'\
               f'Exposure_Ctrl: {self._exposure_ctrl}\n'\
               f'AEC_Value: {self._aec_value}\n'\
               f'Vflip: {self._vflip}\n'


if __name__ == "__main__":

    cam_settings = Camera_Settings()
    print(cam_settings.to_json())

    # print(cam_settings)