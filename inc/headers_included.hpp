#ifndef HEADERS_INCLUDED_HPP
#define HEADERS_INCLUDED_HPP

#include <stdio.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/select.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "client.hpp"
#include "server_status.hpp"


template <class Context>
using GotMessage = void(*)(Client &a_client, int _id, char *_msg, int _length, Context &_context);

template <class Context>
using CloseClient = void (*)(int _id, Context &_context);

template <class Context>
using NewClient = bool (*)(int _id, Context &_context);

template <class Context>
using OnFail = void (*)(int _id, std::string const &_err, Context &_context);

#endif // HEADERS_INCLUDED_HPP