import cv2
import numpy as np
import mediapipe as mp
import tensorflow as tf
import os
from keras.models import load_model

class HandGesturesController:
    def __init__(self):
        self.mpHands = mp.solutions.hands
        self.hands = self.mpHands.Hands(max_num_hands=1, min_detection_confidence=0.7)
        self.mpDraw = mp.solutions.drawing_utils
        os.path.join("./../HandGestures")
        # Load the gesture recognizer model
        self.model = load_model('./../HandGestures/mp_hand_gesture')
        f = open('./../HandGestures/gesture.names', 'r')
        self.classNames = f.read().split('\n')
        f.close()
        print(self.classNames)

    def get_gesture_in_frame(self, img, is_flip_needed = True):
        x, y, c = img.shape
        if is_flip_needed:
            img = cv2.flip(img, 1)
        frame_rgb = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        result = self.hands.process(frame_rgb)
        if result.multi_hand_landmarks:
            landmarks = []
            for hands_lms in result.multi_hand_landmarks:
                for lm in hands_lms.landmark:
                    # print(id, lm)
                    lmx = int(lm.x * x)
                    lmy = int(lm.y * y)
                    landmarks.append([lmx, lmy])
            prediction = self.model.predict([landmarks])
            classID = np.argmax(prediction)
            return self.classNames[classID]
        return None

    def get_gesture_in_frame_and_draw(self, img, is_flip_needed = True):
        x, y, c = img.shape
        if is_flip_needed:
            img = cv2.flip(img, 1)
        frame_rgb = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        result = self.hands.process(frame_rgb)
        if result.multi_hand_landmarks:
            landmarks = []
            for hands_lms in result.multi_hand_landmarks:
                for lm in hands_lms.landmark:
                    # print(id, lm)
                    lmx = int(lm.x * x)
                    lmy = int(lm.y * y)
                    landmarks.append([lmx, lmy])
            self.mpDraw.draw_landmarks(img, hands_lms, self.mpHands.HAND_CONNECTIONS)
            prediction = self.model.predict([landmarks])
            classID = np.argmax(prediction)
            cv2.imshow("Output", img)
            if cv2.waitKey(500) == ord('q'):
                return self.classNames[classID]
            return self.classNames[classID]
        return None

    def camera_stream_gestures(self, video_id):
        # Initialize the webcam for Hand Gesture Recognition Python project
        cap = cv2.VideoCapture(video_id)
        if not cap.isOpened():
            print("Cannot open camera")
            exit()
        while True:
            # Read each frame from the webcam
            ret, frame = cap.read()
            if not ret:
                print("Can't receive frame (stream end?). Exiting ...")
                break
            x , y, c = frame.shape

            # Flip the frame vertically
            frame = cv2.flip(frame, 1)
            # Show the final output
            framergb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            # Get hand landmark prediction
            result = self.hands.process(framergb)
            className = ''
            # post process the result
            if result.multi_hand_landmarks:
                landmarks = []
                for handslms in result.multi_hand_landmarks:
                    for lm in handslms.landmark:
                        # print(id, lm)
                        lmx = int(lm.x * x)
                        lmy = int(lm.y * y)

                        landmarks.append([lmx, lmy])

                # Drawing landmarks on frames
                self.mpDraw.draw_landmarks(frame, handslms, self.mpHands.HAND_CONNECTIONS)
                prediction = self.model.predict([landmarks])
                classID = np.argmax(prediction)
                className = self.classNames[classID]
                if className == 'thumbs up':
                    print(className)
            cv2.imshow("Output", frame)
            if cv2.waitKey(1) == ord('q'):
                break

                # release the webcam and destroy all active windows
            cap.release()
            cv2.destroyAllWindows()


#hgc = HandGesturesController()
#HandGesturesController.camera_stream_gestures(hgc, 1)