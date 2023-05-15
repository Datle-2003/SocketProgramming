#include "Client.h"

int main() {

    Client C;
   // int port = 0;
    std::string ipAddress = "";
    std::cout << "Enter ip address of server(if server is the same device as client, press S): ";
    std::cin >> ipAddress;
   // std::cout << "Enter port: ";
   // cin >> port;
    if (ipAddress != "S") {
        C.setServerAddress(8080, ipAddress.c_str());
    }
    std::cin.ignore();

    C.connectToServer();
    

    while (true) {
        C.receiveData(); // menu
        C.sendData();
        if (strcmp(Buffer, "0") == 0) {
            C.receiveData();
            break;
        }
        else if (strcmp(Buffer, "1") == 0) {
            
            while (true) {
                C.receiveData(); // menu
                C.sendData();
                if (strcmp(Buffer, "0") == 0) {
                    C.receiveData();
                    break;
                }
                else if (strcmp(Buffer, "1") == 0) {
                    C.receiveData();// data
                }
                else if (strcmp(Buffer, "2") == 0) {
                    C.receiveData(); // nameMessage
                    C.sendData();
                    C.receiveData();// empty or not
                }
                else {
                    // invalid
                    C.receiveData();
                }
            }
        }

        else if (strcmp(Buffer, "2") == 0) {
            while (true) {
                C.receiveData(); // menu
                C.sendData();
                if (strcmp(Buffer, "0") == 0) {
                    C.receiveData();
                    break;
                }
                else if (strcmp(Buffer, "1") == 0) {
                    C.receiveData();// data
                }
                else if (strcmp(Buffer, "2") == 0) {
                    C.receiveData(); // nameMessage
                    C.sendData();
                    C.receiveData();// empty or not
                }
                else {
                    // invalid
                    C.receiveData();
                }
            }
        }

        else if (strcmp(Buffer, "3") == 0) {
            C.receiveImage();
            std::cout << "Image received\n";
        }

        else if (strcmp(Buffer, "4") == 0) {
            C.receiveData();
            C.catchKeyInput();
        }

        else if (strcmp(Buffer, "5") == 0) {
            C.receiveData(); // menu
            while (strcmp(Buffer, "exit") != 0) {
                if ((strstr(Buffer, "cd") != NULL) || (strstr(Buffer, "..") != NULL)) {
                    C.receiveData();
                    C.sendData();
                }
                else {
                    C.receiveData();
                    C.receiveData();
                    C.sendData();
                }
            }
        }
        else {
            // invalid or message
            C.receiveData();
        }
       
    }
    return 0;
}