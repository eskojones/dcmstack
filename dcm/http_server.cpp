#include "http_server.h"

namespace dcm {
    HttpServer::HttpResponse::HttpResponse () {}
    HttpServer::HttpResponse::~HttpResponse () {}

    HttpServer::HttpServer (int port, std::string_view root) : m_Port { port }, m_Root { root }, m_DateBuffer { "" } {
        m_Socket.AddHandler("accept", [&](ServerSocket *server, int socketIndex) {
            OnAccept(server, socketIndex);
        });
        m_Socket.AddHandler("recv", [&](ServerSocket *server, int socketIndex) {
            OnRecv(server, socketIndex);
        });
    }

    HttpServer::~HttpServer() {
        HttpServer(80, "./html");
    }

    bool HttpServer::Listen(int port) {
        if (m_Socket.IsListening()) return false;
        m_Port = port;
        return m_Socket.Listen(m_Port);
    }

    bool HttpServer::Listen() {
        return Listen(m_Port);
    }


    std::string HttpServer::GetDateString(time_t *time, std::string fmt = "%a, %d %b %Y") {
        auto tm = *std::localtime(time);
        memset(m_DateBuffer, 0, 30);
        strftime(m_DateBuffer, 30, fmt.data(), &tm);
        return { m_DateBuffer };
    }


    void HttpServer::GenerateResponse(HttpResponse *res, const std::map<std::string,std::string>& customHeaders) {
        time_t now = time(nullptr);
        std::string responseDateStr = GetDateString(&now);

        res->headers.insert({ "Server", fmt::format("{}/{}", m_ServerName, m_ServerVersion ) });
        res->headers.insert({ "Last-Modified", fmt::format("{} 00", responseDateStr) });
        res->headers.insert({ "Date", fmt::format("{} 00", responseDateStr) });
        res->headers.insert({ "Content-Type", "text/plain" });
        res->headers.insert({ "Content-Length", fmt::format("{}", res->body.size()) });
        res->headers.insert({ "Connection", "close" });
        res->headers.insert({ "Accept-Ranges", "bytes" });

        for (auto const& [k,v] : customHeaders) {
            if (res->headers.contains(k)) {
                res->headers.find(k)->second = v;
            } else res->headers.insert({ k, v });
        }
    }


    void HttpServer::HandleHeadRequest(HttpResponse *res) {
        return GenerateResponse(res, {});
    }

    void HttpServer::HandleGetRequest(HttpResponse *res) {
        //determine if path is in a sub-dir
        int status = 404;
        std::string contents;
        string _path { res->request.find("Path")->second };
        auto path_words = _path.split("/");
        int depth = 0;
        for (auto const& word : path_words) {
            if (word.data == "..") depth--;
            else depth++;
        }
        //if path depth is ok, then read the file
        if (depth >= 0) {
            std::ifstream f(fmt::format("{}{}", m_Root, res->request.find("Path")->second));
            if (f.is_open()) {
                status = 200;
                std::string fline{};
                while (std::getline(f, fline)) {
                    contents.append(fline);
                }
                f.close();
            }
        }
        if (status == 404) {
            contents = "File not found";
        } else {
            res->body = contents;
        }

        return GenerateResponse(res, {});
    }

    void HttpServer::HandlePostRequest(HttpResponse *res) {
        //Not Yet Implemented
        return HandleGetRequest(res);
    }

    void HttpServer::LogRequest(HttpResponse *res) {
        time_t now = time(nullptr);
        std::string logDateStr = GetDateString(&now, "%D/%b/%Y:%H:%M:%S %z");
        std::string log = fmt::format(
            R"({} {} {} [{}] "{} {} {}" {} {} "{}" "{}" "{}"{})",
            res->request.find("IP-Address")->second,
            "-", "-",
            logDateStr,
            res->request.find("Method")->second,
            res->request.find("Path")->second,
            res->request.find("Protocol")->second,
            res->status,
            res->body.size(),
            res->request.contains("Referer") ? res->request.find("Referer")->second : "",
            res->request.contains("User-Agent") ? res->request.find("User-Agent")->second : "Unknown",
            "0.00", //unknown
            "\n"
        );
        fmt::print("{}", log);
        std::ofstream logf("access.log");
        if (logf.is_open()) {
            logf.write(log.data(), static_cast<long>(log.size()));
            logf.close();
        }
    }

    std::string HttpServer::GetResponseString(HttpResponse *res) {
        std::string str = fmt::format("HTTP {} OK\n", res->status);
        for (auto const& [k,v] : res->headers) {
            str = fmt::format("{}{}: {}\n", str, k, v);
        }
        str = fmt::format("{}\n{}", str, res->body);
        return str;
    }

    void HttpServer::HandleRequest(int socketIndex, std::string_view request_buffer) {
        HttpResponse res {};
        string request { request_buffer.data() };
        std::vector<string> lines = request.filter("\r\t").split("\n");
        std::vector<string> words = lines[0].split(" ");
        for (auto const& line : lines) {
            if (line.empty()) continue;
            string header { line.data };
            std::vector<string> kvp = header.split(":");
            if (kvp.size() < 2) continue;
            if (kvp.size() > 2) {
                for (int i = 2; i < kvp.size(); i++) {
                    kvp[1].data.append(fmt::format(":{}", kvp[i].data));
                }
            }
            for (auto s : kvp) s.trim().rtrim().filter("\n");
            res.request.insert({ kvp[0].data, kvp[1].trim().data });
            //fmt::print("header: {}={}\n", kvp[0].data, headers.find(kvp[0].data)->second);
        }
        std::string method = words[0].data;
        std::string path = words[1].data;
        std::string proto = words[2].data;
        std::string host = res.request.contains("Host") ? res.request.find("Host")->second : "localhost";
        std::string contents {};
        res.request.insert({ "Method", method });
        res.request.insert({ "Path", path });
        res.request.insert({ "Protocol", proto });
        res.request.insert({ "IP-Address", m_Socket.GetAddress(socketIndex).data() });
        if (!res.request.contains("Host")) res.request.insert({ "Host", host });

        if (method == "HEAD") HandleHeadRequest(&res);
        else if (method == "GET") HandleGetRequest(&res);
        else if (method == "POST") HandlePostRequest(&res);
        else return; //unknown request, close without response
        LogRequest(&res);
        m_Socket.Send(socketIndex, GetResponseString(&res));
    }

    void HttpServer::OnAccept(ServerSocket *server, int index) {
    }

    void HttpServer::OnRecv(ServerSocket *server, int socketIndex) {
        std::string_view buffer = server->GetBuffer(socketIndex);
        int lines = 0;
        for (auto const& ch : buffer) if (ch == '\n') lines++;
        if (lines < 2) return;
        HandleRequest(socketIndex, buffer);
        server->Close(socketIndex);
    }

    void HttpServer::Tick() {
        m_Socket.Accept();
        m_Socket.RecvAll();
    }
}
