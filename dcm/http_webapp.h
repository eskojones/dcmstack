#ifndef DCM_HTTP_CMS_H
#define DCM_HTTP_CMS_H

#include <iostream>
#include <fstream>
#include <vector>
#include <map>

#include "http_server.h"



namespace dcm {

    class HttpWebApp {

        public:
            HttpWebApp() = default;
            void Handler(HttpServer *server, HttpServer::HttpResponse *res);
            static std::string GetFileContents(const std::string& filename);

    };

}

#endif //DCM_HTTP_CMS_H
