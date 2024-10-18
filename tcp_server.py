import socket

server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

server_socket.bind(('127.0.0.1', 9090))

server_socket.listen(5)

print('Server is listening on 127.0.0.1:9090')

while True:
    client_socket, addr = server_socket.accept()
    print(f'Accepted connection from {addr}')
    data = client_socket.recv(1024)
    if data:
        print(f'Received data: {data}')
    client_socket.close()
