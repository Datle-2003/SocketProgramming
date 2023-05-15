#ifndef CLIENT_H
#define CLIENT_H
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <iostream>
#include <cstring>
#include <vector>
#include <string>
#include <TlHelp32.h>
#include <conio.h>
#include <unordered_set>
#include <fstream>
#include <stdio.h>



const int IMAGE_LENGTH = 2048;
const int BUFFER_LENGTH = 10000;
extern char Buffer[BUFFER_LENGTH];

class Client
{
private:
    SOCKET _ClientSocket;
    sockaddr_in _serverAddr;

public:
    Client();
    Client(int Port, const char* IPAddress);
    ~Client();

    void setServerAddress(int, const char*);
    int initializeWinsock();
    int createSocket();
    int connectToServer();
    int sendData();
    int receiveData();
    int receiveImage();
    void catchKeyInput();
    void clearBuffer();
};


#endif
