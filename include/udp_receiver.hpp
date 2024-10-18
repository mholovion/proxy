#ifndef UDP_RECEIVER_HPP
#define UDP_RECEIVER_HPP

#include "message_container.hpp"
#include "message_queue.hpp"

void udp_receiver(uint16_t port, message_container& container, message_queue& msg_queue);

#endif
