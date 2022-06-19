import paho.mqtt.client as mqtt

class mqtt_sub(mqtt.Client):

    def on_connect(self, mqttc, obj, flags, rc):
        print("rc: "+str(rc))

    def on_connect_fail(self, mqttc, obj):
        print("Connect failed")

    def on_message(self, mqttc, obj, msg):
        """"Recebe e processa os dados"""
        #paridade = verify(msg.payload)
        print(msg.topic+" "+str(msg.qos)+" "+str(msg.payload))
        self.publish("topic/resp", "recebido", 1)

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

def verify(num):
    if num%2 == 0:
        return "Par"
    else:
        return "Impar"


if __name__ == "__main__":
    
    mqttc = mqtt_sub()
    mqttc.username_pw_set(username="luis", password="DMK178qtS")
    rc = mqttc.run()
    print("rc: "+str(rc))