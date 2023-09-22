#ifndef DCM_HTTP_SERVER_H
#define DCM_HTTP_SERVER_H
#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include "server_socket.h"
#include "string.h"

namespace dcm {

    class HttpServer {

        public:
            class HttpResponse {
                public:
                    int status;
                    std::map<std::string,std::string> request{};
                    std::map<std::string,std::string> headers{};
                    std::string body{};
                    HttpResponse();
                    ~HttpResponse();
            };
            std::string m_Root { "./html" };
            int m_Port = 80;
            std::string m_ServerName { "DcmStackHttpServer" };
            std::string m_ServerVersion { "0.1.0" };
            char m_DateBuffer[30]{};

            HttpServer(int port, std::string_view root);
            ~HttpServer();
            bool Listen(int port);
            bool Listen();
            std::string GetDateString(time_t *time, std::string fmt);
            void LogRequest(HttpResponse *response);
            void GenerateResponse(HttpResponse *res, const std::map<std::string,std::string>& customHeaders);
            static std::string GetResponseString(HttpResponse *res);
            void HandleHeadRequest(HttpResponse *res);
            void HandleGetRequest(HttpResponse *res);
            void HandlePostRequest(HttpResponse *res);
            void HandleRequest(int socketIndex, std::string_view request_buffer);
            void OnAccept(ServerSocket *server, int index);
            void OnRecv(ServerSocket *server, int index);
            void Tick();

        private:
            ServerSocket m_Socket { ServerSocket::SocketType::TCP };

    };
}

#endif //DCM_HTTP_SERVER_H
