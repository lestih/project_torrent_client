#include "tcp_connect.h"
#include "byte_tools.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <chrono>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <limits>
#include <utility>

TcpConnect::TcpConnect(std::string ip, int port, 
                       std::chrono::milliseconds connectTimeout,
                       std::chrono::milliseconds readTimeout)
: ip_(ip),
  port_(port),
  connectTimeout_(connectTimeout),
  readTimeout_(readTimeout) {}

TcpConnect::~TcpConnect() {
    CloseConnection();
}

void TcpConnect::EstablishConnection() {
    socket_ = socket(AF_INET, SOCK_STREAM, 0);
    
    if(socket_ < 0) {
        throw std::runtime_error("Socket creation error");
    }

    int flags = fcntl(socket_, F_GETFL, 0);
    if (flags == -1) {
        close(socket_);
        throw std::runtime_error("fcntl F_GETFL error");
    }

    if (fcntl(socket_, F_SETFL, flags | O_NONBLOCK) == -1) {
        close(socket_);
        throw std::runtime_error("fcntl F_SETFL error");
    }

    struct timeval timeout;      
    timeout.tv_sec = readTimeout_.count() / 1000;
    timeout.tv_usec = readTimeout_.count() % 1000 * 1000;
    
    if (setsockopt (socket_, SOL_SOCKET, SO_RCVTIMEO, &timeout,
                sizeof(timeout)) < 0) {
        close(socket_);
        throw std::runtime_error("setsockopt error");
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_); 
    inet_pton(AF_INET, ip_.data(), &addr.sin_addr);

    int connect_result = connect(socket_, (sockaddr*)&addr, sizeof(addr));

    if (connect_result < 0 && errno != EINPROGRESS) {  // при EINPROGRESS поток не блокируется
        close(socket_);
        throw std::runtime_error("Connect error");
    }

    fd_set write_fds;
    FD_ZERO(&write_fds);
    FD_SET(socket_, &write_fds);

    timeval connect_timeval{};
    connect_timeval.tv_sec = connectTimeout_.count() / 1000;
    connect_timeval.tv_usec = (connectTimeout_.count() % 1000) * 1000;
    int select_result = select(socket_ + 1, nullptr, &write_fds, nullptr, &connect_timeval);

    if (select_result <= 0) {
        close(socket_);
        throw std::runtime_error("ошибка select");
    }

    fcntl(socket_, F_SETFL, flags & ~O_NONBLOCK); 
}

void TcpConnect::SendData(const std::string& data) const {
    const char* data_ = data.data();
    size_t len = data.size();
    int sent_bytes = send(socket_, data_, len, 0);
    if (sent_bytes <= 0) {
        throw std::runtime_error("ошибка send");
    }
}

std::string TcpConnect::ReceiveData(size_t bufferSize) const {
    char length_buffer[4];
    long long message_length = bufferSize;

    if (bufferSize == 0) {
        int received_bytes = recv(socket_, length_buffer, sizeof(length_buffer), MSG_WAITALL);
        if (received_bytes < 4) {
            throw std::runtime_error("Receive length error");
        }
        message_length = BytesToInt(length_buffer); 
    }

    std::string result;
    result.resize(message_length);
    char* data = result.data();

    int received_bytes = recv(socket_, data, message_length, MSG_WAITALL);
    if (received_bytes < message_length) {
        throw std::runtime_error("Receive data error");
    }

    return result;
}

void TcpConnect::CloseConnection() {
    close(socket_);
}

const std::string &TcpConnect::GetIp() const {
    return ip_;
}

int TcpConnect::GetPort() const {
    return port_;
}
