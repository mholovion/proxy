#ifndef TCP_SENDER_HPP
#define TCP_SENDER_HPP

#include "message_container.hpp"
#include "message_queue.hpp"

void tcp_sender(const char* server_ip, uint16_t port, message_queue& msg_queue, message_container& container);

#endif
