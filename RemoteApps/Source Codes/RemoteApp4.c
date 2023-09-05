#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <process.h>

#pragma comment(lib, "ws2_32.lib")

#define BUFLEN 4096
#define PORT 9996
#define CANARY1 0x12345678
#define CANARY2 0xdeadb33f
#define CANARY3 0x20202020



void Function1(SOCKET clientSocket, char* userInput) {

	HMODULE hModule;
	char module[11] = "";
	module[10] = '\00';
	char* message = "";

	for (int i = 0; i < 10; i++) {
		module[i] = (char)userInput[i + 56];
	}

	hModule = GetModuleHandleA((char*)module);
	if (hModule == NULL) {
		message = "[-] Failed getting handle of the module | ";
		send(clientSocket, message, strlen(message), 0);
	}
	else {
		char message2[100];

		snprintf(message2, 50, "[+] Address of %s: %p | ", module, (void*)hModule);
		send(clientSocket, message2, strlen(message2), 0);
	}
}


void Function2(SOCKET clientSocket, char* userInput) {

	HMODULE hModule;
	char module[13] = "";
	module[12] = '\00';
	char* message = "";

	for (int i = 0; i < 12; i++) {
		module[i] = (char)userInput[i + 56];
	}

	hModule = GetModuleHandleA(module);
	if (hModule == NULL) {
		message = "[-] Failed getting handle of the module | ";
		send(clientSocket, message, strlen(message), 0);
	}
	else {
		char message2[100];

		snprintf(message2, 100, "[+] Address of %s: %p | ", module, (void*)hModule);
		send(clientSocket, message2, strlen(message2), 0);
	}
}


void Function3(SOCKET clientSocket, char* userInput) {

	char function[4] = { 'm', 'a', 'i', 'n' };
	char* message;
	int result = 0;

	for (int i = 0; i < 4; i++) {
		if (function[i] != (char*)userInput[i + 56]) {
			message = "[-] Incorrect function name | ";
			send(clientSocket, message, strlen(message), 0);
			break;
		}
		result++;
	}

	if (result == 4) {
		int address = LeakMain();

		char message2[100];

		snprintf(message2, 100, "[+] Address of main: %p | ", (void*)address);
		send(clientSocket, message2, strlen(message2), 0);
	}
}


void Function4(SOCKET clientSocket, char* userInput) {

	char module[10] = "";
	module[9] = '\00';
	HMODULE hModule;
	char* message = "";

	for (int i = 0; i < 9; i++) {
		module[i] = (char)userInput[i + 56];
	}

	hModule = GetModuleHandleA((char*)module);
	if (hModule == NULL) {
		message = "[-] Error while getting handle of the module | ";
		send(clientSocket, message, strlen(message), 0);
	}
	else {
		char message2[100];

		snprintf(message2, 100, "[+] Address of %s: %p | ", module, (void*)hModule);
		send(clientSocket, message2, strlen(message2), 0);
	}
}


void Function5(char* userInput)
{
	char buffer[2000];
	ZeroMemory(buffer, 2000);

	memcpy(buffer, userInput, BUFLEN);

}


void Function6(SOCKET clientSocket, char* userInput) {

	char buffer[4096];
	ZeroMemory(buffer, BUFLEN);

	memcpy(userInput, buffer, BUFLEN);

}


void Function7(SOCKET clientSocket, char* userInput) {

	char buf[100];
	ZeroMemory(buf, 100);

	snprintf(buf, 100, userInput + 56);

	send(clientSocket, buf, 100, 0);


}


void Function8(SOCKET clientSocket, char* userInput) {

	char* message = "[*] Not here :)";
	send(clientSocket, message, strlen(message), 0);

}



