#include "http_webapp.h"


namespace dcm {

    HttpWebApp::HttpWebApp (int port)
    : HttpServer(
        [&] (dcm::HttpServer *server, dcm::HttpServer::HttpResponse *res) {
            Handler(server, res);
        },
        port
    ) {
        m_Config.insert({ "html_root", "../html" });
        m_Config.insert({ "default_header_file", "header.html" });
        m_Config.insert({ "default_footer_file", "footer.html" });
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
        std::string uri = res->request.find("URI")->second;
        for (auto const& [k,v] : m_Routes) {
            if (!uri.starts_with(k)) continue;
            res->request.insert({ "Route", k });
            v(server, res);
            return;
        }

        res->status = 200;
        std::map<std::string,std::string> customHeaders { { "Web-App-Version", "0.1.0" } };

        if (res->path.size() > 1) {
            dcm::string filename{res->request.find("URI")->second};
            std::string extension = filename.substr(filename.lastIndexOf('.'),
                                                    static_cast<int>(filename.data.size())).data;

            bool validFile = false;
            if (extension == ".html" || extension == ".htm") {
                customHeaders.insert({"Content-Type", "text/html"});
                validFile = true;
            } else if (extension == ".js") {
                customHeaders.insert({"Content-Type", "text/javascript"});
                validFile = true;
            } else if (extension == ".css") {
                customHeaders.insert({"Content-Type", "text/stylesheet"});
                validFile = true;
            }

            if (validFile) {
                res->body = GetFileContents(res->request.find("URI")->second);
                if (res->body.empty()) res->status = 404;
                GenerateResponse(res, customHeaders);
                return;
            }
        }

        //fall-through to default page
        customHeaders.insert({ "Content-Type", "text/html" });
        res->body = "";
        res->body.append(GetFileContents(m_Config.find("default_header_file")->second));
        //default page
        res->body.append("<h1>HttpWebApp.Handler</h1><hr/>\n");
        for (auto const &[k, v]: res->request) {
            res->body.append(fmt::format("<li>{}: {}</li>\n", k, v));
        }
        res->body.append(fmt::format("<hr/><i>{}/{}\n", m_ServerName, m_ServerVersion));
        res->body.append(GetFileContents(m_Config.find("default_footer_file")->second));
        GenerateResponse(res, customHeaders);
    }


    std::string HttpWebApp::GetFileContents(const std::string& filename) {
        std::string contents{}, line{};
        std::ifstream f(fmt::format("{}/{}", m_Config.find("html_root")->second, filename));
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
