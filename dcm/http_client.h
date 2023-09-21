#ifndef DCM_HTTP_CLIENT_H
#define DCM_HTTP_CLIENT_H
#include <iostream>
#include <map>

#include "client_socket.h"
#include "string.h"


namespace dcm{

    class HttpClient {

        private:
            bool m_Complete { false };
            int m_Port;
            std::string m_Host, m_Path;
            ClientSocket m_Socket { ClientSocket::SocketType::TCP };

        public:
            int m_StatusCode { 0 };
            std::map<std::string,std::string> m_Headers { };
            std::string m_Body { };

            explicit HttpClient(std::string_view host, std::string_view path = "/", int port = 80);
            ~HttpClient();
            void ParseHeaders(const std::vector<std::string>& headerLines);
            void ParseResponse(std::string_view res);
            void Finalise();
            std::string_view GetResponse();
            [[nodiscard]] std::map<std::string,std::string> GetHeaders() const;
            std::string_view GetHeader(const std::string& name);
    };
}

#endif //DCM_HTTP_CLIENT_H