void handleConnection(SOCKET clientSocket) {

	char userInput[BUFLEN];
	char* message = "";
	unsigned int opcode;
	unsigned int userCanary1, userCanary2, userCanary3;
	unsigned int check1, check2, check3, check4, check5, check6, check7, check8, check9, check10;
	int recvLen;
	char badChars[] = { 0x00, 0x01, 0x0a, 0x0b, 0x0c, 0x0f, 0xa1, 0xb1, 0xc1, 0x0e, 0xaa, 0xbb, 0xcc, 0xd1 };

	ZeroMemory(userInput, BUFLEN);

	message = "Type something ";
	if (send(clientSocket, message, strlen(message), 0) == SOCKET_ERROR) {
		printf("[-] Send failed\n");
		closesocket(clientSocket);
		exit(1);
	}
	printf("[+] Send done\n");

	if ((recvLen = recv(clientSocket, userInput, BUFLEN, 0)) == 0) {
		printf("[-] Recv failed\n");
		closesocket(clientSocket);
		exit(1);
	}
	printf("[+] Received %d bytes from client\n", recvLen);

	for (int i = 0; i < recvLen; i++) {
		for (int j = 0; j < sizeof(badChars); j++) {
			if ((int)userInput[i] == badChars[j]) {
				ZeroMemory(userInput, BUFLEN);
				break;
			}
		}
	}

	memcpy(&userCanary1, (char*)&userInput + 0, sizeof(unsigned int));
	memcpy(&userCanary2, (char*)&userInput + 4, sizeof(unsigned int));
	memcpy(&userCanary3, (char*)&userInput + 8, sizeof(unsigned int));

	if (userCanary1 == CANARY1 && userCanary2 == CANARY2 && userCanary3 == CANARY3) {
		message = "[+] Stack Canaries bypassed | ";
		send(clientSocket, message, strlen(message), 0);
	}
	else {
		message = "[-] Stack Canaries still alive | ";
		send(clientSocket, message, strlen(message), 0);
		closesocket(clientSocket);
		exit(1);
	}


	memcpy(&check1, (char*)&userInput + 12, sizeof(unsigned int));
	memcpy(&check2, (char*)&userInput + 16, sizeof(unsigned int));

	if (check1 == 0x5A3Fd0FA && check2 == 0xB8D2C0B3) {
		message = "[+] First check step bypassed | ";
		send(clientSocket, message, strlen(message), 0);
	}
	else {
		message = "[-] First check step failed | ";
		send(clientSocket, message, strlen(message), 0);
		closesocket(clientSocket);
		exit(1);
	}


	memcpy(&check3, (char*)&userInput + 20, sizeof(unsigned int));
	memcpy(&check4, (char*)&userInput + 24, sizeof(unsigned int));

	if (check3 == 0xE7A9BEEF && check4 == 0x12F8D0C0) {
		message = "[+] Second check step bypassed | ";
		send(clientSocket, message, strlen(message), 0);
	}
	else {
		message = "[-] Second check step failed | ";
		send(clientSocket, message, strlen(message), 0);
		closesocket(clientSocket);
		exit(1);
	}


	memcpy(&check5, (char*)&userInput + 28, sizeof(unsigned int));
	memcpy(&check6, (char*)&userInput + 32, sizeof(unsigned int));

	if (check5 == 0x9C71CAFE && check6 == 0x3BEAC0D3) {
		message = "[+] Third check step bypassed | ";
		send(clientSocket, message, strlen(message), 0);
	}
	else {
		message = "[-] Thrid check step failed | ";
		send(clientSocket, message, strlen(message), 0);
		closesocket(clientSocket);
		exit(1);
	}


	memcpy(&check7, (char*)&userInput + 36, sizeof(unsigned int));
	memcpy(&check8, (char*)&userInput + 40, sizeof(unsigned int));

	if (check7 == 0x6F50DEAD && check8 == 0xDAB2FFFF) {
		message = "[+] Fourth check step bypassed | ";
		send(clientSocket, message, strlen(message), 0);
	}
	else {
		message = "[-] Fourth check step failed | ";
		send(clientSocket, message, strlen(message), 0);
		closesocket(clientSocket);
		exit(1);
	}


	memcpy(&check9, (char*)&userInput + 44, sizeof(unsigned int));
	memcpy(&check10, (char*)&userInput + 48, sizeof(unsigned int));

	if (check9 == 0x87C4A2B2 && check10 == 0xF503C2DD) {
		message = "[+] Fifth check step bypassed | ";
		send(clientSocket, message, strlen(message), 0);
	}
	else {
		message = "[-] Fifth check step failed | ";
		send(clientSocket, message, strlen(message), 0);
		closesocket(clientSocket);
		exit(1);
	}


	memcpy(&opcode, (char*)&userInput + 52, sizeof(unsigned int));

	switch (opcode) {
	case 0xfffff384:
		message = "[*] Calling Function1 | ";
		send(clientSocket, message, strlen(message), 0);
		Function1(clientSocket, (char*)userInput);
		message = "[+] Function1 completed | ";
		send(clientSocket, message, strlen(message), 0);
		break;

	case 0xfffff385:
		message = "[*] Calling Function2 | ";
		send(clientSocket, message, strlen(message), 0);
		Function2(clientSocket, (char*)userInput);
		message = "[+] Function2 completed | ";
		send(clientSocket, message, strlen(message), 0);
		break;

	case 0xfffff386:
		message = "[*] Calling Function3 | ";
		send(clientSocket, message, strlen(message), 0);
		Function3(clientSocket, (char*)userInput);
		message = "[+] Function3 completed | ";
		send(clientSocket, message, strlen(message), 0);
		break;

	case 0xfffff387:
		message = "[*] Calling Function4 | ";
		send(clientSocket, message, strlen(message), 0);
		Function4(clientSocket, (char*)userInput);
		message = "[+] Function4 completed | ";
		send(clientSocket, message, strlen(message), 0);
		break;

	case 0xfffff388:
		message = "[*] Calling Function5 | ";
		send(clientSocket, message, strlen(message), 0);
		Function5((char*)userInput);
		message = "[+] Function5 completed | ";
		send(clientSocket, message, strlen(message), 0);
		break;

	case 0xfffff389:
		message = "[*] Calling Function6 | ";
		send(clientSocket, message, strlen(message), 0);
		Function6(clientSocket, (char*)userInput);
		message = "[+] Function6 completed | ";
		send(clientSocket, message, strlen(message), 0);
		break;

	case 0xfffff390:
		message = "[*] Calling Function7 | ";
		send(clientSocket, message, strlen(message), 0);
		Function7(clientSocket, (char*)userInput);
		message = "[+] Function7 completed | ";
		send(clientSocket, message, strlen(message), 0);
		break;

	case 0xfffff391:
		message = "[*] Calling Function8 | ";
		send(clientSocket, message, strlen(message), 0);
		Function8(clientSocket, (char*)userInput);
		message = "[+] Function8 completed | ";
		send(clientSocket, message, strlen(message), 0);
		break;

	default:
		message = "[-] Wrong opcode | ";
		send(clientSocket, message, strlen(message), 0);
		break;
	}

}


