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

TcpConnect::TcpConnect(std::string ip, int port, std::chrono::milliseconds connectTimeout, std::chrono::milliseconds readTimeout): ip_(ip), port_(port),
    connectTimeout_(connectTimeout), readTimeout_(readTimeout){}

TcpConnect::~TcpConnect(){
    TcpConnect::CloseConnection();
}


void TcpConnect::EstablishConnection(){
    sock_ = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_ < 0){
        throw std::runtime_error("ошибка сокета");
    }

    int flag = fcntl(sock_, F_GETFL, 0);
    if(flag == -1){
        close(sock_);
        throw std::runtime_error("ошибка флага");
    }

    if (fcntl(sock_, F_SETFL, flag | O_NONBLOCK) == -1) {
        close(sock_);
        throw std::runtime_error("ошибка fcntl");
    }

    struct timeval timeout;      
    timeout.tv_sec = readTimeout_.count() / 1000;
    timeout.tv_usec = readTimeout_.count() % 1000 * 1000;
    
    if (setsockopt (sock_, SOL_SOCKET, SO_RCVTIMEO, &timeout,
                sizeof(timeout)) < 0) {
        close(sock_);
        throw std::runtime_error("ошибка setsockopt");
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_); 
    inet_pton(AF_INET, ip_.data(), &addr.sin_addr); //check

    int con = connect(sock_, (sockaddr*)&addr, sizeof(addr)); //check
    if(con < 0 && errno != EINPROGRESS){
        close(sock_);
        throw std::runtime_error("ошибка connect");
    }

    fd_set writefds;
    FD_ZERO(&writefds);
    FD_SET(sock_, &writefds);
    timeval time_connect{};
    time_connect.tv_sec = connectTimeout_.count() / 1000;
    time_connect.tv_usec = (connectTimeout_.count() % 1000) * 1000;
    int sel = select(sock_ + 1, nullptr, &writefds, nullptr, &time_connect);
    if(sel <= 0){
        close(sock_);
        throw std::runtime_error("ошибка select");
    }
    fcntl(sock_, F_SETFL, flag & ~O_NONBLOCK); 




}

void TcpConnect::SendData(const std::string& data) const{
    const char* data_ = data.data();
    size_t len = data.size();
    int sent = send(sock_, data_, len, 0);
    if(sent <= 0) throw std::runtime_error("ошибка send");
}

std::string TcpConnect::ReceiveData(size_t bufferSize) const{
    // pollfd fds[1];
    // fds[0].fd = sock_;
    // fds[0].events = POLLIN;

    // int con = poll(fds, 1, readTimeout_.count());

    // if(con <= 0){
    //     close(sock_);
    //     throw std::runtime_error("poll error");
    // }

    char buff_num[4]; // check
    long long len = bufferSize;
    if(bufferSize == 0){
        int rd = recv(sock_, buff_num, sizeof(buff_num), MSG_WAITALL);
        if(rd < 4) throw std::runtime_error("ошибка recv");
        len = BytesToInt(buff_num); 
    }
    std::string ans;
    ans.resize(len);
    char* date = ans.data();
    long long cnt = len;
    // while(cnt > 0){
    //     int rd = recv(sock_, date, cnt, MSG_WAITALL);
    //     if(rd <= 0) throw std::runtime_error("ошибка recv");
    //     date += rd;
    //     cnt -= rd;
    // }

    int rd = recv(sock_, date, cnt, MSG_WAITALL);
    if(rd < cnt) throw std::runtime_error("ошибка recv");
    return ans;
}

void TcpConnect::CloseConnection(){
    close(sock_);
}

const std::string &TcpConnect::GetIp() const {
    return ip_;
}

int TcpConnect::GetPort() const {
    return port_;
}
