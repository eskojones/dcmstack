#include "server_socket.h"

namespace dcm {

    bool ServerSocket::SetBlocking (int fd, bool blocking) {
        #ifdef _WIN32
            int mode = blocking ? 1 : 0;
            return ioctlsocket(fd, FIONBIO, &mode) == 0;
        #else
            int flags = fcntl(fd, F_GETFL, 0);
            if (flags == -1) return false;
            flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
            return fcntl(fd, F_SETFL, flags) == 0;
        #endif
    }

    void ServerSocket::Alloc(int size) {
        m_BufferSize = size;
        m_Buffers = (char *)malloc(m_BufferSize * 256 * sizeof(char));
    }

    void ServerSocket::Event (const std::string& ev, int index) {
        for (auto const& fn : m_EventHandlers.find(ev)->second) {
            fn(this, index);
        }
    }

    ServerSocket::ServerSocket() : m_Type { TCP } {
        Alloc(m_BufferSize);
    }

    ServerSocket::ServerSocket(SocketType type) : m_Type { type } {
        Alloc(m_BufferSize);
    }

    ServerSocket::~ServerSocket() {
        free(m_Buffers);
    }

    bool ServerSocket::IsListening() const {
        return m_Listening;
    }

    bool ServerSocket::IsValidSocketIndex(int socketIndex) {
        if (socketIndex < 0 || socketIndex > 255 || !m_Connected[socketIndex]) {
            SetFailure(1, socketIndex, "Invalid client socket");
            return false;
        }
        return true;
    }

    struct SocketFailureInfo *ServerSocket::GetFailure() {
        return &m_Failure;
    }

    void ServerSocket::SetFailure(int errorCode, int socketIndex, std::string_view message) {
        m_Failure.ErrorCode = errorCode;
        m_Failure.SocketIndex = socketIndex;
        m_Failure.Message = message;
    }

    std::string_view ServerSocket::GetAddress(int socketIndex) {
        if (!IsValidSocketIndex(socketIndex)) return "";
        return std::string_view { m_Address[socketIndex] };
    }

    std::string_view ServerSocket::GetBuffer(int socketIndex) {
        if (!IsValidSocketIndex(socketIndex)) return std::string_view { "" };
        return std::string_view { (char *)(m_Buffers + (socketIndex * m_BufferSize)) };
    }

    void ServerSocket::AddHandler (const std::string& event, const std::function<void(ServerSocket*,int)>& fn) {
        if (!m_EventHandlers.contains(event)) {
            m_EventHandlers.insert({ event, { } });
        }
        m_EventHandlers.find(event)->second.push_back(fn);
    }

    bool ServerSocket::Listen(int port) {
        m_Listening = false;
        m_Port = port;
        m_ListenDescriptor = socket(AF_INET, m_Type == TCP ? SOCK_STREAM : SOCK_DGRAM, 0);
        memset(&m_ListenSocket, 0, sizeof(struct sockaddr_in));
        m_ListenSocket.sin_family = AF_INET;
        m_ListenSocket.sin_addr.s_addr = htonl(INADDR_ANY);
        m_ListenSocket.sin_port = htons(m_Port);
        if (bind(m_ListenDescriptor, reinterpret_cast<const sockaddr *>(&m_ListenSocket), sizeof(struct sockaddr_in)) != 0) {
            SetFailure(2, -1, "bind() failed");
            Event("listen-failure", -1);
            return false;
        }
        if (listen(m_ListenDescriptor, 255) != 0) {
            SetFailure(3, -1, "listen() failed");
            Event("listen-failure", -2);
            return false;
        }
        SetBlocking(m_ListenDescriptor, false);
        m_Listening = true;
        Event("listen", 0);
        return true;
    }

