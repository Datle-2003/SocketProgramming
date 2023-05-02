#include "Server.h"

int main()
{
	Server S;
	S.initWinsock();
	S.createSocket();
	S.bindSocket();
	S.listenSocket();
	SOCKET Client = S.acceptConnect();
	return 0;
}