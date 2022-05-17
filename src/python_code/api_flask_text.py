from flask import Flask, request, Response
import numpy as np
import cv2

# Inicializa a aplicação flask
app = Flask(__name__)

# Rotea posts http para este método
@app.route('/api/test', methods=['POST'])
def server():
    r = request
    # Converte a string da imagem para uint8
    nparr = np.frombuffer(r.data, np.uint8)
    # Decodifica a imagem e converte para grayscale
    img = cv2.imdecode(nparr, cv2.IMREAD_GRAYSCALE)
    # Armazena a média de intensidades da imagem recebida
    mean = np.mean(img)
    # Insere a média em uma string
    response = f'{mean}'
    # Envia a resposta para o cliente
    return Response(response=response, status=200, mimetype="text/plain")


# Executa a aplicação flask
app.run(host="0.0.0.0", port=5000)