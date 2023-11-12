#include <string>
#include <vector>

bool is_next_digits(int a_num_digits, size_t a_pos, std::string const &a_str);
void take_out_counter_prefix(std::string &a_str);
bool is_str_within(char *a_str, char *a_search, int a_len);
bool topic_from_to(char *a_msg, int a_length, size_t &a_from, size_t &a_to, char a_pre[4], char a_post[4]);
std::vector<size_t> find_all(std::string const& a_sub, std::string const& a_str);
std::vector<size_t> split_indexes(std::string const& a_str, std::string const& a_divider);
std::vector<std::string> split_out(std::string const& a_str, std::string const& a_pre, std::string const& a_post);