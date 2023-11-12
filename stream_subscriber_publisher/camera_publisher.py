import cv2
import time

import numpy as np

from data_worker import *
import random
HOST = "10.0.0.44"  # The server's hostname or IP address
PORT = 3000  # The port used by the server

def rand_str():
    out = str()
    for i in range(0, (random.randint(10, 50))):
        out += chr(random.randint(97, 122))
    return out

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.settimeout(1)
    s.connect((HOST, PORT))
    cap = cv2.VideoCapture(1)
    if not cap.isOpened():
        print("Cannot open camera")
        exit()
    while True:
        # Read each frame from the webcam
        ret, frame = cap.read()
        if not ret:
            print("Can't receive frame (stream end?). Exiting ...")
            break
        frame = cv2.resize(frame, (320, 240))
        encode_param = [int(cv2.IMWRITE_JPEG_QUALITY), 60]
        img_jpg = cv2.imencode(".jpg", frame, encode_param)
        img_jpg = np.frombuffer(img_jpg[1], dtype=np.uint8, count=-1)
        if frame.size == 0:
            continue
        send_img(s, img_jpg, "PUB/robot_view<|||>", "/pub")
        time.sleep(1/24)
        try:
            data = s.recv(500)
        except:
            continue
        cv2.waitKey(5)
