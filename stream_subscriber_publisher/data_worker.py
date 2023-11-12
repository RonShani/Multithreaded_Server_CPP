import socket
import cv2
import numpy
import numpy as np
import io
from PIL import Image
def zeroes_pre(num, digits):
    string_num = str(num+4+digits)
    needed = digits-len(string_num)
    return ("0"*needed)+string_num

def str_to_bytes(str_msg):
    b = bytearray()
    b.extend(map(ord, str_msg))
    return b
def send_msg(s:socket, msg):
    size = len(msg)
    b = bytearray()
    m = zeroes_pre(size,5)
    b.extend(map(ord, m))
    pre = b">>"+b+b"<<"+str_to_bytes(msg)
    s.sendall(pre)


def send_img(s:socket, img:np.ubyte, pre:str, post:str):
    size = img.size + len(pre) + len(post)
    print("size:",size)
    b = bytearray()
    m = zeroes_pre(size,5)
    b.extend(map(ord, m))
    pre = b">>"+b+b"<<"+str_to_bytes(pre)+img.tobytes()+str_to_bytes(post)
    s.sendall(pre)

def send_msg_pre_bytes_post(s:socket, pre, bytes_arr_msg, post):
    bpre = str_to_bytes(pre)
    bpst = str_to_bytes(post)
    size = len(bytes_arr_msg) + len(bpre) + len(bpst)
    m = str_to_bytes(">>"+zeroes_pre(size,5)+"<<")
    whole = m+bpre+bytes_arr_msg+bpst
    print(whole)
    s.sendall(whole)

def parse_to_img_raw_rgb565(img_data):
    print(len(img_data))
    try:
        image = np.frombuffer(img_data, dtype=np.uint8)
        im = image.reshape(120, 160, 2)
        rgb = cv2.cvtColor(im, cv2.COLOR_BGR5652BGR)
        #image = Image.open(io.BytesIO(img_data))
        open_cv_image = np.array(rgb)
        cv2.imshow("t", open_cv_image)
        cv2.waitKey(50)
        return True
    except Exception as error:
        print(error)
        return False
def parse_to_img(img_data):
    print(len(img_data))
    try:
        image = Image.open(io.BytesIO(img_data))
        open_cv_image = np.array(image)
        open_cv_image = cv2.cvtColor(open_cv_image, cv2.COLOR_BGR2RGB)
        cv2.imshow("t", open_cv_image)
        cv2.waitKey(50)
        return open_cv_image
    except Exception as error:
        print(error)
        return None