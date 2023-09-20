HOST = "10.0.0.37"  # The server's hostname or IP address
PORT = 3000  # The port used by the server
from data_worker import *

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    img = bytes()
    send_msg(s,b"/sub/robot_view")
    data = s.recv(640 * 480 * 3)
    while True:
        data = s.recv(640 * 480 * 3)
        data = data[12:]
        try:
            parse_to_img(data)
        except Exception as error:
            print(error)