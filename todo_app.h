#ifndef DCM_TODO_APP_H
#define DCM_TODO_APP_H

#include <iostream>
#include "dcm/http_webapp.h"


class TodoApp : dcm::HttpWebApp {
        TodoApp () : HttpWebApp(42069) {}


        class Account {
            int m_Id;
        std::string m_Username, m_Password;
        public:
            Account(std::string name, std::string password);
    };
    public:

    private:
        std::map<std::string,Account> m_Accounts { };


};


#endif //DCM_TODO_APP_H
