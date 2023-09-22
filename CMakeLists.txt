cmake_minimum_required(VERSION 3.12)
project(untitled1)

set(CMAKE_CXX_STANDARD 20)
include_directories(/usr/local/include)
link_directories(/usr/local/lib)
link_libraries(fmt)
add_executable(untitled1 main.cpp
        dcm/server_socket.cpp
        dcm/server_socket.h
        dcm/client_socket.cpp
        dcm/client_socket.h
        dcm/http_client.cpp
        dcm/http_client.h
        dcm/string.cpp
        dcm/string.h
        dcm/http_server.cpp
        dcm/http_server.h
        dcm/http_webapp.cpp
        dcm/http_webapp.h
        todo_app.cpp
        todo_app.h
)
