#include "todo_app.h"

#include <utility>


TodoApp::Account::Account(std::string name, std::string password)
    : m_Username { std::move(name) }, m_Password { std::move(password) } {
    static int currentId = 0;
    m_Id = ++currentId;
}


