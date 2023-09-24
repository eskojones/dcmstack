#include "http_webapp.h"


namespace dcm {

    HttpWebApp::HttpWebApp ()
    : HttpServer(
        [&] (dcm::HttpServer *server, dcm::HttpServer::HttpResponse *res) {
            Handler(server, res);
        },
        3000
    ) {
        m_VersionMajor = 0;
        m_VersionMinor = 2;
        m_VersionRelease = 5;

        m_Config.insert({ "html_root", "../html" });

        AddRoute("default", "default",
            [&](HttpServer *server, HttpServer::HttpResponse *res) {
                res->status = 200;
                std::map<std::string,std::string> customHeaders { { "Content-Type", "text/html" } };
                res->body.append("<h1>HttpWebApp Default Handler</h1><hr/>\n");
                for (auto const &[k, v]: res->request) {
                    res->body.append(fmt::format("<li>{}: {}</li>\n", k, v));
                }
                res->body.append(fmt::format("<hr/><i>{}/{}\n", m_ServerName, m_ServerVersion));
                GenerateResponse(res, customHeaders);
            }
        );
    }


    bool HttpWebApp::Listen() {
        return HttpServer::Listen(3000);
    }

    bool HttpWebApp::Listen(int port) {
        return HttpServer::Listen(port);
    }


    void HttpWebApp::Tick() {
        HttpServer::Tick();
    }


    bool HttpWebApp::IsListening () {
        return HttpServer::IsListening();
    }


    std::string HttpWebApp::GetRoute(HttpServer::HttpResponse *res) {
        dcm::string uri { res->request.find("URI")->second };
        std::vector<dcm::string> uri_words = uri.split("/");

        for (auto const & [ name, path ] : m_RoutePaths) {
            //fmt::print("{}->\n", name);
            std::map<std::string,std::string> vars { };
            bool match = true;
            int word_num = 0;
            for (auto const & p : path) {
                //if (p.empty()) continue;
                if (word_num >= uri_words.size()) {
                    match = false;
                    break;
                }
                //fmt::print("[{}] [{}]\n", p, uri_words[word_num].data);
                if (p[0] == ':') {
                    vars.insert({ p.substr(1), uri_words[word_num].data });
                } else if (p != uri_words[word_num].data) {
                    match = false;
                    break;
                }
                word_num++;
            }
            if (word_num == uri_words.size()  && match) {
                for (auto const & [k,v] : vars) {
                    std::string varname = fmt::format("Var[{}]", k);
                    res->request.insert({ varname, v });
                }
                return name;
            }
        }

        return "default";
    }


    void HttpWebApp::Handler(HttpServer *server, HttpServer::HttpResponse *res) {
        std::string method = res->request.find("Method")->second;
        std::string uri = res->request.find("URI")->second;
        res->headers.insert({ "Web-App-Version", fmt::format("{}.{}.{}", m_VersionMajor, m_VersionMinor, m_VersionRelease) });
        std::string routeName = GetRoute(res);
        if (!routeName.empty()) {
            m_RouteFunctions.find(routeName)->second(server, res);
            return;
        }
        m_RouteFunctions.find("default")->second(server, res);
    }


    std::string HttpWebApp::GetFileContents(const std::string& filename) {
        std::string contents{}, line{};
        std::ifstream f(fmt::format("{}/{}", m_Config.find("html_root")->second, filename));
        if (!f.is_open())  return contents;
        while (std::getline(f, line)) contents.append(line);
        f.close();
        return contents;
    }


    void HttpWebApp::AddRoute(const std::string& name, const std::string& path, const std::function<void(HttpServer*,HttpServer::HttpResponse*)>& fn) {
        dcm::string _path { path };
        std::vector<dcm::string> path_words = _path.split("/");
        std::vector<std::string> path_words_std { };

        path_words_std.reserve(path_words.size());
        for (auto & path_word : path_words) {
            path_words_std.push_back(path_word.data);
        }

        if (m_RoutePaths.contains(name)) {
            m_RoutePaths.find(name)->second = path_words_std;
            m_RouteFunctions.find(name)->second = fn;
        } else {
            m_RoutePaths.insert({ name, path_words_std });
            m_RouteFunctions.insert({ name, fn });
        }
    }


    void HttpWebApp::Stop() {
        HttpServer::Stop();
    }

}
