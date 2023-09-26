#ifndef REMOTE_AI_CLIENT
#define REMOTE_AI_CLIENT

#define LWIP_IPV4 1
#define LWIP_IPV6 1

#include <WiFi.h>
#include <WiFiMulti.h>
#include <AsyncTCP.h>
#include <map>
#include "timer.hpp"
#include "functionality.hpp"
#include "data_context.hpp"

using ok_func = void(*)(String const &a_msg);

class RemoteAIClient{
using response_func = bool(RemoteAIClient*, String &);

public:
    RemoteAIClient() = default;
    ~RemoteAIClient() = default;
    RemoteAIClient(const char *a_ssid, const char *a_pass, ok_func a_signal_ok);

    void begin();
    void connect_host(const char *host, const uint16_t port);
    void connect_host(IPAddress const &a_ip, const uint16_t a_port);
    void reconnect_host();
    void addAP(const char *a_ssid, const char *a_pass);

    void add_topic(String const &a_topic, Functionality *a_func = nullptr, bool a_is_first_time = true);
    void remove_topic(String const &a_topic);
    void topic_loader(std::vector<String> &a_topics, std::vector<Functionality> &a_func_vector);
    void act_upon_topic(String const &a_line);

    void publish(const char *buf, size_t size);
    void publish(String const &a_stream);
    void publish_two(const char *a_msg, size_t a_len, const char *b_msg, size_t b_len);
    void send_as_is(unsigned char *a_msg, unsigned long a_len);

    static void subscribed_ok(String const &a_msg);
    static void data_handler(void *a_context, AsyncClient *a_client, void *a_data, size_t len);
    static void error_handler(void *a_context, AsyncClient *a_client, int8_t error);
    static void connect_handler(void *a_context, AsyncClient *a_client);
    
    size_t topics_size() const;
    const char *rv() const;
    
private:
    bool is_word_contained(String const&a_msg, String const &a_topic);
    void resubscribe_topics();
    void check_connection();
    static size_t pre_msg(unsigned long a_len, char *pre);
    void write_pre_msg(unsigned long a_len);
    void initialize_client();

private:
    WiFiMulti m_wifimulti;
    AsyncClient m_client;
    std::map<String, Functionality*> m_topics;
    IPAddress m_host;
    uint16_t m_port;
    ok_func m_signal_ok;
    DataContext m_topics_context;
};


#endif // REMOTE_AI_CLIENT

