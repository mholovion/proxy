# Proxy Application

## Description

This project implements a multithreaded application that receives messages over UDP and sends them over TCP based on certain conditions.

### Features

- **UDP Receivers**: Two threads listen for incoming UDP messages on specified ports.
- **TCP Sender**: A thread that connects to a server via TCP and sends messages that meet certain criteria.
- **Message Handling**: Messages are stored in a container. If a message's `MessageData` equals `10`, it is queued for sending over TCP.

### Message Format

Each message has the following binary structure (little-endian):

- `uint16_t MessageSize` (2 bytes)
- `uint8_t MessageType` (1 byte)
- `uint64_t MessageId` (8 bytes)
- `uint64_t MessageData` (8 bytes)

## Building the Project

The project uses CMake for building. Follow the steps below to build the application:

1. **Prepare the Build Directory**

   Navigate to the project directory and create a build directory:

   ```bash
   cd <project_directory>
   mkdir build
   cd build
    ```
2. **Generate Build Files with CMake**

    Generate the build files using CMake:
    ```bash
    cmake -S . -B build
    ```

    Build the Project

3. **Build the project using the generated build files:**

On Linux/macOS

```bash
cmake --build build
```
On Windows
```bash
cmake --build build --config Release
```

This will compile the project and create the executable in the build directory.

**Configuration**

Create a configuration file (e.g., config.txt) in the project directory with the following content:

```txt
udp_port1=8080
udp_port2=8081
tcp_port=9090
server_ip=127.0.0.1
```
*Configuration Parameters*

`udp_port1`: UDP port for the first receiver thread.
`udp_port2`: UDP port for the second receiver thread.
`tcp_port`: TCP port for the sender thread to connect to.
`server_ip`: IP address of the TCP server.


**Testing the Application**

Testing with Netcat
You can send a test message to the UDP receiver using netcat:

```bash
echo -n -e "\x13\x00\x01\x08\x00\x00\x00\x00\x00\x00\x00\x0A\x00\x00\x00\x00\x00\x00\x00" | nc -u 127.0.0.1 8080
```

**Running the Server:**
```bash
python3 tcp_server.py
```

**Stopping the Application**

To gracefully stop the application, press Ctrl + C in the terminal. The application will handle the interrupt signal and shut down cleanly.