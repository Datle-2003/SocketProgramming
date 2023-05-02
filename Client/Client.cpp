#include "Client.h"

const int DEFALUT_PORT = 8080;
const char* DEFALUT_IPADDRESS = "172.20.46.145";
const int BUFFER_LENGTH = 2048;
char Buffer[BUFFER_LENGTH];

Client::Client()
{
    _ClientSocket = INVALID_SOCKET;
    _Port = DEFALUT_PORT;
    _ServerIPAddress = new char[strlen(DEFALUT_IPADDRESS) + 1];
    strcpy_s(_ServerIPAddress, strlen(DEFALUT_IPADDRESS) + 1, DEFALUT_IPADDRESS);
    memset(Buffer, 0, BUFFER_LENGTH);
}

Client::Client(int Port, const char* IPAddress)
{
    _ClientSocket = INVALID_SOCKET;
    _Port = Port;
    _ServerIPAddress = new char[strlen(IPAddress) + 1];
    strcpy_s(_ServerIPAddress, strlen(DEFALUT_IPADDRESS) + 1, IPAddress);
    memset(Buffer, 0, BUFFER_LENGTH);
}

Client::~Client()
{
    closesocket(_ClientSocket);
    WSACleanup();
    delete[] _ServerIPAddress;
}

int Client::InitWinsock()
{
    WSADATA wsaData; // wsaData: lưu trữ thông tin về phiên bản Winsock được sử dụng
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    /* WSAStartup(wVersionRequested, lpWSAData)
    wVersionRequested: Phiên bản Winsock, ở đây là phiên bản 2.2
    lpWSAData: Lưu trữ thông tin về phiên bản
    */
    if (result != 0)
    {
        printf("WSAStartup failed: %d\n", result);
        return 1;
    }
    return 0;
}

int Client::CreateSocket()
{
    _ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    /*
    socket(int domain, int type, int protocol)
    AF_INET: IPv4
    SOCK_STREAM: truyền dữ liệu tin cậy, trình tự và giao tiếp 2 phía
    */
    if (_ClientSocket == INVALID_SOCKET)
    {
        printf("Failed to create socket: %d\n", WSAGetLastError());
        return 1;
    }
    return 0;
}

int Client::ConnectToServer()
{
    sockaddr_in serverAddr; // store information about server
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(_Port); // Use the same port as the server
    // htons(): convert port number from host byte order to network byte order
    inet_pton(AF_INET, _ServerIPAddress, &serverAddr.sin_addr);
    // inet_pton(): convert "127.0.0.1" (loopback) to binary format

    // connect to server
    if (connect(_ClientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        printf("Failed to connect: %d\n", WSAGetLastError());
        return 1;
    }
    return 0;
}

int Client::SendToServer()
{
    cout << "Enter message to send to server:\n";
    cin.getline(Buffer, BUFFER_LENGTH);

    int sendResult = send(_ClientSocket, Buffer, strlen(Buffer), 0);
    if (sendResult == SOCKET_ERROR)
    {
        printf("Failed to send: %d\n", WSAGetLastError());
        return 1;
    }

    return 0;
}

int Client::ReceiveFromServer()
{
    int recvResult = recv(_ClientSocket, Buffer, BUFFER_LENGTH, 0);
    if (recvResult == SOCKET_ERROR)
    {
        printf("Failed to receive: %d\n", WSAGetLastError());
        return 1;
    }

    if (recvResult == 0)
    {
        printf("Connection closed\n");
        return 1;
    }

    printf("Received message: %s\n", Buffer);
    return 0;
}

void Client::Communicate() {
    do {
        ReceiveFromServer();
        SendToServer();
    } while (strcmp(Buffer, "stop") != 0);
}

