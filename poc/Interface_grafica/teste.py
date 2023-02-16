from tkinter import *
import os

from PIL import ImageTk, Image

class IG():
    """Classe para criar e manipular os recursos da interface gráfica"""

    def __init__(self, w_size: str, icon_path: str) -> None:
        """Intancia a interface e inicializa as principais variáveis para manipular o objeto"""
        self.W_Handle = Tk()
        self.res_x, self.res_y = [int(i) for i in w_size.split("x")]
        self.W_Handle.title("WIN_CONFIG")
        self.W_Handle.geometry(w_size)
        icon_path = PhotoImage(file = icon_path)
        self.W_Handle.iconphoto(False, icon_path)
        self.W_Handle.iconposition(x=10, y=10)
        self.img = None
        self.canvas = None

    def img_box(self, dir):
        x = 133
        y = 100
        img_small = Image.open(dir)
        img_small = img_small.resize((x, y))
        self.img = ImageTk.PhotoImage(img_small)
        self.canvas = Canvas(self.W_Handle, width=x, height=y)
        self.canvas.place(x=0,y=0)
        self.canvas.create_image(10, 10, anchor = NW, image=self.img)

    def run(self):
        a = r"C:\Users\LuisF\Desktop\6.png"
        self.img_box(a)
        self.W_Handle.mainloop()
        

    

# load= Image.open("/Users/omprakash/Desktop/Gmail-new-logo.jpg")
# render = ImageTk.PhotoImage(load)
# img = Label(root, image=render)
# img.place(x=100, y=100)


if __name__ == '__main__':
    icon = r'C:\Users\LuisF\Desktop\TCC\ESS_ESP32CAM\poc\Interface_grafica\IF_ICONE.png'
    w_size = "1080x720"
    mainAPP = IG(w_size=w_size, icon_path=icon)
    mainAPP.run()