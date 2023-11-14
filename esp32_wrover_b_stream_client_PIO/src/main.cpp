#include <vector>
#include "remote_ai_client.hpp"
#include "camera_manager.hpp"
#include "vehicle_driver.hpp"
#include "communication_manager.hpp"
#include "functionality.hpp"
#include "servo_handler.hpp"

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "driver/rtc_io.h"

CameraManager camera;
TwoWire i2c_car = TwoWire(0);
CommunicationManager cm(i2c_car);
Vehicle car{cm};

void ok_drive()
{
  //Serial.println("ok_drive");
  car.forward(1024);
  yield();
  delay(2);
  car.forward(1024);
  yield();
  delay(2);
  car.forward(1024);
  yield();
  delay(2);
  car.forward(1024);
}

void stop_drive()
{
  car.stop();
  yield();
  delay(2);
  car.stop();
  yield();
  delay(2);
  car.stop();
  yield();
  delay(2);
  car.stop();
}


RemoteAIClient rai_client{"home", "0522945054", RemoteAIClient::subscribed_ok};
std::vector<String> topics = {"askstream", "askstopstream", "go_forward", "stop"};
std::vector<Functionality> actions{{CameraManager::ask_stream},{CameraManager::ask_stop_stream}, {ok_drive}, {stop_drive}};

void send_img(char *a_img, unsigned long a_len)
{
  //rai_client.publish_two(rai_client.rv(), 15, a_img, a_len);
    const char *c = "/pub";
    rai_client.publish_three(rai_client.rv(), 20, a_img, a_len, c, 4);

}

void setup()
{
  //Serial.begin(115200);
  //WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  //Serial.setDebugOutput(true);
  cm.begin();
  camera.begin();
  rai_client.begin();
  rai_client.connect_host("10.0.0.44", 3000);
  rai_client.topic_loader(topics, actions);
}

void loop() {
  camera.capture_image(send_img);
  delay(40);
}