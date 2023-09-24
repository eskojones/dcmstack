#include "todo_app.h"

#include <utility>


TodoApp::Account::Account(std::string name, std::string password)
    : m_Username { std::move(name) }, m_Password { std::move(password) } {
    static int currentId = 0;
    m_Id = ++currentId;
}

/*
bool TodoApp::Start(int port) {
    Listen(port);
    AddRoute("list-of-lists", "/", ShowListOfLists);
    AddRoute("list", "/list/:listId", ShowList);
    AddRoute("item", "/list/:listId/item/:itemId", ShowItem);
    AddRoute("create-list", "/create-list", ShowListCreate);
    AddRoute("create-item", "/list/:listId/create-item", ShowItemCreate);
    AddRoute("update-item", "/update/:itemId", ShowUpdateItem);
}
*/