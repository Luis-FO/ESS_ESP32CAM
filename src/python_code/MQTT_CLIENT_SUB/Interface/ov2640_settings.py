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

        # self._brightness = brightness     # -2 to 2
        # self._contrast = 0       # -2 to 2
        # self._saturation = 0    # -2 to 2

        self._exposure_ctrl = 1 # 0 = disable , 1 = enable
        # self._aec2 = 0           # 0 = disable , 1 = enable
        # self._ae_level = 0       # -2 to 2
        self._aec_value = 0   # 0 to 1200

        # self._hmirror = 0               # 0 = disable , 1 = enable
        # assossiate a function to a atribute
        self._vflip = vflip        # 0 = disable , 1 = enable

    @property
    def vflip(self):
        return self._vflip
    
    @vflip.setter
    def vflip(self, value):
        """value: 0 = disable, 1 = enable"""
        self._vflip = value

    def to_dict(self):
        return {'vflip': self._vflip, 'aec_value': self._aec_value}
    
    def to_json(self):
        return json.dumps(self.to_dict())

if __name__ == "__main__":

    cam_settings = Camera_Settings()