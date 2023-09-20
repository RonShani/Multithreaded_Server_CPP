#ifndef REMOTE_AI_CLIENT
#define REMOTE_AI_CLIENT

#include <WiFi.h>
#include <WiFiMulti.h>
#include <map>
#include "timer.hpp"
#include "functionality.hpp"

using ok_func = void(*)(String const &a_msg);

class RemoteAIClient{
using response_func = bool(RemoteAIClient*, String &);

public:
    RemoteAIClient() = default;
    ~RemoteAIClient() = default;
    RemoteAIClient(const char *a_ssid, const char *a_pass, ok_func a_signal_ok);

    void begin();
    void connect_host(const char *host, const uint16_t port);
    void addAP(const char *a_ssid, const char *a_pass);

    void add_topic(String const &a_topic, Functionality *a_func = nullptr, bool a_is_first_time = true);
    void remove_topic(String const &a_topic);
    void topic_loader(std::vector<String> &a_topics, std::vector<Functionality> &a_func_vector);

    static void send_img(RemoteAIClient &a_client, unsigned char *a_img, unsigned long a_len);
    static void publish_two(WiFiClient &a_client, const uint8_t *a_msg, size_t a_len, const uint8_t *b_msg, size_t b_len);
    static void subscribed_ok(String const &a_msg);

    bool act_upon_topic(String &a_line);
    bool validate_subscription(String const &a_line, String const &a_topic);
    bool response_action(int a_recieve_timeout, bool a_is_validate_sub = false, String const &a_context = {});
    
    void publish(String const &a_topic, const char *img, unsigned long);
    void publish(const uint8_t *buf, size_t size);
    void publish(String const &a_stream);
    void publish_two(const uint8_t *a_msg, size_t a_len, const uint8_t *b_msg, size_t b_len);
    void send_as_is(unsigned char *a_msg, unsigned long a_len);
    
    size_t topics_size() const;
    const unsigned char *rv() const;
    
private:  
    bool is_topic(String const&a_topic, String &a_msg);
    size_t find_shtrudel(String const &a_msg) const;
    bool is_word_contained(String const&a_msg, String const &a_topic);
    void resubscribe_topics();
    void check_connection();
    bool recieve_timeout(int a_timeout);
    bool read_income(String &a_income);
    static size_t pre_msg(unsigned long a_len, char *pre);

private:
    WiFiMulti m_wifimulti;
    WiFiClient m_client;
    std::map<String, Functionality*> m_topics;
    Timer m_timer;
    char m_host[20];
    uint16_t m_port;
    ok_func m_signal_ok;
    const unsigned char *m_rv = (const unsigned char*)"@robot_view|";
};


#endif // REMOTE_AI_CLIENT

