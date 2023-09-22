#ifndef DCM_HTTP_CMS_H
#define DCM_HTTP_CMS_H

#include <iostream>
#include <fstream>
#include <vector>
#include <map>

#include "http_server.h"



namespace dcm {

    class HttpWebApp : HttpServer {
        private:
            std::map<std::string,std::function<void(HttpServer*,HttpServer::HttpResponse*)>> m_Routes { };

        public:
            explicit HttpWebApp (int port);

            void Handler(HttpServer *server, HttpServer::HttpResponse *res);
            static std::string GetFileContents(const std::string& filename);
            void AddRoute(const std::string& prefix, const std::function<void(HttpServer*,HttpServer::HttpResponse*)>& fn);
            bool Listen() override;
            void Tick() override;
            bool IsListening() override;
    };

}

#endif //DCM_HTTP_CMS_H
