#include "tcp_sender.hpp"
#include "message.hpp"
#include "utils.hpp"
#include <cstdio>
#include <cstring>
#include <stdint.h> 
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #pragma comment(lib, "Ws2_32.lib")
#else
    #include <arpa/inet.h>
    #include <errno.h>
    #include <sys/socket.h>
    #include <sys/select.h>
    #include <sys/time.h>
    #include <unistd.h>
#endif

extern volatile sig_atomic_t keep_running;

void tcp_sender(const char* server_ip, uint16_t port, message_queue& msg_queue, message_container& container) {
    initialize_sockets();

    int sock = -1;
    bool is_connected = false;

    uint64_t last_connect_time = 0;
    const uint64_t reconnect_interval_ms = 5000; 

    while (keep_running) {
        if (!is_connected) {
            uint64_t current_time = get_current_time_ms();
            if (sock == -1) {
                last_connect_time = current_time;
            } else {
                uint64_t elapsed_time = current_time - last_connect_time;
                if (elapsed_time < reconnect_interval_ms) {
                    
                    yield_thread();
                    continue;
                } else {
                    last_connect_time = current_time;
                }
            }

            if (sock == -1) {
                
                sock = socket(AF_INET, SOCK_STREAM, 0);
                if (sock < 0) {
                    print_error("socket");
                    
                    yield_thread();
                    continue;
                }

                set_non_blocking(sock);

                
                sockaddr_in server_addr{};
                server_addr.sin_family = AF_INET;
                server_addr.sin_port = htons(port);
                inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

                
                int result = connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
                if (result < 0) {
            #ifdef _WIN32
                    int error = WSAGetLastError();
                    if (error != WSAEWOULDBLOCK && error != WSAEINPROGRESS) {
                        print_socket_error("connect", error);
                        close_socket(sock);
                        sock = -1;
                        yield_thread();
                        continue;
                    }
            #else
                    if (errno != EINPROGRESS && errno != EWOULDBLOCK && errno != EAGAIN) {
                        perror("connect");
                        close_socket(sock);
                        sock = -1;
                        yield_thread();
                        continue;
                    }
            #endif
                }
                
                if (result == 0) {
                    is_connected = true;
                    printf("TCP connection established.\n");
                }
            } else {
                
                
                fd_set write_set;
                FD_ZERO(&write_set);
                FD_SET(sock, &write_set);

                timeval timeout{};
                timeout.tv_sec = 0;  
                timeout.tv_usec = 0;

                int result = select(sock + 1, nullptr, &write_set, nullptr, &timeout);
                if (result > 0 && FD_ISSET(sock, &write_set)) {
                    int so_error = 0;
                    socklen_t len = sizeof(so_error);
                    getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&so_error, &len);

                    if (so_error == 0) {
                        is_connected = true;
                        printf("TCP connection established.\n");
                    } else {
                        print_socket_error("connect", so_error);
                        close_socket(sock);
                        sock = -1;
                        is_connected = false;
                        yield_thread();
                        continue;
                    }
                } else if (result == 0) {
                    
                    yield_thread();
                    continue;
                } else {
                    
                    perror("select");
                    close_socket(sock);
                    sock = -1;
                    is_connected = false;
                    yield_thread();
                    continue;
                }
            }
        }
        
        char buffer;
        int bytes_received = recv(sock, &buffer, 1, MSG_PEEK);
        if (bytes_received == 0) {
            
            printf("Connection closed by server.\n");
            close_socket(sock);
            sock = -1;
            is_connected = false;
            yield_thread();
            continue;
        } else if (bytes_received < 0) {
        #ifdef _WIN32
            int error = WSAGetLastError();
            if (error != WSAEWOULDBLOCK && error != WSAEINPROGRESS) {
                print_socket_error("recv", error);
                close_socket(sock);
                sock = -1;
                is_connected = false;
                yield_thread();
                continue;
            }
        #else
            if (errno != EWOULDBLOCK && errno != EAGAIN) {
                perror("recv");
                close_socket(sock);
                sock = -1;
                is_connected = false;
                yield_thread();
                continue;
            }
        #endif
        }
        
        uint64_t message_id;
        if (msg_queue.pop(message_id)) {
            message msg;
            if (container.get(message_id, msg)) {
                int sent_bytes = send(sock, (char*)&msg, sizeof(msg), 0);
                if (sent_bytes <= 0) {
        #ifdef _WIN32
                    int error = WSAGetLastError();
                    if (error != WSAEWOULDBLOCK && error != WSAEINPROGRESS) {
                        print_socket_error("send", error);
                        close_socket(sock);
                        sock = -1;
                        is_connected = false;
                        yield_thread();
                        continue;
                    }
        #else
                    if (errno != EWOULDBLOCK && errno != EAGAIN) {
                        perror("send");
                        close_socket(sock);
                        sock = -1;
                        is_connected = false;
                        yield_thread();
                        continue;
                    }
        #endif
                } else {
                    printf("Message sent: MessageId = %llu\n", msg.message_id);
                }
            } else {
                
                yield_thread();
                continue;
            }
        } else {
            
            yield_thread();
            continue;
        }
    }

    if (sock >= 0) {
        close_socket(sock);
    }
}
