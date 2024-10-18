#include "utils.hpp"
#include <cstdio>
#include <cstring>
#include <thread>
#include <fstream>
#include <iostream>
#include <errno.h> 
#ifdef _WIN32
    #include <winsock2.h>
    #include <windows.h>
#else
    #include <unistd.h>
    #include <fcntl.h>
    #include <errno.h>
    #include <sched.h>
    #include <time.h> 
#endif

void initialize_sockets() {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

void set_non_blocking(int sock) {
#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(sock, FIONBIO, &mode);
#else
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1) {
        flags = 0;
    }
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
#endif
}

void close_socket(int sock) {
#ifdef _WIN32
    closesocket(sock);
#else
    close(sock);
#endif
}

void yield_thread() {
    std::this_thread::yield();
}

void print_error(const char* msg) {
#ifdef _WIN32
    DWORD error_code = GetLastError();
    char* error_msg = nullptr;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&error_msg, 0, nullptr);
    if (error_msg) {
        std::cerr << msg << ": " << error_msg << std::endl;
        LocalFree(error_msg);
    } else {
        std::cerr << msg << ": Unknown error code " << error_code << std::endl;
    }
#else
    std::cerr << msg << ": " << strerror(errno) << std::endl;
#endif
}


void print_socket_error(const char* msg, int error_code) {
#ifdef _WIN32
    char* error_msg = nullptr;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        nullptr, error_code, 0, (LPSTR)&error_msg, 0, nullptr);
    fprintf(stderr, "%s: %s\n", msg, error_msg);
    LocalFree(error_msg);
#else
    fprintf(stderr, "%s: %s\n", msg, strerror(error_code));
#endif
}

uint64_t get_current_time_ms() {
#ifdef _WIN32
    return GetTickCount64();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint64_t time_ms = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    return time_ms;
#endif
}

int read_config(const char* filename, struct Config* config) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Unable to open config file: %s\n", filename);
        return -1;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char* newline = strchr(line, '\n');
        if (newline)
            *newline = '\0';

        if (line[0] == '\0' || line[0] == '#')
            continue;

        char* equal_sign = strchr(line, '=');
        if (!equal_sign)
            continue; 

        *equal_sign = '\0';
        char* key = line;
        char* value = equal_sign + 1;

        if (strcmp(key, "udp_port1") == 0) {
            config->udp_port1 = (uint16_t)atoi(value);
        } else if (strcmp(key, "udp_port2") == 0) {
            config->udp_port2 = (uint16_t)atoi(value);
        } else if (strcmp(key, "tcp_port") == 0) {
            config->tcp_port = (uint16_t)atoi(value);
        } else if (strcmp(key, "server_ip") == 0) {
            strncpy(config->server_ip, value, sizeof(config->server_ip) - 1);
            config->server_ip[sizeof(config->server_ip) - 1] = '\0';
        }
    }
    fclose(file);
    return 0;
}