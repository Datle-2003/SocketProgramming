#ifndef CLIENT_H
#define CLIENT_H
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <iostream>
#include <cstring>

using std::cin;
using std::cout;


class Client
{
private:
    SOCKET _ClientSocket;
    int _Port;
    char* _ServerIPAddress;

public:
    Client();
    Client(int Port, const char* IPAddress);
    ~Client();
    int InitWinsock();
    int CreateSocket();
    int ConnectToServer();
    int SendToServer();
    int ReceiveFromServer();
    void Communicate();
};

#endif
