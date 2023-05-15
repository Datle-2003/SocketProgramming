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
#include <filesystem>

namespace fs = std::filesystem;

const int BUFFER_LENGTH = 10000;
const int IMAGE_LENGTH = 2048;
extern char Buffer[BUFFER_LENGTH];

class Server
{
private:
    SOCKET _ServerSocket;
    sockaddr_in serverAddress;

public:
    Server(int port, char *ipAddress);
    Server();
    ~Server();
    void setServerAddress(int port, const char *ipAddress);
    void setup();

    int initializeWinsock();
    int createSocket();
    int bindSocket();
    int listenForClient();
    SOCKET acceptConnect();

    void clearBuffer();
    void sendInvalidChoiceMessage(SOCKET &Client);
    void sendExitMessage(SOCKET& Client);

    void processOptions(SOCKET &Client);
    void processExitOption(SOCKET &Client);
    void processEnumerateAppsOption(SOCKET &Client);
    void processEnumerateProcessesOption(SOCKET &Client);
    void processCatchKeyPressOption(SOCKET &Client);
    void processTakeScreenshotOption(SOCKET &Client);
    void traversingDirectoryTree(SOCKET &Client);

    int sendData(SOCKET ClientSocket, const char *Data, int length);
    int receiveData(SOCKET ClientSocket);

    int catchKeyPress(SOCKET ClientSocket);

    std::string enumerateApps(HKEY HKeyRoot, LPCSTR SubKey);
    std::string enumerateRunningProcesses();

    std::string startProcess(const char *);
    std::string stopProcess(const char *);

    void takeScreenshot(std::string filename);

    std::string changeDirectory(const std::string &newPath);
    std::string list();
    std::string changeName(const std::string &path, const std::string &new_name);
    std::string moveFile(const std::string &src, const std::string &dest);
    std::string changeToParentDirectory();
    std::string copyFile(const std::string &src, const std::string &dest);
    std::string deleteFile(const std::string &path);
};

#endif