#ifndef DCM_SERVER_SOCKET_H
#define DCM_SERVER_SOCKET_H
#include <iostream>
#include <map>
#include <fmt/core.h>
#include <functional>
#include <vector>

#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>

namespace dcm {
    struct SocketFailureInfo {
        int ErrorCode { 0 };
        int SocketIndex { -1 };
        std::string Message { };
    };

    class ServerSocket {
        public:
            enum SocketType { TCP, UDP };
            ServerSocket();
            explicit ServerSocket(SocketType type);
            ~ServerSocket();
            [[nodiscard]] bool IsListening() const;
            bool IsValidSocketIndex(int socketIndex);
            struct SocketFailureInfo *GetFailure();
            void SetFailure(int errorCode, int socketIndex, std::string_view message);
            std::string_view GetAddress(int socketIndex);
            std::string_view GetBuffer(int socketIndex);
            void AddHandler (const std::string& event, const std::function<void(ServerSocket*,int)>& fn);
            bool Listen(int port);
            int Accept();
            bool Close(int socketIndex);
            bool Send (int socketIndex, const char *buf, int len);
            bool Send (int socketIndex, std::string_view str);
            int Recv (int socketIndex);
            void RecvAll ();
            int Broadcast(const char *buf, int len);
            int Broadcast(std::string_view str);
            [[nodiscard]] bool IsConnected (int socketIndex) const;

        private:
            SocketType m_Type { TCP };
            int m_Port { 42069 };
            int m_ListenDescriptor { 0 };
            int m_Descriptors[256] { 0 };
            struct sockaddr_in m_ListenSocket{};
            struct sockaddr_in m_ClientSockets[256]{};
            int m_BufferSize { 4096 };
            int m_BufferUsed[256] { 0 };
            char *m_Buffers { };
            bool m_Listening { false };
            bool m_Connected[256] { false };
            std::string m_Address[256];
            std::map<std::string,std::vector<std::function<void(ServerSocket*,int)>>> m_EventHandlers;
            struct SocketFailureInfo m_Failure { };

            static bool SetBlocking (int fd, bool blocking);
            void Alloc(int size);
            void Event (const std::string& ev, int index);

    };

}
#endif //DCM_SERVER_SOCKET_H
