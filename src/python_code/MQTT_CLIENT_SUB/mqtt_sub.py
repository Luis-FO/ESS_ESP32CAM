import paho.mqtt.client as mqtt
import cv2
import numpy as np
from multiprocessing import Value

counter = Value('i', 0)

class mqtt_sub(mqtt.Client):

    def __init__(self, client_id: str | None = ..., clean_session: bool | None = ..., userdata: _UserData | None = ..., protocol: int = ..., transport: str = ..., reconnect_on_failure: bool = ...) -> None:
        super().__init__(client_id, clean_session, userdata, protocol, transport, reconnect_on_failure)
        #self.data_receive = 0
        
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
        self.connect("192.168.15.12", 1883, 60)
        self.subscribe("topic/img", 2)

        rc = 0
        while rc == 0:
            rc = self.loop_forever()
        #print('s')
        return rc

def save_img(img):
	with counter.get_lock():
		counter.value += 1
		count = counter.value
	img_dir = r"C:\Users\LuisF\Desktop\TCC\T1_HTTP_V1_segmentos_CIF_20mhz"
	if not os.path.isdir(img_dir):
		os.mkdir(img_dir)
	cv2.imwrite(os.path.join(img_dir,"img_"+str(count)+".jpeg"), img)

if __name__ == "__main__":
    mqttc = mqtt_sub("SUB")
    mqttc.username_pw_set(username="luis", password="DMK178qtS")
    rc = mqttc.run()
    print("rc: "+str(rc))