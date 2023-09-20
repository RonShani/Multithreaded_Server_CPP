#include "remote_ai_client.hpp"
#include "functionality.hpp"

RemoteAIClient::RemoteAIClient(const char *a_ssid, const char *a_pass, ok_func a_signal_ok)
: m_wifimulti{}, m_client{}, m_topics{}, m_timer{}, m_host{}, m_port{0}, m_signal_ok(a_signal_ok)
{
    m_wifimulti.addAP(a_ssid, a_pass);

}

void RemoteAIClient::connect_host(const char *host, const uint16_t port)
{
    while (!m_client.connect(host, port)) {
        delay(10);
        yield();
    }
    m_client.setNoDelay(true);
    response_action(250);
    if(port != m_port){
        strcpy(m_host, host);
        m_port = port;
    }
    if(m_topics.size() > 0){
        resubscribe_topics();
    }
}

void RemoteAIClient::begin()
{
    while(m_wifimulti.run() != WL_CONNECTED) {
        delay(500);
    }
}

void RemoteAIClient::add_topic(String const &a_topic, Functionality *a_func, bool a_is_first_time)
{
    do{
        publish("/sub/"+a_topic);
    }while(!response_action(750, true, a_topic));
    if (a_is_first_time){
        m_topics[a_topic] = a_func;
    }
    
}

void RemoteAIClient::remove_topic(String const &a_topic)
{
    for(std::map<String, Functionality*>::const_iterator i = m_topics.cbegin(); i != m_topics.cend(); ++i){
        if(i->first.compareTo(a_topic) == 0){
            m_topics.erase(i);
            return;
        }
    }
}

bool RemoteAIClient::response_action(int a_recieve_timeout, bool a_is_validate_sub, String const &a_context)
{
    check_connection();
    if(recieve_timeout(a_recieve_timeout)) return  false;
    String line;
    if(not read_income(line)) return false;
    if(a_is_validate_sub){
        return validate_subscription(line, a_context);
    } else {
        return act_upon_topic(line);
    }
}

bool RemoteAIClient::is_topic(String const &a_topic, String &a_msg)
{
  return a_msg.startsWith(a_topic) || is_word_contained(a_msg, a_topic);
}

void RemoteAIClient::resubscribe_topics()
{
    for(auto &topic : m_topics){
        add_topic(topic.first, nullptr, false);
    }
}

void RemoteAIClient::topic_loader(std::vector<String> &a_topics, std::vector<Functionality> &a_func_vector)
{
    for(size_t i = 0; i < a_topics.size(); ++i){
        add_topic(a_topics[i], &a_func_vector[i]);
        m_signal_ok(a_topics[i]);
    }
}

void RemoteAIClient::publish(String const &a_topic, const char *a_data, unsigned long a_legnt)
{
    publish("@"+a_topic+"|"+String{a_data});
}

void RemoteAIClient::addAP(const char *a_ssid, const char *a_pass)
{
    m_wifimulti.addAP(a_ssid, a_pass);
}

size_t RemoteAIClient::topics_size() const
{
    return m_topics.size();
}

size_t RemoteAIClient::find_shtrudel(String const &a_msg) const
{
    for(size_t i = 0; i < a_msg.length(); ++i){
        if(a_msg[i] == '@'){
            return i;
        }
    }
    return a_msg.length();
}

bool RemoteAIClient::is_word_contained(String const &a_msg, String const &a_topic)
{
    size_t shtrudel = find_shtrudel(a_msg);
    if(shtrudel == a_msg.length()){
        return false;
    }
    size_t j = 0;
    for(size_t i = shtrudel+1; i < a_topic.length()+shtrudel; ++i){
        if(a_msg[i] == a_topic[j]){
            ++j;
        } else {
            return false;
        }
    }
    return true;
}

void RemoteAIClient::publish(const uint8_t *buf, size_t size)
{
    char pre[10] = ">>00000<<";
    pre_msg(size, pre);
    Serial.println(pre);
    m_client.write(pre, 10);
    m_client.write(buf, size);
}

void RemoteAIClient::publish_two(WiFiClient &a_client, const uint8_t *a_msg, size_t a_len, const uint8_t *b_msg, size_t b_len)
{
    char pre[10] = ">>00000<<";
    pre_msg(a_len+b_len, pre);
    Serial.println(pre);
    a_client.write(pre, 10);
    a_client.write(a_msg, a_len);
    a_client.write(b_msg, b_len);
}

void RemoteAIClient::publish(String const &a_string)
{
    char pre[10] = ">>00000<<";
    pre_msg(a_string.length(), pre);
    Serial.println(pre);
    m_client.write(pre, 10);
    m_client.write(a_string.c_str(), a_string.length());
}

void RemoteAIClient::send_as_is(unsigned char *a_msg, unsigned long a_len)
{
    m_client.write(a_msg, a_len);
}

const unsigned char *RemoteAIClient::rv() const
{
    return m_rv;
}

void RemoteAIClient::check_connection()
{
    if(m_client.connected() == 0){
        connect_host(m_host, m_port);
    }
}

bool RemoteAIClient::recieve_timeout(int a_timeout)
{
    Timer timer;
    while (m_client.available() == 0) {
      delay(1);
      yield();
      if(timer.is_passed_ms(a_timeout))return true;
    }
    return false;
}

bool RemoteAIClient::read_income(String &a_income)
{
    a_income = m_client.readStringUntil('\r');
    if(a_income.length() == 0){
        return false;
    }
    return true;
}

bool RemoteAIClient::act_upon_topic(String &a_line)
{
    for(auto &topic : m_topics){
      if(is_topic(topic.first, a_line)){
          topic.second->run();
          return true;
      }
    }
    return false;
}

bool RemoteAIClient::validate_subscription(String const &a_line, String const &a_topic)
{
    if(a_topic.compareTo(a_line) == 0){
      return true;
    } else {
      return false;
    }
}

void RemoteAIClient::send_img(RemoteAIClient &a_client, unsigned char *a_img, unsigned long a_len)
{
  publish_two(a_client.m_client, a_client.rv(),12, a_img, a_len);
}

void RemoteAIClient::subscribed_ok(String const &a_msg)
{
  Serial.print("subscribed ");
  Serial.println(a_msg);
}

void RemoteAIClient::publish_two(const uint8_t *a_msg, size_t a_len, const uint8_t *b_msg, size_t b_len)
{
    char pre[10] = ">>00000<<";
    pre_msg(a_len+b_len, pre);
    m_client.write(pre, 9);
    m_client.write(a_msg, a_len);
    m_client.write(b_msg, b_len);
}

size_t RemoteAIClient::pre_msg(unsigned long a_len, char * pre)
{
    a_len+=9;
    int pos = 6 - int(log10(a_len));
    sprintf(&pre[pos], "%lu<<", a_len);
    return a_len+9;
}

