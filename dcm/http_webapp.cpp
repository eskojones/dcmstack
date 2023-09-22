#include "http_webapp.h"

namespace dcm {
    HttpWebApp::HttpWebApp (int port)
    : HttpServer(
        [&] (dcm::HttpServer *server, dcm::HttpServer::HttpResponse *res) {
            Handler(server, res);
        },
        port
    ) {
    }

    bool HttpWebApp::Listen() {
        return HttpServer::Listen();
    }

    void HttpWebApp::Tick() {
        HttpServer::Tick();
    }

    bool HttpWebApp::IsListening () {
        return HttpServer::IsListening();
    }

    void HttpWebApp::Handler(HttpServer *server, HttpServer::HttpResponse *res) {
        std::string method = res->request.find("Method")->second;
        std::string path = res->request.find("Path")->second;
        for (auto const& [k,v] : m_Routes) {
            if (path.starts_with(k)) {
                res->request.insert({ "Route", k });
                v(server, res);
                return;
            }
        }

        std::map<std::string,std::string> customHeaders { { "Web-App-Version", "0.1.0" } };
        res->status = 200;

        //Javascript files
        if (res->path.size() > 2 && res->path[1] == "js" && res->path[res->path.size() - 1].ends_with(".js")) {
            res->body = GetFileContents(fmt::format("js/{}", res->path[res->path.size() - 1]));
            if (res->body.empty()) res->status = 404;
            customHeaders.insert({ "Content-Type", "text/javascript" });
            GenerateResponse(res, customHeaders);
            return;
        }

        customHeaders.insert({ "Content-Type", "text/html" });
        res->body = "";
        res->body.append(GetFileContents("header.html"));
        //.html files
        if (res->path.size() > 1 && res->path[res->path.size() - 1].ends_with(".html")) {
            res->body.append(GetFileContents(res->path[res->path.size() - 1]));
        } else {
            //default page
            res->body.append("<h1>HttpWebApp.Handler</h1><hr/>\n");
            for (auto const &[k, v]: res->request) {
                res->body.append(fmt::format("<li>{}: {}</li>\n", k, v));
            }
            res->body.append(fmt::format("<hr/><i>{}/{}\n", m_ServerName, m_ServerVersion));
        }
        res->body.append(GetFileContents("footer.html"));

        GenerateResponse(res, customHeaders);
    }

    std::string HttpWebApp::GetFileContents(const std::string& filename) {
        std::string contents{}, line{};
        std::ifstream f(fmt::format("./html/{}", filename));
        if (!f.is_open())  return contents;
        while (std::getline(f, line)) contents.append(line);
        f.close();
        return contents;
    }

    void HttpWebApp::AddRoute(const std::string& prefix, const std::function<void(HttpServer*,HttpServer::HttpResponse*)>& fn) {
        if (m_Routes.contains(prefix)) {
            m_Routes.find(prefix)->second = fn;
        } else {
            m_Routes.insert({ fmt::format("{}/", prefix), fn });
        }
    }

}
