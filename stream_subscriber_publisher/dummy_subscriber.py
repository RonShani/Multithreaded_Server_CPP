from hand_gestures import *
from data_worker import *
from PIL import Image
from io import BytesIO
HOST = "10.0.0.44"  # The server's hostname or IP address
PORT = 3000  # The port used by the server
hg = HandGesturesController()

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.settimeout(1)
    s.connect((HOST, PORT))
    img = bytes()
    publisher_topic(s, "askstream", "")
    try_recieve(s, 500)
    send_msg(s,"SUB/robot_view/sub")
    while True:
        data = try_recieve(s, 10000)
        if data is not None:
            img = try_parse_jpg(data, True)
            if img is not None:
                gest = hg.get_gesture_in_frame(img)
                if gest == 'fist':
                    publisher_topic(s, "go_forward", "")
                elif gest == 'stop':
                    publisher_topic(s, "stop", "")
                print(gest)

        """
            try:
                parse_to_img(data[15:])
            except:
                print(data[15:])
        """