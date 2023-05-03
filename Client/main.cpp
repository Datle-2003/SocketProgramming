#include "Client.h"

int main() {
    Client C;
    C.InitWinsock();
    C.CreateSocket();
    C.ConnectToServer();
    C.ControlServer();
    return 0;
}