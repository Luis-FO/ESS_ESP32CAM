import serial

def interface_serial(porta):
    # Abre a conexão com a porta serial
    serial_port = serial.Serial('COM3', 115200)

    # Espera por dados na porta serial
    while True:
        if serial_port.in_waiting == 0:
            pass
        else:
            dados_serial = serial_port.readline().decode().rstrip()
            print(dados_serial)
            if dados_serial == "go":
                # Lê os três valores inteiros
                valor1 = int(input("aec_value: "))
                valor2 = int(input("agc_gain: "))
                valor3 = int(input("Digite o terceiro valor: "))

                # Escreve os valores na porta serial
                serial_port.write((str(valor1) + "-" + str(valor2) + "-" + str(valor3)).encode())

                # Fecha a conexão com a porta serial
                serial_port.close()
                return 0
            

if __name__ == "__main__":
    p = 'COM3'
    interface_serial(p)