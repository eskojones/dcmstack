#include <iostream>
#include <fmt/core.h>

#include "dcm/string.h"
#include "dcm/server_socket.h"
#include "dcm/client_socket.h"
#include "dcm/http_client.h"
#include "dcm/http_server.h"
#include "dcm/http_webapp.h"





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
    std::string_view buffer = server->GetBuffer(socketIndex);
    if (!buffer.find('\n')) return;
    fmt::print("<{}> {}", socketIndex, buffer);
    server->Broadcast(fmt::format("<{}> {}", socketIndex, buffer));
    server->ClearBuffer(socketIndex);
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





void httpNodeRequestRouter(dcm::HttpServer *server, dcm::HttpServer::HttpResponse *res) {
    std::string method = res->request.find("Method")->second;
    std::string path = res->request.find("Path")->second;
    res->status = 200;
    res->body = path;
    std::string tmpFile = fmt::format("/tmp/{}{}.txt", res->request.find("IP-Address")->second, time(nullptr));
    std::string cmd = fmt::format("{} {} >{}", "/opt/homebrew/bin/node", "server.js", tmpFile);
    system(cmd.data());
    std::ifstream f(tmpFile);
    if (f.is_open()) {
        std::string line {};
        res->body = "";
        while (std::getline(f, line)) res->body.append(line);
        f.close();
    }
    server->GenerateResponse(res, { });
}


int main () {
    //test dcm/string.h
    /*
    dcm::string s { "    \t   this:    is :a:test  " };
    auto words = s.trim().rtrim().split(":");
    for (auto &word : words) fmt::print("{}\n", word.trim().rtrim().data);
    */

    //test dcm/http_client.h (and client socket)
    /*
    dcm::HttpClient http { "vectrex.dcm.nz", "/", 80 };
    http.GetResponse();
    std::map<std::string,std::string> headers = http.GetHeaders();
    for (auto const& [k,v] : headers) {
        fmt::print("{} = {}\n", k, v);
    }
     */

    //test dcm/server_socket.h
    /*
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
    }*/

    dcm::HttpWebApp app { };
    dcm::HttpServer httpd {
        [&](dcm::HttpServer *server, dcm::HttpServer::HttpResponse *res) {
            app.Handler(server, res);
        },
        42069
    };
    if (!httpd.Listen()) return 1;
    fmt::print("Listening on 42069\n");
    while (true) {
        httpd.Tick();
        usleep(200 * 1000);
    }

    return 0;
}
