// test_socket_server.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
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

    auto listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_fd == INVALID_SOCKET) {
        std::cout << "socket err: " << WSAGetLastError() << std::endl;
        return 0;
    }

    // 设置非阻塞
    u_long blocking = 0;
    ioctlsocket(listen_fd, FIONBIO, &blocking);

    do
    {
        sockaddr_in sa_server;
        sa_server.sin_family = AF_INET;
        inet_pton(AF_INET, "127.0.0.1", &sa_server.sin_addr);
        sa_server.sin_port = htons(30000);
        err = bind(listen_fd, reinterpret_cast<const sockaddr*>(&sa_server), sizeof(sa_server));
        if (SOCKET_ERROR == err) {
            std::cout << "bind err: " << WSAGetLastError() << std::endl;
            break;
        }

        err = listen(listen_fd, SOMAXCONN);
        if (SOCKET_ERROR == err) {
            std::cout << "listen err:" << WSAGetLastError() << std::endl;
            break;
        }

        // 一直在接受数据包，每次接受一个数据包连接
        fd_set readers, writers;
        FD_ZERO(&readers);
        FD_ZERO(&writers);
        FD_SET(listen_fd, &readers);
        
        while (true) {
            timeval t = { 5, 0 };
            switch (select(0, &readers, &writers, nullptr, &t)) {
            case 0:
                std::cout << "select timeout" << std::endl;
                break;
            case SOCKET_ERROR:
                std::cout << "select error: " << WSAGetLastError() << std::endl;
                break;
            default:
                // 如果是监听socket，一定是接受到了连接
                if (FD_ISSET(listen_fd, &readers)) {
                    sockaddr_in conn_addr;
                    int conn_addr_len = sizeof(conn_addr);
                    auto conn_fd = accept(listen_fd, reinterpret_cast<sockaddr*>(&conn_addr), &conn_addr_len);
                    if (conn_fd == INVALID_SOCKET) {
                        std::cout << "accept err:" << WSAGetLastError() << std::endl;
                        continue;
                    }

                    FD_SET(conn_fd, &readers);
                    FD_SET(conn_fd, &writers);
                }

                FD_SETSIZE;
            }
            

            char ip[20] = { 0 };
            inet_ntop(AF_INET, &conn_addr.sin_addr, ip, sizeof(ip));

            std::cout << "new connection, ip: " << ip << ", port: " << ntohs(conn_addr.sin_port) << std::endl;

            std::string response;
            std::array<char, 10> buf;
            int recv_len = 0;
            do {
                // buffer没填满，会一直阻塞
                recv_len = recv(conn_fd, &buf[0], 10, 0);
                if (recv_len > 0)
                    response.append(buf.data(), recv_len);
            } while (recv_len > 0);

            err = shutdown(conn_fd, SD_RECEIVE);
            if (SOCKET_ERROR == err) {
                std::cout << "shutdown err: " << WSAGetLastError() << std::endl;
                continue;
            }

            std::string msg = "server msg: keen";
            err = send(conn_fd, msg.c_str(), msg.size(), 0);
            if (SOCKET_ERROR == err) {
                std::cout << "send err: " << WSAGetLastError() << std::endl;
                continue;
            }

            std::cout << response << std::endl;

            err = shutdown(conn_fd, SD_SEND);
            if (SOCKET_ERROR == err) {
                std::cout << "shutdown err: " << WSAGetLastError() << std::endl;
                continue;
            }
        }
    } while (false);

    closesocket(listen_fd);
    WSACleanup();
}
