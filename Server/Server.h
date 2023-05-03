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


const int BUFFER_LENGTH = 2048;
extern char Buffer[BUFFER_LENGTH];



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
    int sendData(SOCKET ClientSocket, const char* Data, int length);
    int receiveData(SOCKET ClientSocket);
    int catchKeyPress(SOCKET ClientSocket);
    string EnumerateApps(HKEY HKeyRoot, LPCSTR SubKey);
    int startApp(const char* Name);
    string ListRunningProcesses();
    int StopProcess(const char*);
    void TakeScreenshot(string filename);
};

#endif