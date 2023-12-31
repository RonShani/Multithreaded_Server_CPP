#include "camera_manager.hpp"
#include "esp_timer.h"
#include "esp_camera.h"
#include "img_converters.h"
#include "Arduino.h"
#define DEBUG_MODE

bool CameraManager::m_is_stream = false;

CameraManager::CameraManager()
: config{}
{}

void CameraManager::begin()
{
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
  config.jpeg_quality = 40;
  config.fb_count = 2;
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
#ifdef DEBUG_MODE
    Serial.printf("Camera init failed with error 0x%x", err);
#endif
    return;
  }
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_QVGA);
}

void CameraManager::ask_stream()
{
#ifdef DEBUG_MODE
  Serial.println("ask_stream started");
#endif
  m_is_stream = true;
}

void CameraManager::ask_stop_stream()
{
  m_is_stream = false;
}

void CameraManager::capture_image(local_action a_action)
{
  if(not CameraManager::m_is_stream){
    return;
  }
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
#ifdef DEBUG_MODE
  delay(15);
  if (!fb) {
    Serial.println("failed to get camera");
    return;
  }
  delay(15);
#endif
  a_action((char*)fb->buf, fb->len);
  esp_camera_fb_return(fb);
}
