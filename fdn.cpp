#include "fdn.h"


FDN::Paste::Paste() {
    static int nextId = 12345;
    Id = nextId++;
    Contents = "";
}


FDN::FDN() {
    m_FDNVersionMajor = 0;
    m_FDNVersionMinor = 1;
}


bool FDN::Start(int port) {
    if (!Listen(port)) return false;
    AddRoute("list", "/",
         [&](dcm::HttpServer *server, dcm::HttpServer::HttpResponse *res) {
             res->status = 200;
             InsertAppHeader(res);
             ListPastes(server, res);
             server->GenerateResponse(res, { { "Content-Type", "text/html" } });
        }
    );

    AddRoute("view", "/view/:pasteId",
         [&](dcm::HttpServer *server, dcm::HttpServer::HttpResponse *res) {
             res->status = 200;
             InsertAppHeader(res);
             ViewPaste(server, res);
             server->GenerateResponse(res, { { "Content-Type", "text/html" } });
         }
    );

    AddRoute("create", "/create",
         [&](dcm::HttpServer *server, dcm::HttpServer::HttpResponse *res) {
             res->status = 200;
             InsertAppHeader(res);
             CreatePaste(server, res);
             server->GenerateResponse(res, { { "Content-Type", "text/html" } });
         }
    );
    return true;
}


void FDN::InsertAppHeader(dcm::HttpServer::HttpResponse *res) {
    res->headers.insert({ "Fuck-Discord-Nitro", fmt::format("{}.{}", m_FDNVersionMajor, m_FDNVersionMinor) });
}


void FDN::ListPastes(dcm::HttpServer *server, dcm::HttpServer::HttpResponse *res) {
    res->body = "<h1>List of Pastes</h1>\n<hr/>\n";
    for (auto const & [ id, p ] : m_Pastes) {
        res->body.append(fmt::format("<li><a href=\"http://{}/view/{}\">#{}</a></li>\n", res->request.find("Host")->second, id, id));
    }
}


void FDN::ViewPaste(dcm::HttpServer *server, dcm::HttpServer::HttpResponse *res) {
    res->body = fmt::format("<h1>View Paste {}</h1>\n<hr/>\n", res->request.find("Var[pasteId]")->second);
    if (res->request.contains("Var[pasteId]")) {
        int id = std::stoi(res->request.find("Var[pasteId]")->second);
        if (m_Pastes.contains(id)) {
            res->body.append(fmt::format("ID={}\n", id));
            res->body.append(m_Pastes.find(id)->second.Contents);
        } else {
            res->body.append("Invalid ID");
        }
    }
}


void FDN::CreatePaste(dcm::HttpServer *server, dcm::HttpServer::HttpResponse *res) {
    Paste p;
    res->body = fmt::format("<h1>Create New Paste</h1>\n<hr/>\n");
    if (res->request.contains("Query[text]")) {
        p.Contents = res->request.find("Query[text]")->second;
    } else {
        return;
    }
    //serialise to disk here
    m_Pastes.insert({ p.Id, p });
    res->body.append(fmt::format("Created a new paste at <a href=\"http://{}/view/{}\">here</a>\n", res->request.find("Host")->second, p.Id));
}


bool FDN::IsListening() {
    return dcm::HttpWebApp::IsListening();
}


void FDN::Tick() {
    dcm::HttpWebApp::Tick();
}

