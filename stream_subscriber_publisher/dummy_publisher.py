import time
from data_worker import *
import random
HOST = "10.0.0.44"  # The server's hostname or IP address
PORT = 3000  # The port used by the server

def rand_str():
    out = str()
    for i in range(0, (random.randint(10, 50))):
        out += chr(random.randint(97, 122))
    return out

topics = {"one", "two", "three"}
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    num = 0
    while True:
        for t in topics:
            str_msg = "PUB/" + t + "<|||>" + str(num) + "/pub\n"
            send_msg(s, str_msg)
            data = s.recv(500)
            print(data)
            num += 1
            time.sleep(0.5)
