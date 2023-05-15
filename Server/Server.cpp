#include "Server.h"

char Buffer[BUFFER_LENGTH];
const int DEFAULT_PORT = 8080;

Server::~Server()
{
    closesocket(_ServerSocket);
    WSACleanup();
}

Server::Server(int Port, char *IPAddress)
{
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(Port);
    inet_pton(AF_INET, IPAddress, &serverAddress.sin_addr);
    // set buffer to all zeros
    memset(Buffer, 0, BUFFER_LENGTH);
    _ServerSocket = INVALID_SOCKET;
}

Server::Server()
{
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(DEFAULT_PORT);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    // set buffer to all zeros
    memset(Buffer, 0, BUFFER_LENGTH);
    _ServerSocket = INVALID_SOCKET;
}

void Server::setServerAddress(int port, const char *ipAddress)
{
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress, &serverAddress.sin_addr);
}

int Server::initializeWinsock()
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

void Server::setup()
{
    initializeWinsock();
    createSocket();
    bindSocket();
    listenForClient();
}

int Server::bindSocket()
{
    if (bind(_ServerSocket, (sockaddr *)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
    {
        std::cerr << "Error binding socket: " << WSAGetLastError() << '\n';
        return 1;
    }
    return 0;
}

int Server::listenForClient()
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
    SOCKET clientSocket = accept(this->_ServerSocket, (sockaddr *)&clientAddress, &clientAddressSize);
    if (clientSocket == INVALID_SOCKET)
    {
        std::cerr << "Error accepting client connection: " << WSAGetLastError() << '\n';
        return INVALID_SOCKET;
    }

    // Return the handle to the client socket
    return clientSocket;
}

int Server::sendData(SOCKET clientSocket, const char *data, int length)
{
    clearBuffer();
    int bytesSent = send(clientSocket, data, length, 0);
    if (bytesSent == SOCKET_ERROR)
    {
        std::cerr << "Error sending data to client: " << WSAGetLastError() << '\n';
        return SOCKET_ERROR;
    }
    return bytesSent;
}

int Server::receiveData(SOCKET clientSocket)
{
    clearBuffer();
    int bytesReceived = recv(clientSocket, Buffer, BUFFER_LENGTH, 0);
    if (bytesReceived == SOCKET_ERROR)
    {
        std::cerr << "Error receiving data from client: " << WSAGetLastError() << '\n';
        return SOCKET_ERROR;
    }
    return bytesReceived;
}

void Server::sendInvalidChoiceMessage(SOCKET &Client)
{
    const char *invalidChoiceMessage = "Invalid choice, please try again\n";

    sendData(Client, invalidChoiceMessage, strlen(invalidChoiceMessage));
}

void Server::sendExitMessage(SOCKET &Client)
{
    const char *exitMessage = "Exited\n";
    sendData(Client, exitMessage, strlen(exitMessage));
}

void Server::processExitOption(SOCKET &Client)
{
    const char *exitMessage = "Abort connection!\n";
    sendData(Client, exitMessage, strlen(exitMessage));
}

void Server::processEnumerateAppsOption(SOCKET &Client)
{
    const char *appMenu = "0.Exit\n1.List App\n2.Start App\nChoose your option: ";

    while (true)
    {
        sendData(Client, appMenu, strlen(appMenu));
        receiveData(Client);
        if (strcmp(Buffer, "0") == 0)
        {
            sendExitMessage(Client);
            break;
        }
        else if (strcmp(Buffer, "1") == 0)
        {
            std::string appsName = enumerateApps(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall");
            const char *apps = appsName.c_str();
            sendData(Client, apps, strlen(apps));
        }

        else if (strcmp(Buffer, "2") == 0)
        {
            const char *startProcessNameMessage = "Enter name of process: ";
            sendData(Client, startProcessNameMessage, strlen(startProcessNameMessage));
            receiveData(Client);
            std::string result = startProcess(Buffer);
            sendData(Client, result.c_str(), result.length());
        }
        else
        {
            sendInvalidChoiceMessage(Client);
        }
    }
}

void Server::processEnumerateProcessesOption(SOCKET &Client)
{
    const char *processMenu = "0.Exit\n1.List process\n2.Stop processing\nChoose your option: ";
    while (true)
    {
        sendData(Client, processMenu, strlen(processMenu));
        receiveData(Client);

        if (strcmp(Buffer, "0") == 0)
        {
            sendExitMessage(Client);
            break;
        }
        else if (strcmp(Buffer, "1") == 0)
        {
            std::string processesName = enumerateRunningProcesses();
            const char *processes = processesName.c_str();
            sendData(Client, processes, strlen(processes));
        }

        else if (strcmp(Buffer, "2") == 0)
        {
            const char *stopProcessNameMessage = "Enter name of process: ";
            sendData(Client, stopProcessNameMessage, strlen(stopProcessNameMessage));
            receiveData(Client);
            std::string result = stopProcess(Buffer);
            sendData(Client, result.c_str(), result.length());
        }
        else
            sendInvalidChoiceMessage(Client);
    }
}

void Server::processTakeScreenshotOption(SOCKET &Client)
{
    // take an image
    takeScreenshot("image");
    // Open the .bmp file
    std::ifstream file("image.bmp", std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Error opening BMP file: " << '\n';
        return;
    }
    // Get the file size
    file.seekg(0, std::ios::end);
    int fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(IMAGE_LENGTH);
    std::size_t bytesSent = 0;
    // send an image in packets of length IMAGE_LENGTH
    while (bytesSent < fileSize)
    {
        int bytesLeft = fileSize - bytesSent;
        int bytesToSend = min(bytesLeft, IMAGE_LENGTH);
        file.read(buffer.data(), bytesToSend);
        sendData(Client, buffer.data(), bytesToSend);
        bytesSent += bytesToSend;
    }
    file.close();
}

void Server::processCatchKeyPressOption(SOCKET &Client)
{
    const char *startCatchKeyPress = "Listening for key press events...\n";
    sendData(Client, startCatchKeyPress, strlen(startCatchKeyPress));
    catchKeyPress(Client);
}

void Server::traversingDirectoryTree(SOCKET &Client)
{
    fs::current_path(fs::path("/"));
    const char *helpMessage = "Commands:\n"
                              "  ls                               List the contents of current directory\n"
                              "  cd <path>                        Change the current directory\n"
                              "  mv <filename/source> <dest>      Move a file or directory to a new location\n"
                              "  cp <source> <dest>               Copy a file or directory to a new location\n"
                              "  rm <path>                        Remove a file or directory\n"
                              "  rn <old_name> <new_name>         Rename a file or directory\n"
                              "  ..                               Go up one directory level\n"
                              "  help                             Show this help message\n"
                              "  exit                             Quit the program\n\n";
    sendData(Client, helpMessage, strlen(helpMessage));

    while (true)
    {
        std::string currentPath = fs::current_path().string();
        std::string pathMessage = currentPath + '>';
        sendData(Client, pathMessage.c_str(), pathMessage.length());
        receiveData(Client);
        std::string command(Buffer);
        std::string sendMessage = "";

        if (command == "exit")
            break;

        else if (command == "help")
        {
            sendMessage = (std::string)helpMessage;
            sendData(Client, sendMessage.c_str(), sendMessage.length());
        }
        else if (command == "ls")
        {
            sendMessage = list();
            if (sendMessage == "")
            {
                sendMessage = "\n";
            }
            sendData(Client, sendMessage.c_str(), sendMessage.length());
        }

        else if (command.substr(0, 3) == "cd ")
        {
            std::string path = command.substr(3);
            sendMessage = changeDirectory(path);
            currentPath = fs::current_path().string();
        }
        else if (command.substr(0, 3) == "mv ")
        {
            std::string source = command.substr(3, command.find(' ', 3) - 3);
            std::string dest = command.substr(command.find(' ', 3) + 1) + "\\" + source;
            sendMessage = moveFile(source, dest);
            sendData(Client, sendMessage.c_str(), sendMessage.length());
        }
        else if (command.substr(0, 3) == "cp ")
        {
            std::string source = command.substr(3, command.find(' ', 3) - 3);
            std::string dest = command.substr(command.find(' ', 3) + 1) + "\\" + source;
            sendMessage = copyFile(source, dest);
            sendData(Client, sendMessage.c_str(), sendMessage.length());
        }
        else if (command.substr(0, 3) == "rm ")
        {
            std::string path = command.substr(3);
            sendMessage = deleteFile(path);
            sendData(Client, sendMessage.c_str(), sendMessage.length());
        }
        else if (command.substr(0, 3) == "rn ")
        {
            std::string old_name = command.substr(3, command.find(' ', 3) - 3);
            std::string new_name = command.substr(command.find(' ', 3) + 1);
            sendMessage = changeName(old_name, new_name);
            sendData(Client, sendMessage.c_str(), sendMessage.length());
        }
        else if (command == "..")
        {
            changeToParentDirectory();
            currentPath = fs::current_path().string();
        }
        else
        {
            sendMessage = "Error: invalid command\n";
            sendData(Client, sendMessage.c_str(), sendMessage.length());
        }
    }
}

void Server::processOptions(SOCKET &Client)
{
    const char *menuOptions = "0. Exit\n1. List app\n2. List process\n3. Take a screenshot\n4. Catch key press\n5. Directory\nChoose your option: ";
    const char *emptyChoice = "Empty choice, please try again!\n";
    while (1)
    {
        sendData(Client, menuOptions, strlen(menuOptions));
        receiveData(Client);

        // option 0: exit
        if (strcmp(Buffer, "0") == 0)
        {
            processExitOption(Client);
            break;
        }

        // option 1: list app
        else if (strcmp(Buffer, "1") == 0)
            processEnumerateAppsOption(Client);

        // option 2: list process
        else if (strcmp(Buffer, "2") == 0)
            processEnumerateProcessesOption(Client);
        // option 3: take a screenshot
        else if (strcmp(Buffer, "3") == 0)
        {
            processTakeScreenshotOption(Client);
        }
        // option 4: catch key press
        else if (strcmp(Buffer, "4") == 0)
            processCatchKeyPressOption(Client);
        // option 5: directory tree traversal
        else if (strcmp(Buffer, "5") == 0)
            traversingDirectoryTree(Client);
        else
            sendInvalidChoiceMessage(Client);
    }
}

int Server::catchKeyPress(SOCKET clientSocket)
{
    while (true)
    {
        receiveData(clientSocket);
        if (strcmp(Buffer, "stop") == 0)
            // Client wants to stop sending key presses
            break;
        else
        {
            // Convert the received message back to the key code
            int keyCode = atoi(Buffer);
            // cout << keyCode;
            //  Send the key code to the input queue of the server
            INPUT input = {0};
            input.type = INPUT_KEYBOARD;
            input.ki.wScan = MapVirtualKey(keyCode, MAPVK_VK_TO_VSC);
            input.ki.time = 0;
            input.ki.dwExtraInfo = 0;
            input.ki.wVk = keyCode;
            input.ki.dwFlags = 0;
            SendInput(1, &input, sizeof(INPUT));
        }
    }

    return 0;
}

std::string Server::enumerateApps(HKEY hKeyRoot, LPCSTR subKey)
{
    // Khi liệt kê có một số app bị trùng nên lưu vào set
    std::unordered_set<std::string> s;
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

    std::string appNames;
    for (const auto &app : s)
        appNames += app + '\n';
    return appNames;
}

std::string Server::startProcess(const char *name)
{
    std::string processName(name);
    int wstrLength = MultiByteToWideChar(CP_UTF8, 0, processName.c_str(), -1, nullptr, 0);
    std::wstring wideProcessName(wstrLength, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, processName.c_str(), -1, &wideProcessName[0], wstrLength);

    STARTUPINFO si = {};
    PROCESS_INFORMATION pi = {};
    if (CreateProcess(NULL, &wideProcessName[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        std::cout << "Process started with ID " << pi.dwProcessId << std::endl;
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return "Start process successfully\n";
    }
    return "Failed to start process: " + processName + '\n';
}

std::string wcharToString(WCHAR *t)
{
    char ch[260];
    char DefChar = ' ';
    WideCharToMultiByte(CP_ACP, 0, t, -1, ch, 260, &DefChar, NULL);
    std::string ss(ch);
    return ss;
}

std::string Server::enumerateRunningProcesses()
{
    std::unordered_set<std::string> s;
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        std::cerr << "Failed to create snapshot" << std::endl;
        return "Error when enumarate running process";
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32))
    {
        std::cerr << "Failed to get first process" << std::endl;
        CloseHandle(hProcessSnap);
        return "Error when enumarate running process";
    }

    do
    {
        std::string ss = wcharToString(pe32.szExeFile);
        s.insert(ss);

    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);

    std::string processNames;
    for (const auto &process : s)
        processNames += process + '\n';
    return processNames;
}

void Server::clearBuffer()
{
    memset(Buffer, 0, BUFFER_LENGTH);
}

std::string Server::stopProcess(const char *ProcessName)
{
    std::string process(ProcessName);
    std::string command = "taskkill /F /IM " + process;
    int result = system(command.c_str());

    if (result == 1)
        return "Failed to stop " + process + '\n';

    return "Stop " + process + "sucessfully\n";
}

void Server::takeScreenshot(std::string fileName)
{
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

    char *lpbitmap = new char[dwBmpSize];
    GetDIBits(hdcScreen, hBitmap, 0, height, lpbitmap, (BITMAPINFO *)&bi, DIB_RGB_COLORS);

    std::ofstream file(fileName, std::ios::out | std::ios::binary | std::ios::trunc);

    file.write((char *)&bmfHeader, sizeof(BITMAPFILEHEADER));
    file.write((char *)&bi, sizeof(BITMAPINFOHEADER));
    file.write(lpbitmap, dwBmpSize);

    file.close();

    delete[] lpbitmap;
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);
}

std::string Server::changeDirectory(const std::string &new_path)
{
    fs::path current_path = fs::current_path();

    // Kiểm tra xem đường dẫn mới có tồn tại hay không
    if (!fs::exists(new_path))
    {
        return "Error: directory '" + new_path + "' does not exist\n";
    }

    // Thiết lập đường dẫn hiện tại
    current_path /= new_path;
    fs::current_path(current_path);
    return " ";
}

std::string listDrives()
{
    std::string drives = "";
    char buffer[MAX_PATH];
    GetLogicalDriveStringsA(MAX_PATH, buffer);

    char *drive = buffer;
    while (*drive)
    {
        // Kiểm tra nếu drive là một ổ đĩa
        if (GetDriveTypeA(drive) == DRIVE_FIXED)
            drives = drives + (std::string)drive + "\n";
        drive += strlen(drive) + 1;
    }
    return drives;
}

std::string Server::list()
{
    fs::path current_path = fs::current_path();
    std::string filename = "";
    for (const auto &entry : fs::directory_iterator(current_path))
        filename += entry.path().filename().string() + '\n';

    return filename;
}

std::string Server::changeName(const std::string &path, const std::string &new_name)
{
    // Kiểm tra xem tệp tồn tại hay không
    if (!fs::exists(path))
    {
        std::cerr << "Error: file '" << path << "' does not exist\n";
        return "Error: file '" + path + "' does not exist\n";
    }

    // Lấy đường dẫn của tệp
    fs::path file_path = fs::path(path);

    // Lấy đường dẫn của thư mục chứa tệp
    fs::path directory_path = file_path.parent_path();

    // Thiết lập đường dẫn mới cho tệp
    fs::path new_path = directory_path / new_name;

    // Thay đổi tên tệp
    fs::rename(file_path, new_path);
    return "Renamed " + path + " to " + new_name + '\n';
}

std::string Server::moveFile(const std::string &source_path, const std::string &dest_path)
{
    // Kiểm tra xem tệp nguồn có tồn tại hay không
    if (!fs::exists(source_path))
        return "Error: source file '" + source_path + "' does not exist\n";

    // Kiểm tra xem tệp đích đã tồn tại chưa
    if (fs::exists(dest_path))
        return "Error: destination file '" + dest_path + "' already exists\n";

    // Di chuyển tệp từ đường dẫn nguồn tới đường dẫn đích
    try
    {
        fs::rename(source_path, dest_path);
    }
    catch (const fs::filesystem_error &e)
    {
        return "Error: " + (std::string)e.what() + '\n';
    }
    return "File moved successfully\n";
}

std::string Server::changeToParentDirectory()
{
    // Lấy đường dẫn hiện tại
    fs::path current_path = fs::current_path();

    // Lấy đường dẫn thư mục cha
    fs::path parent_path = current_path.parent_path();

    // Thay đổi đường dẫn hiện tại đến thư mục cha
    fs::current_path(parent_path);
    return current_path.string();
}

std::string Server::copyFile(const std::string &source_path, const std::string &dest_path)
{
    // Kiểm tra xem tệp nguồn có tồn tại hay không
    if (!fs::exists(source_path))
        return "Error: source file '" + source_path + "' does not exist\n";

    // Kiểm tra xem tệp đích đã tồn tại chưa
    if (fs::exists(dest_path))
        return "Error: destination file '" + dest_path + "' already exists\n";

    // Sao chép tệp từ đường dẫn nguồn tới đường dẫn đích
    try
    {
        fs::copy(source_path, dest_path);
    }
    catch (const fs::filesystem_error &e)
    {
        return "Error: " + (std::string)e.what() + '\n';
    }
    return "File copied successfully\n";
}

std::string Server::deleteFile(const std::string &path)
{
    // Kiểm tra xem tệp tồn tại hay không
    if (!fs::exists(path))
    {
        return "Error: file '" + path + "' does not exist\n";
    }

    // Xóa tệp
    fs::remove(path);
    return "Remove " + path + " sucessful\n";
}
