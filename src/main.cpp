#include "message_container.hpp"
#include "message_queue.hpp"
#include "udp_receiver.hpp"
#include "tcp_sender.hpp"
#include "utils.hpp"
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <functional>
#include <iostream>

#include <string>
#include <csignal>

volatile sig_atomic_t keep_running = 1;

void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        keep_running = false;
        printf("\nReceived exit signal. Shutting down...");
    }
}

void udp_thread_func(uint16_t port, message_container& container, message_queue& msg_queue) {
    while (keep_running) {
        udp_receiver(port, container, msg_queue);
    }
}

void tcp_thread_func(const std::string& server_ip, uint16_t port, message_queue& msg_queue, message_container& container) {
    while (keep_running) {
        tcp_sender(server_ip.c_str(), port, msg_queue, container);
    }
}

int main(int argc, char* argv[]) {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    if (argc < 2)
        std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;

    const char* config_filename = argv[1];

    struct Config config;

    config.udp_port1 = 8080;
    config.udp_port2 = 8081;
    config.tcp_port = 9090;
    strncpy(config.server_ip, "127.0.0.1", sizeof(config.server_ip) - 1);
    config.server_ip[sizeof(config.server_ip) - 1] = '\0';

    if (read_config(config_filename, &config) != 0)
        printf("Using default configuration.\n");

    message_container container(1024);
    message_queue msg_queue(1024);    

    std::thread udp_thread1(udp_thread_func, config.udp_port1, std::ref(container), std::ref(msg_queue));
    std::thread udp_thread2(udp_thread_func, config.udp_port2 , std::ref(container), std::ref(msg_queue));

    std::thread tcp_thread(tcp_thread_func, config.server_ip, config.tcp_port, std::ref(msg_queue), std::ref(container));

    udp_thread1.join();
    udp_thread2.join();
    tcp_thread.join();

    return 0;
}
