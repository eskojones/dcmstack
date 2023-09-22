#include "http_server.h"

#include <utility>

namespace dcm {

    HttpServer::HttpResponse::HttpResponse () {}
    HttpServer::HttpResponse::~HttpResponse () {}


    HttpServer::HttpServer(std::function<void(HttpServer*,HttpResponse*)> router, int port) : m_Port { port }, m_RequestRouter { std::move(router) }, m_DateBuffer { "" } {
        m_Socket.AddHandler("recv", [&](ServerSocket *server, int socketIndex) { OnRecv(server, socketIndex); });
    }


    bool HttpServer::Listen(int port) {
        if (m_Socket.IsListening()) return false;
        m_Port = port;
        return m_Socket.Listen(m_Port);
    }


    bool HttpServer::Listen() {
        return Listen(m_Port);
    }


    std::string HttpServer::GetDateString(time_t *time, const std::string& fmt) {
        auto tm = *std::localtime(time);
        memset(m_DateBuffer, 0, 100);
        strftime(m_DateBuffer, 99, fmt.data(), &tm);
        return { m_DateBuffer };
    }


    void HttpServer::GenerateResponse(HttpResponse *res, const std::map<std::string,std::string>& customHeaders) {
        time_t now = time(nullptr);
        std::string responseDateStr = GetDateString(&now, "%a, %d %b %Y");

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


    void HttpServer::LogRequest(HttpResponse *res) {
        time_t now = time(nullptr);
        std::string logDateStr = GetDateString(&now, "%d/%b/%Y:%H:%M:%S %z");
        std::string log = fmt::format(
            R"({} {} {} [{}] "{} {} {}" {} {} "{}" "{}" "{}"{})",
            res->request.find("IP-Address")->second,
            "-", "-", //unused + remote_user
            logDateStr,
            res->request.find("Method")->second,
            res->request.find("Path")->second,
            res->request.find("Protocol")->second,
            res->status,
            res->body.size(),
            res->request.contains("Referer") ? res->request.find("Referer")->second : "",
            res->request.contains("User-Agent") ? res->request.find("User-Agent")->second : "Unknown",
            "0.00", //gzip ratio
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
        if (lines.size() < 2 || lines[0].empty()) return;
        std::vector<string> words = lines[0].split(" ");
        if (words.size() < 3) return;
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
        std::string host = res.request.contains("Host") ? res.request.find("Host")->second : "localhost";
        std::string contents {};
        res.request.insert({ "Method", words[0].data });
        res.request.insert({ "Path", words[1].data });
        res.request.insert({ "Protocol", words[2].data });
        res.request.insert({ "IP-Address", m_Socket.GetAddress(socketIndex).data() });
        if (!res.request.contains("Host")) res.request.insert({ "Host", host });
        std::vector<dcm::string> _path = words[1].split("/");
        for (auto const &dir : _path) res.path.push_back(dir.data);
        std::vector<dcm::string> _query { };
        std::vector<dcm::string> _args { };
        if (!_path.empty()) _query = _path[_path.size() - 1].split("?");
        if (!_query.empty()) _args = _query[_query.size() - 1].split("&");
        for (auto const& arg : _args) {
            std::vector<dcm::string> kvp = arg.split("=");
            if (kvp.size() < 2) continue;
            std::string queryname = fmt::format("query_string[{}]", kvp[0].data);
            if (res.request.contains(queryname)) {
                res.request.find(queryname)->second = kvp[1].data;
            } else {
                res.request.insert({ queryname, kvp[1].data });
            }
        }
        m_RequestRouter(this, &res);
        LogRequest(&res);
        m_Socket.Send(socketIndex, GetResponseString(&res));
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


    bool HttpServer::IsListening () {
        return m_Socket.IsListening();
    }
}
