#include <iostream>
#include <fmt/core.h>

#include "dcm/string.h"
#include "dcm/server_socket.h"
#include "dcm/client_socket.h"
#include "dcm/http_client.h"


void onListen (dcm::ServerSocket *server, int socketIndex) {
    fmt::print("ServerSocket.Listen\n");
}

void onAccept (dcm::ServerSocket *server, int socketIndex) {
    fmt::print("ServerSocket.Accept: {} connected.\n", server->GetAddress(socketIndex));
    server->Send(socketIndex, "Welcome to the server! Feel free to type stuff...\n");
    server->Broadcast(fmt::format("{} has joined.\n", server->GetAddress(socketIndex)));
}

void onClose (dcm::ServerSocket *server, int socketIndex) {
    fmt::print("ServerSocket.Close: {} disconnected.\n", server->GetAddress(socketIndex));
}

void onSend (dcm::ServerSocket *server, int socketIndex) {
}

void onRecv (dcm::ServerSocket *server, int socketIndex) {
    fmt::print("<{}> {}", socketIndex, server->GetBuffer(socketIndex));
    server->Broadcast(fmt::format("<{}> {}", socketIndex, server->GetBuffer(socketIndex)));
}

void onListenFailure(dcm::ServerSocket *server, int) {
    fmt::print("ServerSocket.Listen: Error: {}\n", server->GetFailure()->Message);
}

void onAcceptFailure (dcm::ServerSocket *server, int socketIndex) {
    if (socketIndex > -1) fmt::print("ServerSocket.Accept: Error: {}\n", server->GetFailure()->Message);
}

void onCloseFailure (dcm::ServerSocket *server, int socketIndex) {
    fmt::print("ServerSocket.Close: Error: {}\n", server->GetFailure()->Message);
}

void onSendFailure (dcm::ServerSocket *server, int socketIndex) {
    fmt::print("ServerSocket.Send: Error: {}\n", server->GetFailure()->Message);
}

void onRecvFailure (dcm::ServerSocket *server, int socketIndex) {
    if (socketIndex > -1) fmt::print("ServerSocket.Recv: Error: {}\n", server->GetFailure()->Message);
}


int main () {
    //test dcm/string.h
    dcm::string s { "    \t   this:    is :a:test  " };
    auto words = s.trim().rtrim().split(":");
    for (auto &word : words) fmt::print("{}\n", word.trim().rtrim().data);

    //test dcm/http_client.h (and client socket)
    dcm::HttpClient http { "vectrex.dcm.nz", "/", 80 };
    http.GetResponse();
    fmt::print("{}\n", http.GetHeader("Server"));

    //test dcm/server_socket.h
    dcm::ServerSocket server { dcm::ServerSocket::SocketType::TCP };
    server.AddHandler("listen", onListen);
    server.AddHandler("accept", onAccept);
    server.AddHandler("close", onClose);
    server.AddHandler("send", onSend);
    server.AddHandler("recv", onRecv);
    server.AddHandler("listen-failure", onListenFailure);
    server.AddHandler("accept-failure", onAcceptFailure);
    server.AddHandler("close-failure", onCloseFailure);
    server.AddHandler("send-failure", onSendFailure);
    server.AddHandler("recv-failure", onRecvFailure);
    server.Listen(42069);

    while(server.IsListening()) {
        server.Accept();
        server.RecvAll();
        usleep(200 * 1000);
    }

    return 0;
}
