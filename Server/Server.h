#ifndef SERVER_H
#define SERVER_H
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <iostream>
#include <string>
#include <TlHelp32.h>
#include <conio.h>
#include <unordered_set>
#include <fstream>
#include <vector>


using std::cerr;
using std::cin;
using std::cout;
using std::string;
using std::unordered_set;

enum process
{
    LIST_APP = 1,
    LIST_PROCESS = 2,
    SCREENSHOOT = 3,
    KEYPRESS = 4,
    TREE_DIRECTORY = 5
};


class Server
{
private:
    SOCKET _ServerSocket;
    int _Port;
    char* _IPAddress;

public:
    Server(int Port, char* IPAddress);
    Server();
    ~Server();
    int initWinsock();
    int createSocket();
    int bindSocket();
    int listenSocket();
    SOCKET acceptConnect();
    int sendData(SOCKET ClientSocket, const char* Data);
    int receiveData(SOCKET ClientSocket, char* Buffer, int Length);
    int catchKeyPress(SOCKET ClientSocket);
    string EnumerateApps(HKEY HKeyRoot, LPCSTR SubKey);
    int startApp(const char* Name);
    void ListRunningProcesses();
    int StopProcess(string);
    void TakeScreenshot(string filename);
};

#endif