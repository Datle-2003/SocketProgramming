#include "Server.h"

const char* menu = "0. Exit\n1. List app\n2. List process\n3. Take a screenshot\n4. Catch key press\n5. Directory\n";
const char* emptyError = "Empty message, please try again!\n";
string message = "";
void process(SOCKET Client) {

}


int main()
{
	Server S;
	S.initWinsock();
	S.createSocket();
	S.bindSocket();
	S.listenSocket();
	SOCKET Client = S.acceptConnect();
	S.receiveData(Client);
	cout << Buffer;
	int cnt = 0;
	while (1)
	{
		cnt++;
		if (cnt == 10) break;

		S.sendData(Client, menu, strlen(menu));
		int byte = S.receiveData(Client);
		if (byte == 0)
			S.sendData(Client, emptyError, strlen(emptyError));
		if (strcmp(Buffer, "0") == 0) {
			message = "Abort connection!\n";
			S.sendData(Client, message.c_str(), message.size());
			break;
		}



		if (strcmp(Buffer, "1") == 0) {
			message = "0.Exit\n1.List App\n2.Start App\n";
			S.sendData(Client, message.c_str(), message.size());
			S.receiveData(Client);
			while (strcmp(Buffer, "1") != 0 && strcmp(Buffer, "2") != 0 && strcmp(Buffer, "0") != 0) {
				cnt++;
				if (cnt == 10) break;
				message = "Invalid choice, please try again\n";
				S.sendData(Client, message.c_str(), message.size());
				S.receiveData(Client);
			}

			if (strcmp(Buffer, "1") == 0) {
				string temp = S.EnumerateApps(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall") + '\n';
				const char* listApp = temp.c_str();
				S.sendData(Client, listApp, temp.size());// đang bị lỗi, phía client phải nhấn Enter vài lần mới hiện thị được hết app
			}

			else if (strcmp(Buffer, "2") == 0) {
				message = "Enter name of application\n";
				S.sendData(Client, message.c_str(), message.size());
				S.receiveData(Client);
				S.startApp(Buffer);
			}
			else if (strcmp(Buffer, "0") == 0) {
			}

		}


		else if (strcmp(Buffer, "2") == 0) {
			message = "0.Exit\n1.List process\n2.Stop processing\n";
			S.sendData(Client, message.c_str(), message.size());
			S.receiveData(Client);
			while (strcmp(Buffer, "1") != 0 && strcmp(Buffer, "2") != 0 && strcmp(Buffer, "0") != 0) {
				cnt++;
				if (cnt == 10) break;
				message = "Invalid choice, please try again\n";
				S.sendData(Client, message.c_str(), message.size());
				S.receiveData(Client);
			}

			if (strcmp(Buffer, "1") == 0) {
				string temp = S.ListRunningProcesses() + '\n';
				const char* listProcess = temp.c_str();
				S.sendData(Client, listProcess, temp.size());
			}

			else if (strcmp(Buffer, "2") == 0) {
				message = "Enter name of process\n";
				S.sendData(Client, message.c_str(), message.size());
				S.receiveData(Client);
				S.StopProcess(Buffer);
			}
			else if (strcmp(Buffer, "0") == 0) {
			}

		}

		else if (strcmp(Buffer, "3") == 0)
		{
			S.TakeScreenshot("image");
			// Open the .bmp file
			std::ifstream file("image.bmp", std::ios::binary);

			// Get the file size
			file.seekg(0, std::ios::end);
			std::size_t fileSize = file.tellg();
			file.seekg(0, std::ios::beg);

			std::vector<char> buffer(BUFFER_LENGTH);
			std::size_t bytesSent = 0;
			while (bytesSent < fileSize) {
				std::size_t bytesLeft = fileSize - bytesSent;
				std::size_t bytesToSend = min(bytesLeft, BUFFER_LENGTH);
				file.read(buffer.data(), bytesToSend);
				S.sendData(Client, buffer.data(), bytesToSend);
				bytesSent += bytesToSend;
			}
			file.close();
		}
		
		else if (strcmp(Buffer, "4") == 0)
		{
			cout << 4;
			S.catchKeyPress(Client);// lỗi không bắt được phím nhấn
		}
	
	}
	cout << Buffer;
	return 0;
}