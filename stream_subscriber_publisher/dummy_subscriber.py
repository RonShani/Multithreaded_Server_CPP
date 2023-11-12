#from hand_gestures import *
from data_worker import *
from PIL import Image
from io import BytesIO
HOST = "10.0.0.44"  # The server's hostname or IP address
PORT = 3000  # The port used by the server

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.settimeout(1)
    s.connect((HOST, PORT))
    img = bytes()
    send_msg(s,"SUB/robot_view/sub")
    while True:
        try:
            data = s.recv(10000)
        except:
            continue
        try:
            parse_to_img(data[15:])
        except:
            print(data[15:])