    int ServerSocket::Accept() {
        if (!m_Listening) {
            SetFailure(4, -1, "Not listening");
            Event("accept-failure", -1);
            return -1;
        }
        for (int i = 0; i < 256; i++) {
            if (m_Connected[i]) continue;
            int len = sizeof(sockaddr);
            int fd = accept(m_ListenDescriptor,
                            reinterpret_cast<sockaddr *>(&m_ClientSockets[i]),
                            reinterpret_cast<socklen_t *>(&len));
            if (fd < 0) {
                SetFailure(5, -1, "No connection to accept");
                Event("accept-failure", -2);
                return -2;
            }
            m_Connected[i] = true;
            m_Descriptors[i] = fd;
            uint8_t a = (m_ClientSockets[i].sin_addr.s_addr & 0xff000000) >> 24;
            uint8_t b = (m_ClientSockets[i].sin_addr.s_addr & 0x00ff0000) >> 16;
            uint8_t c = (m_ClientSockets[i].sin_addr.s_addr & 0x0000ff00) >> 8;
            uint8_t d = (m_ClientSockets[i].sin_addr.s_addr & 0x000000ff);

            m_Address[i] = fmt::format("{}.{}.{}.{}:{}", d, c, b, a, m_ClientSockets[i].sin_port);
            SetBlocking(m_Descriptors[i], false);
            Event("accept", i);
            return i;
        }
        SetFailure(6, -1, "No free client sockets");
        Event("accept-failure", -3);
        return -3;
    }

    bool ServerSocket::Close(int socketIndex) {
        if (!IsValidSocketIndex(socketIndex)) return false;
        int ret = close(m_Descriptors[socketIndex]);
        if (ret == 0) {
            Event("close", socketIndex);
            m_Connected[socketIndex] = false;
        } else {
            SetFailure(7, socketIndex, "close() failed");
            Event("close-failure", socketIndex);
        }
        return ret == 0;
    }

    bool ServerSocket::Send (int socketIndex, const char *buf, int len) {
        if (!IsValidSocketIndex(socketIndex)) {
            Event("send-failure", -socketIndex);
            return false;
        }
        bool ret = send(m_Descriptors[socketIndex], buf, len, 0) > -1;
        if (!ret) {
            SetFailure(8, socketIndex, "Client socket is disconnected");
            Event("send-failure", socketIndex);
            Close(socketIndex);
        } else Event("send", socketIndex);
        return ret;
    }

    bool ServerSocket::Send (int socketIndex, std::string_view str) {
        return Send(socketIndex, str.data(), static_cast<int>(str.size()));
    }

    int ServerSocket::Recv (int socketIndex) {
        if (!IsValidSocketIndex(socketIndex)) {
            Event("recv-failure", -1);
            return -1;
        }
        char tmp[m_BufferSize];
        ssize_t bytes = recv(m_Descriptors[socketIndex], tmp, m_BufferSize, 0);
        if (bytes < 0) {
            if (errno == EAGAIN) return 0;
            if (errno == EWOULDBLOCK) return 0;
            SetFailure(9, socketIndex, "Client socket is disconnected");
            Event("recv-failure", socketIndex);
            Close(socketIndex);
            return -2;
        } else if (bytes > 0) {
            char *buf = (char *)(m_Buffers + (socketIndex * m_BufferSize));
            m_BufferUsed[socketIndex] = static_cast<int>(bytes);
            memcpy(buf, tmp, bytes);
            buf[m_BufferUsed[socketIndex]] = 0;
            Event("recv", socketIndex);
        }
        return static_cast<int>(bytes);
    }

    void ServerSocket::RecvAll() {
        for (int i = 0; i < 256; i++) Recv(i);
    }

    int ServerSocket::Broadcast(const char *buf, int len) {
        int sent = 0;
        for (int j = 0; j < 256; j++) {
            if (!m_Connected[j]) continue;
            Send(j, buf, len);
            sent++;
        }
        return sent;
    }

    int ServerSocket::Broadcast(std::string_view str) {
        return Broadcast(str.data(), static_cast<int>(str.size()));
    }

    bool ServerSocket::IsConnected (int socketIndex) const {
        return m_Connected[socketIndex];
    }

}
