import time

from data_worker import *
HOST = "10.0.0.44"  # The server's hostname or IP address
PORT = 3000  # The port used by the server

topics = {"one", "two", "three"}
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.settimeout(1)
    s.connect((HOST, PORT))
    time.sleep(1)
    for t in topics:
        print("subscribing", t)
        sub_str = "SUB/"+t+"/sub\n"
        send_msg(s,sub_str)
        data = s.recv(1000)
        time.sleep(1)
    while True:
        try:
            data = s.recv(1024)
            print(data)
            data = ""
        except:
            continue
