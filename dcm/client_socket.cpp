#include "client_socket.h"


namespace dcm {

    void ClientSocket::Alloc(int size) {
        m_BufferSize = size;
        m_Buffer = (char *)malloc(size);
    }

    ClientSocket::ClientSocket() : m_Type { TCP } {
        Alloc(16384);
    }

    ClientSocket::ClientSocket(SocketType type) : m_Type { type } {
        Alloc(16384);
    }

    ClientSocket::~ClientSocket() {
        free(m_Buffer);
    }

    bool ClientSocket::Connect (std::string_view host, int16_t port) {
        if (m_Connected) Close();
        printf("ClientSocket.Connect\n");
        hostent *he = gethostbyname(host.data());
        if (he == nullptr) return false;
        m_Descriptor = socket(AF_INET, (m_Type == SocketType::TCP ? SOCK_STREAM : SOCK_DGRAM), 0);
        if (m_Descriptor < 0) return false;
        m_Destination.sin_family = AF_INET;
        m_Destination.sin_port = htons(port);
        memcpy(&m_Destination.sin_addr, he->h_addr_list[0], he->h_length);
        if (connect(m_Descriptor, (struct sockaddr *)&m_Destination, sizeof(m_Destination)) < 0) {
            fprintf(stderr, "ClientSocket.Connect: Failed to connect to host %s!\n", host.data());
            m_Connected = false;
            return false;
        }
        m_Connected = true;
        return true;
    }

    bool ClientSocket::Close () {
        int ret = close(m_Descriptor);
        if (ret == 0) {
            printf("ClientSocket.Close\n");
            m_Connected = false;
        }
        return ret == 0;
    }

    bool ClientSocket::SetBlocking (bool blocking) const {
        #ifdef _WIN32
        int mode = blocking ? 1 : 0;
            return ioctlsocket(m_Descriptor, FIONBIO, &mode) == 0;
        #else
        int flags = fcntl(m_Descriptor, F_GETFL, 0);
        if (flags == -1) return false;
        flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
        return fcntl(m_Descriptor, F_SETFL, flags) == 0;
        #endif
    }

    bool ClientSocket::Send (std::string_view buf) {
        if (!m_Connected) return false;
        bool ret = send(m_Descriptor, buf.data(), buf.size(), 0) > -1;
        if (!ret) Close();
        return ret;
    }

    int ClientSocket::Recv () {
        if (!m_Connected) return 0;
        char tmp[m_BufferSize];
        ssize_t bytes = recv(m_Descriptor, tmp, m_BufferSize, 0);
        if (bytes < 0) {
            if (errno == EAGAIN) return 0;
            if (errno == EWOULDBLOCK) return 0;
            Close();
            return -1;
        } else if (bytes > 0) {
            memcpy(m_Buffer, tmp, bytes);
            m_BufferUsed = static_cast<int>(bytes);
            m_Buffer[m_BufferUsed - 1] = 0;
        }
        return static_cast<int>(bytes);
    }

    std::string_view ClientSocket::GetBuffer() {
        return m_Buffer;
    }

}