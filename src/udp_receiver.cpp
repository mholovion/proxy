#include "udp_receiver.hpp"
#include "message.hpp"
#include "utils.hpp"
#include <cstdio>
#include <cstring>
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #pragma comment(lib, "Ws2_32.lib")
#else
    #include <arpa/inet.h>
    #include <errno.h>
    #include <sys/socket.h>
#endif

extern volatile sig_atomic_t keep_running;

void udp_receiver(uint16_t port, message_container& container, message_queue& msg_queue) {
    initialize_sockets();

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        print_error("socket");
        
        yield_thread();
        return;
    }

    set_non_blocking(sock);
    
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        print_error("bind");
        close_socket(sock);
        yield_thread();
        return;
    }

    while (keep_running) {
        message msg;
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);

        int recv_len = recvfrom(sock, (char*)&msg, sizeof(msg), 0, (struct sockaddr*)&client_addr, &client_len);
        if (recv_len > 0) {
            if (recv_len != sizeof(message)) {
                continue;
            }
            
            if (!container.exists(msg.message_id)) {
                if (container.insert(msg)) {
                    if (msg.message_data == 10) {
                        if (!msg_queue.push(msg.message_id)) {
                            printf("Message queue is full. Cannot push message_id: %llu\n", msg.message_id);
                        }
                    }
                } else {
                    printf("Failed to insert message with message_id: %llu\n", msg.message_id);
                }
            }
        } else if (recv_len == 0) {
            
            continue;
        } else {
            
        #ifdef _WIN32
            int error = WSAGetLastError();
            if (error != WSAEWOULDBLOCK && error != WSAEINPROGRESS) {
                print_socket_error("recvfrom", error);
                
                close_socket(sock);
                yield_thread();
                return;
            }
        #else
            if (errno != EWOULDBLOCK && errno != EAGAIN) {
                perror("recvfrom");

                close_socket(sock);
                yield_thread();
                return;
            }
        #endif
        }
        yield_thread();
    }
    close_socket(sock);
}
