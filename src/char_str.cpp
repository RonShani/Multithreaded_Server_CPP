#include "char_str.hpp"
#include <string>

bool is_next_digits(int a_num_digits, size_t a_pos, std::string const &a_str)
{
    try{
        std::stoi(a_str.substr(a_pos, a_num_digits));
        return true;
    } catch (...) {
        return false;
    }
    
}

void take_out_counter_prefix(std::string &a_str)
{
    size_t pos = a_str.find(">>", 0);
    if (is_next_digits(5, pos+2, a_str) && a_str[pos+7] == '<' && a_str[pos+8] == '<'){
        a_str.erase(pos, 9);
    }
    while(pos != std::string::npos){
        if (is_next_digits(5, pos+2, a_str) && a_str[pos+7] == '<' && a_str[pos+8] == '<')a_str.erase(pos, 9);
        pos = a_str.find(">>",pos);
    }
}

bool is_str_within(char *a_str, char *a_search, int a_len)
{
    for (int i=0; i<a_len; ++i){
        if(a_str[i]!=a_search[i]) return false;
    }
    return true;
}

bool topic_from_to(char *a_msg, int a_length, size_t &a_from, size_t &a_to, char a_pre[4], char a_post[4])
{
    int i=0;
    for(;i<a_length; ++i){
        if(is_str_within(&a_msg[i], a_pre, 4)){
            i += 4;
            a_from = i;
            break;
        }
    }
    for(;i<a_length; ++i){
        if(is_str_within(&a_msg[i], a_post, 4)){
            a_to = i;
            return true;
        }
    }
    return false;
}

std::vector<size_t> find_all(std::string const& a_sub, std::string const& a_str)
{
    std::vector<size_t> positions;
    size_t pos = a_str.find(a_sub, 0);
    while(pos != std::string::npos){
        positions.push_back(pos);
        pos = a_str.find(a_sub,pos+1);
    }
    return positions;
}

std::vector<size_t> split_indexes(std::string const& a_str, std::string const& a_divider)
{
    std::vector<size_t> result;
    size_t pos = a_str.find(a_divider, 0);
    while(pos != std::string::npos){
        result.push_back(pos);
        pos = a_str.find(a_divider,pos+1);
    }
    return result;
}

std::vector<std::string> split_out(std::string const& a_str, std::string const& a_pre, std::string const& a_post)
{
    std::vector<std::string> result;
    std::vector<size_t> pres = split_indexes(a_str, a_pre);
    std::vector<size_t> posts = split_indexes(a_str, a_post);
    if (pres.size() != posts.size()) return {};
    for (size_t i = 0; i< pres.size(); ++i){
        result.push_back(a_str.substr(pres[i]+a_pre.size(), posts[i]-pres[i]-a_pre.size()));
    }
    return result;
}