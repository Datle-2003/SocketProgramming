#include "Server.h"

const int DEFAULT_PORT = 8080;
const char* DEFAULT_IPADDRESS = "172.20.46.145";
const int BUFFER_LENGTH = 2048;
char Buffer[BUFFER_LENGTH];



Server::Server(int Port, char* IPAddress)
{
    _Port = Port;
    _IPAddress = new char[strlen(IPAddress) + 1];
    strcpy_s(_IPAddress, strlen(IPAddress) + 1, IPAddress);
    // set buffer to all zeros
    memset(Buffer, 0, BUFFER_LENGTH);
    _ServerSocket = INVALID_SOCKET;
}

Server::Server()
{
    _Port = DEFAULT_PORT;
    _IPAddress = new char[strlen(DEFAULT_IPADDRESS) + 1];
    strcpy_s(_IPAddress, strlen(DEFAULT_IPADDRESS) + 1, DEFAULT_IPADDRESS);
    // set buffer to all zeros
    memset(Buffer, 0, BUFFER_LENGTH);
    _ServerSocket = INVALID_SOCKET;
}

Server::~Server()
{
    closesocket(_ServerSocket);
    WSACleanup();
    delete[] _IPAddress;
}

int Server::initWinsock()
{
    WSADATA wsadata;
    WORD wVersionRequested = MAKEWORD(2, 2);
    int result = WSAStartup(wVersionRequested, &wsadata);
    if (result != 0)
    {
        std::cerr << "WSAStartup failed: " << '\n';
        std::cerr << "The status: " << wsadata.szSystemStatus << '\n';
        return 1;
    }
    return 0;
}

int Server::createSocket()
{
    _ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (_ServerSocket == INVALID_SOCKET)
    {
        std::cerr << "Error at socket: " << WSAGetLastError() << '\n';
        return 1;
    }
    return 0;
}

