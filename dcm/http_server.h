#ifndef DCM_HTTP_SERVER_H
#define DCM_HTTP_SERVER_H
#include <iostream>
#include <functional>
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
                    std::map<std::string,std::string> request { };
                    std::map<std::string,std::string> headers { };
                    std::string body { };
                    std::vector<std::string> path { };
                    HttpResponse();
                    ~HttpResponse();
            };
            int m_Port = 80;
            std::string m_ServerName { "DcmStackHttpServer" };
            std::string m_ServerVersion { "0.1.0" };
            char m_DateBuffer[100] { };
            std::function<void(HttpServer *,HttpResponse *)> m_RequestRouter;

            HttpServer(std::function<void(HttpServer *,HttpResponse *)> router, int port);
            virtual bool Listen(int port);
            virtual bool Listen();
            std::string GetDateString(time_t *time, const std::string& fmt);
            void LogRequest(HttpResponse *response);
            void GenerateResponse(HttpResponse *res, const std::map<std::string,std::string>& customHeaders);
            static std::string GetResponseString(HttpResponse *res);
            void HandleHeadRequest(HttpResponse *res);
            void HandleGetRequest(HttpResponse *res);
            void HandlePostRequest(HttpResponse *res);
            void HandleRequest(int socketIndex, std::string_view request_buffer);
            void OnAccept(ServerSocket *server, int index);
            void OnRecv(ServerSocket *server, int index);
            virtual void Tick();
            virtual bool IsListening();

        private:
            ServerSocket m_Socket { ServerSocket::SocketType::TCP };

    };
}

#endif //DCM_HTTP_SERVER_H
