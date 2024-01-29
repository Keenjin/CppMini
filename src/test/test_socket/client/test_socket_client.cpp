// test_socket_client.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <WinSock2.h>
#include <array>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

int main()
{
    WORD wVersionRequested = MAKEWORD(2, 2);
    WSAData data;
    auto err = WSAStartup(wVersionRequested, &data);
    if (err != 0) {
        std::cout << "WSAStartup err: " << WSAGetLastError() << std::endl;
        return 0;
    }

    auto conn_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (conn_fd == INVALID_SOCKET) {
        std::cout << "socket err: " << WSAGetLastError() << std::endl;
        return 0;
    }

    do
    {
        sockaddr_in sa_server;
        sa_server.sin_family = AF_INET;
        inet_pton(AF_INET, "127.0.0.1", &sa_server.sin_addr);
        sa_server.sin_port = htons(30000);
        err = connect(conn_fd, reinterpret_cast<const sockaddr*>(&sa_server), sizeof(sa_server));
        if (SOCKET_ERROR == err) {
            std::cout << "connect err: " << WSAGetLastError() << std::endl;
            break;
        }

        std::string msg = "client msg: keen";
        err = send(conn_fd, msg.c_str(), msg.size(), 0);
        if (SOCKET_ERROR == err) {
            std::cout << "send err: " << WSAGetLastError() << std::endl;
            break;
        }

        err = shutdown(conn_fd, SD_SEND);
        if (SOCKET_ERROR == err) {
            std::cout << "shutdown err: " << WSAGetLastError() << std::endl;
            break;
        }

        std::string response;
        std::array<char, 512> buf;
        int recv_len = 0;
        do {
            recv_len = recv(conn_fd, &buf[0], 512, 0);
            if (recv_len > 0)
                response.append(buf.data(), recv_len);
        } while (recv_len > 0);

        std::cout << response << std::endl;
    } while (false);
    
    closesocket(conn_fd);
    WSACleanup();
}
