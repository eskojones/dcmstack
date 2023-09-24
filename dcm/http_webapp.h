#ifndef DCM_HTTP_CMS_H
#define DCM_HTTP_CMS_H

#include <iostream>
#include <fstream>
#include <vector>
#include <map>

#include "http_server.h"



namespace dcm {

    class HttpWebApp : public HttpServer {

        private:
            std::map<std::string,std::string> m_Config { };
            std::map<std::string,std::vector<std::string>> m_RoutePaths { };
            std::map<std::string,std::function<void(HttpServer*,HttpServer::HttpResponse*)>> m_RouteFunctions { };

        public:
            int m_VersionMajor, m_VersionMinor, m_VersionRelease;
            HttpWebApp ();
            void Handler(HttpServer *server, HttpServer::HttpResponse *res);
            std::string GetFileContents(const std::string& filename);
            std::string GetRoute(HttpServer::HttpResponse *res);
            void AddRoute(const std::string& name, const std::string& path, const std::function<void(HttpServer*,HttpServer::HttpResponse*)>& fn);
            bool Listen() override;
            bool Listen(int port) override;
            void Tick() override;
            bool IsListening() override;
            void Stop() override;

    };

}


#endif //DCM_HTTP_CMS_H
