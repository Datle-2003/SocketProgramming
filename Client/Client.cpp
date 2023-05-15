#include "Client.h"

char Buffer[BUFFER_LENGTH];
char Img_Buffer[IMAGE_LENGTH];
const int DEFAULT_PORT = 8080;

Client::Client()
{
    _ClientSocket = INVALID_SOCKET;
    _serverAddr.sin_family = AF_INET;
    _serverAddr.sin_port = htons(DEFAULT_PORT);
    inet_pton(AF_INET, "127.0.0.1", &_serverAddr.sin_addr);
    memset(Buffer, 0, BUFFER_LENGTH);
}

Client::Client(int Port, const char* IPAddress)
{
    _ClientSocket = INVALID_SOCKET;
    _serverAddr.sin_family = AF_INET;
    _serverAddr.sin_port = htons(Port);
    inet_pton(AF_INET, IPAddress, &_serverAddr.sin_addr);
    memset(Buffer, 0, BUFFER_LENGTH);
}

Client::~Client()
{
    closesocket(_ClientSocket);
    WSACleanup();
}

void Client::setServerAddress(int port, const char* IPAddress) {
        _ClientSocket = INVALID_SOCKET;
        _serverAddr.sin_family = AF_INET;
        _serverAddr.sin_port = htons(port);
        inet_pton(AF_INET, IPAddress, &_serverAddr.sin_addr);
        memset(Buffer, 0, BUFFER_LENGTH);
}

int Client::initializeWinsock()
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

int Client::createSocket()
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

int Client::connectToServer()
{
    initializeWinsock();
    createSocket();
    // connect to server
    if (connect(_ClientSocket, (sockaddr*)&_serverAddr, sizeof(_serverAddr)) == SOCKET_ERROR)
    {
        printf("Failed to connect: %d\n", WSAGetLastError());
        return 1;
    }
    return 0;
}

int Client::sendData()
{
    
    while (1) {
        std::cin.getline(Buffer, BUFFER_LENGTH);
        if (strcmp(Buffer, "\0") != 0)
            break;
        else {
            std::cout << "Empty choice, please try again\nEnter choice: ";
        }
    }

    int sendResult = send(_ClientSocket, Buffer, strlen(Buffer), 0);
    if (sendResult == SOCKET_ERROR)
    {
        printf("Failed to send: %d\n", WSAGetLastError());
        return 1;
    }

    return 0;
}

int Client::receiveData()
{
    clearBuffer();
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
    std::cout << Buffer;
    return 0;
}

int Client::receiveImage()
{
    clearBuffer();
    std::ofstream file("image.bmp", std::ios::binary | std::ios::out);
    if (!file.is_open()) {
        std::cerr << "Error creating BMP file: " << '\n';
        return 1;
    }
    int result = IMAGE_LENGTH;
    while (result == IMAGE_LENGTH)
    {
        result = recv(_ClientSocket, Img_Buffer, IMAGE_LENGTH, 0);
        if (result == SOCKET_ERROR)
        {
            std::cerr << "Error receiving data from server: " << WSAGetLastError() << '\n';
            return SOCKET_ERROR;
        }
        if (result == 0) {
            break; // End of data received
        }
        file.write(Img_Buffer, result);
    }
    file.close();
    return 0;

}

void Client::clearBuffer() {
    memset(Buffer, 0, BUFFER_LENGTH);
}

void Client::catchKeyInput()
{
   while (true)
    {
        int ch = _getch();

        // Exit if ESCAPE key is pressed
        if (ch == VK_ESCAPE)
        {
           // cout << "break";
            send(_ClientSocket, "stop", 4, 0);
           break;
        }
        std::string str_ch = std::to_string(ch);
        const char* char_num = str_ch.c_str();
        int bytes_sent = send(_ClientSocket, char_num, str_ch.length(), 0);
        if (bytes_sent < 0) {
            std::cerr << "Error sending data" << std::endl;
            break;
        }
    }
}


