import socket
import cv2
import numpy as np
import io
from PIL import Image
def zeroes_pre(num, digits):
    string_num = str(num+4+digits)
    needed = digits-len(string_num)
    return ("0"*needed)+string_num


def send_msg(s:socket, msg):
    size = len(msg)
    b = bytearray()
    m = zeroes_pre(size,5)
    b.extend(map(ord, m))
    pre = b">>"+b+b"<<"+msg
    s.sendall(pre)


def parse_to_img(img_data):
    print(img_data)
    try:
        #im = img_data.reshape(120, 160, 2)
        #rgb = cv2.cvtColor(im, cv2.COLOR_BGR5652BGR)
        image = Image.open(io.BytesIO(img_data))
        open_cv_image = np.array(image)
        cv2.imshow("t", open_cv_image)
        cv2.waitKey(5)
        return True
    except Exception as error:
        print(error)
        return False