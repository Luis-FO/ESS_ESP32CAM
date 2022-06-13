import paho.mqtt.publish as publish
# Broker
MQTT_SERVER = "broker.emqx.io" 
# Tópico 
MQTT_PATH = "Image" 

path = r"C:\Users\LuisF\Desktop\TCC\ESS_ESP32CAM\poc\mqtt_python\mqtt\Lena.jpg"
# Abre o arquivo no modo binário
f = open(path, "rb")
# Lê o arquivo e tranforma em bytearray
img = f.read()
byteArr = bytearray(img)
for x in range(30):
    publish.single(MQTT_PATH, byteArr, hostname=MQTT_SERVER)
