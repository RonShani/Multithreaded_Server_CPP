#ifndef CAMERA_MANAGER_HPP
#define CAMERA_MANAGER_HPP
#define CAMERA_MODEL_AI_THINKER

#include "esp_camera.h"
#include "camera_pins.h"

template <class Context>
using img_act = void(Context&, char *, unsigned long);

using local_action = void(*)(char *, unsigned long);

class CameraManager{
public:
    CameraManager();
    ~CameraManager() = default;

    void capture_image(local_action a_action);

    template <class Context>
    void capture_image(img_act<Context>, Context &a_context);

    void begin();
    void startCameraServer();
    static void ask_stream();
    static void ask_stop_stream();

private:
    camera_config_t config;
    static bool m_is_stream;
};

template <class Context>
void CameraManager::capture_image(img_act<Context> a_action, Context &a_context)
{
  if(not CameraManager::m_is_stream){
    return;
  }
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  if (!fb) {
      return;
  }
  a_action(a_context, fb->buf, fb->len);
  esp_camera_fb_return(fb);   
}

#endif // CAMERA_MANAGER_HPP