int main() {

	WSADATA wsa;
	SOCKET s, newSocket;
	struct sockaddr_in server, client;
	int c = sizeof(struct sockaddr_in);

	printf("[*] Initializing WS2_32.dll\n");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("[-] Failed initializing WS2_32.dll\n");
		exit(1);
	}
	printf("[+] WS2_32.dll initialized\n");

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == SOCKET_ERROR) {
		printf("[-] Socket failed\n");
		exit(1);
	}
	printf("[+] Socket created\n");

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);

	if (bind(s, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
		printf("[-] Bind failed\n");
		closesocket(s);
		WSACleanup();
		exit(1);
	}
	printf("[+] Bind created\n");

	if (listen(s, SOMAXCONN) == SOCKET_ERROR) {
		printf("[-] Listen failed\n");
		closesocket(s);
		WSACleanup();
		exit(1);
	}
	printf("[+] Waiting for incoming connection\n");

	while ((newSocket = accept(s, (struct sockaddr*)&client, &c)) != SOCKET_ERROR) {
		printf("[+] Client connected\n");
		_beginthread(&handleConnection, 0, newSocket);
	}
	if (newSocket == SOCKET_ERROR) {
		printf("[-] Accept failed\n");
		closesocket(newSocket);
		closesocket(s);
		WSACleanup();
		exit(1);
	}

	closesocket(newSocket);
	closesocket(s);
	WSACleanup();

	return 0;
}


int LeakMain() {
	void* address = &main;

	return (int)address;
}