int Server::bindSocket()
{
    // Bind the socket to an address and port number
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    inet_pton(AF_INET, _IPAddress, &serverAddress.sin_addr.s_addr);
    serverAddress.sin_port = htons(_Port);
    if (bind(_ServerSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
    {
        std::cerr << "Error binding socket: " << WSAGetLastError() << '\n';
        return 1;
    }
    return 0;
}

int Server::listenSocket()
{
    if (listen(this->_ServerSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "Error listening on socket: " << WSAGetLastError() << std::endl;
        return 1;
    }
    return 0;
}

SOCKET Server::acceptConnect()
{
    // Accept a new client connection
    sockaddr_in clientAddress;
    int clientAddressSize = sizeof(clientAddress);
    SOCKET clientSocket = accept(this->_ServerSocket, (sockaddr*)&clientAddress, &clientAddressSize);
    if (clientSocket == INVALID_SOCKET)
    {
        std::cerr << "Error accepting client connection: " << WSAGetLastError() << '\n';
        return INVALID_SOCKET;
    }

    // Return the handle to the client socket
    return clientSocket;
}

int Server::sendData(SOCKET clientSocket, const char* data)
{
    int dataLength = strlen(data);
    int bytesSent = ::send(clientSocket, data, dataLength, 0);
    if (bytesSent == SOCKET_ERROR)
    {
        std::cerr << "Error sending data to client: " << WSAGetLastError() << '\n';
        return SOCKET_ERROR;
    }
    return bytesSent;
}
int Server::receiveData(SOCKET clientSocket, char* buffer, int length)
{
    int bytesReceived = ::recv(clientSocket, buffer, length, 0);
    if (bytesReceived == SOCKET_ERROR)
    {
        std::cerr << "Error receiving data from client: " << WSAGetLastError() << '\n';
        return SOCKET_ERROR;
    }
    return bytesReceived;
}

int Server::catchKeyPress(SOCKET clientSocket)
{
    int result = 0;
    bool stop = false;
    while (!stop)
    {
        // Check if a key has been pressed
        if (_kbhit())
        {
            char ch = _getch();
            std::cout << "Key pressed: " << ch << '\n';

            // Send the key press to the client
            char msg[2] = { ch, '\0' };
            result = send(clientSocket, msg, 2, 0);
            if (result == SOCKET_ERROR)
            {
                std::cerr << "Error sending message to client: " << WSAGetLastError() << '\n';
                return 1;
            }
        }

        // Check if the client has sent the "stop" message
        char buffer[BUFFER_LENGTH];
        result = recv(clientSocket, buffer, BUFFER_LENGTH, 0);
        if (result == SOCKET_ERROR)
        {
            std::cerr << "Error receiving message from client: " << WSAGetLastError() << '\n';
            return 1;
        }
        else if (result == 0)
        {
            std::cout << "Client disconnected" << '\n';
            return 0;
        }
        else
        {
            buffer[result] = '\0';
            if (strcmp(buffer, "stop") == 0)
            {
                std::cout << "Received 'stop' message from client" << std::endl;
                stop = true;
            }
        }
    }

    return 0;
}

string Server::EnumerateApps(HKEY hKeyRoot, LPCSTR subKey)
{
    // Khi liệt kê có một số app bị trùng nên lưu vào set
    unordered_set<string> s;
    HKEY hKey;
    if (RegOpenKeyExA(hKeyRoot, subKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        DWORD index = 0;
        CHAR keyName[255];
        DWORD keyNameSize = sizeof(keyName);
        FILETIME lastWriteTime;

        while (RegEnumKeyExA(hKey, index++, keyName, &keyNameSize, NULL, NULL, NULL, &lastWriteTime) == ERROR_SUCCESS)
        {
            HKEY hSubKey;
            if (RegOpenKeyExA(hKey, keyName, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS)
            {
                CHAR displayName[255];
                DWORD displayNameSize = sizeof(displayName);
                DWORD type;

                if (RegQueryValueExA(hSubKey, "DisplayName", NULL, &type, (LPBYTE)displayName, &displayNameSize) == ERROR_SUCCESS)
                    s.insert(displayName);

                RegCloseKey(hSubKey);
            }
            keyNameSize = sizeof(keyName);
        }

        RegCloseKey(hKey);
    }

    string appNames;
    for (const auto& app : s)
        appNames += app + '\n';
    return appNames;
}

/*
// Đang bị lỗi
int Server::startApp(const char* name)
{
    std::string appName(name);
    int bufferSize = MultiByteToWideChar(CP_UTF8, 0, appName.c_str(), -1, NULL, 0);
    std::vector<WCHAR> wideBuffer(bufferSize);
    MultiByteToWideChar(CP_UTF8, 0, appName.c_str(), -1, wideBuffer.data(), bufferSize);

    // Start the application
    if (!ShellExecute(NULL, L"open", wideBuffer.data(), NULL, NULL, SW_SHOWNORMAL))
    {
        std::cout << "Error starting application " << appName << '\n';
        return 1;
    }
    return 0;
}
*/

void Server::ListRunningProcesses()
{
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        std::cerr << "Failed to create snapshot" << std::endl;
        return;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32))
    {
        std::cerr << "Failed to get first process" << std::endl;
        CloseHandle(hProcessSnap);
        return;
    }

    do
    {
        std::cout << "Process: " << pe32.szExeFile << std::endl;
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
}

int Server::StopProcess(std::string ProcessName)
{
    std::string command = "taskkill /F /IM " + ProcessName;
    int result = system(command.c_str());

    if (result == 1)
    {
        std::cout << "Command failed to execute." << std::endl;
        return 1;
    }

    return 0;
}

void Server::TakeScreenshot(string fileName) {
    fileName = fileName + ".bmp";
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);

    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);

    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);

    BitBlt(hdcMem, 0, 0, width, height, hdcScreen, 0, 0, SRCCOPY);
    hBitmap = (HBITMAP)SelectObject(hdcMem, hOldBitmap);

    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);

    BITMAPFILEHEADER bmfHeader;
    BITMAPINFOHEADER bi;

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmp.bmWidth;
    bi.biHeight = bmp.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    DWORD dwBmpSize = ((bmp.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmp.bmHeight;

    bmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bmfHeader.bfSize = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bmfHeader.bfType = 'B' + ('M' << 8);

    char* lpbitmap = new char[dwBmpSize];
    GetDIBits(hdcScreen, hBitmap, 0, height, lpbitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    std::ofstream file(fileName, std::ios::out | std::ios::binary);

    file.write((char*)&bmfHeader, sizeof(BITMAPFILEHEADER));
    file.write((char*)&bi, sizeof(BITMAPINFOHEADER));
    file.write(lpbitmap, dwBmpSize);

    file.close();

    delete[] lpbitmap;
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);
}
