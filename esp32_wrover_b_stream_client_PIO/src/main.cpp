#include <vector>
#include "remote_ai_client.hpp"
#include "camera_manager.hpp"
#include "functionality.hpp"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "driver/rtc_io.h"

CameraManager camera;
RemoteAIClient rai_client{"home", "0522945054", RemoteAIClient::subscribed_ok};
std::vector<String> topics = {"askstream", "askstopstream"};
std::vector<Functionality> actions{{CameraManager::ask_stream},{CameraManager::ask_stop_stream}};

void send_img(char *a_img, unsigned long a_len)
{
  rai_client.publish_two(rai_client.rv(), 12, a_img, a_len);
}

void setup()
{
  Serial.begin(115200);
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.setDebugOutput(true);

  camera.begin();
  rai_client.begin();
  rai_client.connect_host("10.0.0.38", 3000);
  rai_client.topic_loader(topics, actions);
}

void loop() {
  camera.capture_image(send_img);
  delay(40);
}