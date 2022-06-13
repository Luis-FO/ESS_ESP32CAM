import paho.mqtt.client as mqtt

contador = 0



class cli_mqtt():
    def __init__(self, cliente):
        self.cont = 0
        # Broker mqtt
        self.MQTT_SERVER = "broker.emqx.io"
        # Tópico
        self.MQTT_PATH = "Image"
        self.cliente = cliente
        
    def on_connect(self, client, userdata, flags, rc):
        print("Connected with result code "+str(rc))
        client.subscribe(self.MQTT_PATH)

    def on_message(self, client, userdata, msg):
        
        path = f"output_{self.cont}.jpg"
        f = open(path, "wb")
        f.write(msg.payload)
        print("Image Received")
        f.close()
        self.cont+=1

if __name__ == "__main__":
    # Instancia o cliente 
    client = mqtt.Client()
    handle_mqtt = cli_mqtt(client)
    client.on_connect = handle_mqtt.on_connect
    client.on_message = handle_mqtt.on_message
    client.connect(handle_mqtt.MQTT_SERVER, 1883, 60)

    client.loop_forever()