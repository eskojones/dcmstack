#ifndef DCM_CLIENT_SOCKET_H
#define DCM_CLIENT_SOCKET_H
#include <iostream>
#include <map>
#include <fmt/core.h>

#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>


namespace dcm {

    class ClientSocket {

        public:
            enum SocketType { TCP, UDP };
            bool m_Connected { false };
            int m_BufferSize { 1024 }; //bytes allocated for m_Buffer
            int m_BufferUsed { 0 };    //bytes Recv()'d on last call that yielded data

            ClientSocket();
            explicit ClientSocket(SocketType type);
            ~ClientSocket();
            [[nodiscard]] bool Connect (std::string_view host, int16_t port);
            bool Close ();
            [[nodiscard]] bool SetBlocking (bool blocking) const;
            bool Send (std::string_view buf);
            int Recv ();
            std::string_view GetBuffer();


        private:
            SocketType m_Type { TCP };
            int m_Descriptor { 0 };
            struct sockaddr_in m_Destination { };
            char *m_Buffer { nullptr };

            void Alloc(int size);
    };

}

#endif //DCM_CLIENT_SOCKET_H
