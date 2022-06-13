from this import d
import paho.mqtt.client as mqtt

class mqtt_sub(mqtt.Client):

    def on_connect(self, mqttc, obj, flags, rc):
        print("rc: "+str(rc))

    def on_connect_fail(self, mqttc, obj):
        print("Connect failed")

    def on_message(self, mqttc, obj, msg):
        print(msg.topic+" "+str(msg.qos)+" "+str(msg.payload))

    def on_publish(self, mqttc, obj, mid):
        print("mid: "+str(mid))

    def on_subscribe(self, mqttc, obj, mid, granted_qos):
        print("Subscribed: "+str(mid)+" "+str(granted_qos))

    def on_log(self, mqttc, obj, level, string):
        print(string)

    def run(self):
        self.connect("mqtt.eclipseprojects.io", 1883, 60)
        self.subscribe("TEMPERATURE", 0)

        rc = 0
        while rc == 0:
            rc = self.loop_forever()
        #print('s')
        return rc

if __name__ == "__main__":
    mqttc = mqtt_sub()
    rc = mqttc.run()
    print("rc: "+str(rc))