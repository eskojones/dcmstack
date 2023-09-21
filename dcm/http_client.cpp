#include "http_client.h"

namespace dcm {
    HttpClient::HttpClient(std::string_view host, std::string_view path, int port)
        : m_Host { host }, m_Path { path }, m_Port { port } {
        if (m_Socket.Connect(m_Host, static_cast<int16_t>(m_Port))) {
            m_Socket.Send(fmt::format("GET {} HTTP/1.0\r\n\r\n", m_Path));
        }
    }

    HttpClient::~HttpClient() {
        m_Socket.Close();
    }

    void HttpClient::ParseHeaders(const std::vector<std::string>& headerLines) {
        for (auto const& line : headerLines) {
            string s { line };
            std::vector<string> words = s.split(":");
            if (words.size() < 2) continue;
            m_Headers.insert({
                                 words[0].filter("\n\r").trim().rtrim().data,
                                 words[1].filter("\n\r").trim().rtrim().data
                             });
        }
    }

    void HttpClient::ParseResponse(std::string_view res) {
        std::string line { };
        char last;
        bool isHeader = true;
        std::vector<std::string> headerLines { };
        std::vector<std::string> bodyLines { };
        for (int i = 0; i < res.size(); i++) {
            char ch = res[i];
            if (isHeader) {
                if (ch == '\n') {
                    headerLines.push_back(line);
                    line.clear();
                    if ((res[i-1] == '\n' || res[i-1] == '\r')
                        && (res[i-2] == '\n' || res[i-2] == '\r')) {
                        isHeader = false;
                    }
                } else line.push_back(ch);
            } else {
                if (ch == '\n') {
                    bodyLines.push_back(line);
                    line.clear();
                } else line.push_back(ch);
            }
        }
        ParseHeaders(headerLines);
        m_Body.clear();
        for (auto const& bodyLine : bodyLines) m_Body.append(bodyLine);
    }

    void HttpClient::Finalise() {
        if (m_Complete) return;
        int bytes = m_Socket.Recv();
        while (bytes == 0) {
            bytes = m_Socket.Recv();
            usleep(1000);
        }
        if (bytes < 0) return;
        ParseResponse(m_Socket.GetBuffer());
        m_Complete = true;
        m_Socket.Close();
    }

    std::string_view HttpClient::GetResponse() {
        if (!m_Complete) Finalise();
        return m_Body;
    }

    std::map<std::string,std::string> HttpClient::GetHeaders() const {
        return m_Headers;
    }

    std::string_view HttpClient::GetHeader(const std::string& name) {
        return m_Headers[name];
    }
}