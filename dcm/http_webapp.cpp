#include "http_webapp.h"

namespace dcm {

    void HttpWebApp::Handler(HttpServer *server, HttpServer::HttpResponse *res) {
        std::string method = res->request.find("Method")->second;
        std::string path = res->request.find("Path")->second;
        std::map<std::string,std::string> customHeaders { { "Web-App-Version", "0.0.1" } };
        res->status = 200;
        if (res->path.size() > 2 && res->path[1] == "js" && res->path[res->path.size() - 1].ends_with(".js")) {
            res->body = GetFileContents(path);
            if (res->body.empty()) res->status = 404;
            customHeaders.insert({ "Content-Type", "text/javascript" });
            server->GenerateResponse(res, customHeaders);
            return;
        }

        customHeaders.insert({ "Content-Type", "text/html" });
        res->body = "";
        res->body.append(GetFileContents("header.html"));
        if (res->path.size() > 1 && res->path[res->path.size() - 1].ends_with(".html")) {
            res->body.append(GetFileContents(res->path[res->path.size() - 1]));
        } else {
            res->body.append("<h1>HttpWebApp.Handler</h1><hr/>\n");
            for (auto const &[k, v]: res->request) {
                res->body.append(fmt::format("<li>{}: {}</li>\n", k, v));
            }
            res->body.append(fmt::format("<hr/><i>{}/{}\n", server->m_ServerName, server->m_ServerVersion));
        }
        res->body.append(GetFileContents("footer.html"));

        server->GenerateResponse(res, customHeaders);
    }

    std::string HttpWebApp::GetFileContents(const std::string& filename) {
        std::string contents{}, line{};
        std::ifstream f(fmt::format("./html/{}", filename));
        if (!f.is_open())  return contents;
        while (std::getline(f, line)) contents.append(line);
        f.close();
        return contents;
    }
}