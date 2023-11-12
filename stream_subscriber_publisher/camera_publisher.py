import cv2
import time

import numpy as np

from data_worker import *
import random
HOST = "10.0.0.44"  # The server's hostname or IP address
PORT = 3000  # The port used by the server
is_stream = False
def rand_str():
    out = str()
    for i in range(0, (random.randint(10, 50))):
        out += chr(random.randint(97, 122))
    return out

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.settimeout(1)
    s.connect((HOST, PORT))
    subscribe_topic(s, "askstrream")
    while not is_stream:
        data = try_recieve(s, 500)
        if "askstream" in str(data):
            is_stream = True
        else:
            print("waiting")
    cap = open_video_device(1)
    while True:
        img_jpg = get_image(cap)
        send_img(s, img_jpg, "PUB/robot_view<|||>", "/pub")
        time.sleep(1/24)
        data = try_recieve(s, 500)
        cv2.waitKey(5)
