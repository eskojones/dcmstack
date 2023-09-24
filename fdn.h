#ifndef FDN_H
#define FDN_H

#include "dcm/http_webapp.h"

class FDN : public dcm::HttpWebApp {
    public:
        class Paste {
            public:
                int Id { 0 };
                std::string Contents;
                explicit Paste();
        };

        int m_FDNVersionMajor, m_FDNVersionMinor;
        std::map<int,Paste> m_Pastes { };

        FDN();
        bool Start(int port);
        void InsertAppHeader(dcm::HttpServer::HttpResponse *res);
        void ListPastes(dcm::HttpServer *server, dcm::HttpServer::HttpResponse *res);
        void ViewPaste(dcm::HttpServer *server, dcm::HttpServer::HttpResponse *res);
        void CreatePaste(dcm::HttpServer *server, dcm::HttpServer::HttpResponse *res);
        bool IsListening() override;
        void Tick() override;

    private:

};


#endif //FDN_H
