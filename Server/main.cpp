#include "Server.h"



int main()
{
	Server S;
    S.setup();
	SOCKET Client = S.acceptConnect();
	S.processOptions(Client);
	return 0;
}