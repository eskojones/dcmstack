#include <iostream>
#include <fmt/core.h>

#include "dcm/string.h"
#include "dcm/server_socket.h"
#include "dcm/client_socket.h"
#include "dcm/http_client.h"
#include "dcm/http_server.h"
#include "dcm/http_webapp.h"
#include "todo_app.h"
#include "fdn.h"


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


int main () {
//    dcm::HttpWebApp app{};
//
//    app.AddRoute("test", "/foo/:id/bar/:name",
//         [&](dcm::HttpServer *server, dcm::HttpServer::HttpResponse *res) {
//             std::map<std::string,std::string> customHeaders {
//                 { "Content-Type", "text/html" }
//             };
//             res->status = 200;
//             res->body = "<h1>Test Route</h1><hr/>\n";
//             for (auto const &[k, v]: res->request) {
//                 res->body.append(fmt::format("<li>{}: {}</li>\n", k, v));
//             }
//             server->GenerateResponse(res, customHeaders);
//         }
//    );
//    if (!app.Listen(42069)) {
//        fmt::print("Failed to start HttpWebApp server\n");
//        return 1;
//    }
//    fmt::print("Listening on 42069\n");
//    while (app.IsListening()) {
//        app.Tick();
//        usleep(1000);
//    }
    FDN fdn;
    fdn.Start(42069);
    while(fdn.IsListening()) {
        fdn.Tick();
        usleep(100 * 1000);
    }

    return 0;
}
