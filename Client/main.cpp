#include "Client.h"

int main() {
    Client C;
    C.InitWinsock();
    C.CreateSocket();
    C.ConnectToServer();
    C.Communicate();
    return 0;
}