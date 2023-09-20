import socket
import time
from data_worker import *
HOST = "10.0.0.37"  # The server's hostname or IP address
PORT = 3000  # The port used by the server

file = open("capture.jpg","rb")

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    img = b"@robot_view|" + bytearray(file.read())
    while True:
        send_msg(s, img)
        time.sleep(2)
