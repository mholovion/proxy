cmake_minimum_required(VERSION 3.5)
project(Proxy)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

set(SOURCES
    src/main.cpp
    src/message_container.cpp
    src/message_queue.cpp
    src/udp_receiver.cpp
    src/tcp_sender.cpp
    src/utils.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _WIN32_WINNT=0x0601)
    target_link_libraries(${PROJECT_NAME} ws2_32)
endif()
