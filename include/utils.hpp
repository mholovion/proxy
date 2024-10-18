#ifndef UTILS_HPP
#define UTILS_HPP

#include <stdint.h> 

struct Config {
    uint16_t udp_port1;
    uint16_t udp_port2;
    uint16_t tcp_port;
    char server_ip[64];
};

void initialize_sockets();
void set_non_blocking(int sock);
void close_socket(int sock);
void yield_thread();
void print_error(const char* msg);
void print_socket_error(const char* msg, int error_code);
uint64_t get_current_time_ms();
int read_config(const char* filename, struct Config* config);

#endif 
