#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <stdlib.h>
#include <time.h>


#pragma comment(lib, "ws2_32.lib") //Winsock Library

#define DEFAULT_BUFLEN 16384
#define DEFAULT_PORT 9999
#define CANARY 0x11223344


void Function0(SOCKET clientSock, char* userInput) {
	HMODULE hModule;
	int result = 0;
	char dll[12] = { 'W','S','2','_', '3', '2', '.', 'd', 'l', 'l' };

	for (int i = 0; i < 10; i++) {
		if (dll[i] != userInput[i + 10]) {
			char* message = "[-] Not correct dll";
			send(clientSock, message, strlen(message), 0);
			break;
		}
		else
			result++;
	}
	if (result == 10) {
		hModule = GetModuleHandleA("WS2_32.dll");
		if (hModule == NULL) {
			char* message = "[-] Failed to get the handle of the dll | ";
			send(clientSock, message, strlen(message), 0);
		}
		char message[50];
		snprintf(message, sizeof(message), "[+] Address of WS2_32.dll: %p", (void*)hModule);

		send(clientSock, message, strlen(message), 0);
	}
}


void Function1(SOCKET clientSock, char* userInput) {
	HMODULE hModule;
	int result = 0;
	char dll[12] = { 'K','E','R','N','E','L','3','2','.', 'd', 'l', 'l' };
	
	for (int i = 0; i < 12; i++) {
		if (dll[i] != userInput[i + 10]) {
			char* message = "[-] Not correct dll";
			send(clientSock, message, strlen(message), 0);
			break;
		}
		else
			result++;
	}
	if(result == 12) {
		hModule = GetModuleHandleA("KERNEL32.dll");
		if (hModule == NULL) {
			char* message = "[-] Failed to get the handle of the dll | ";
			send(clientSock, message, strlen(message), 0);
		}
		char message[50];
		snprintf(message, sizeof(message), "[+] Address of KERNEL32.dll: %p", (void*)hModule);
		
		send(clientSock, message, strlen(message), 0);
	}
}

void Function2(SOCKET clientSock, char* userInput) {
	char func[4] = { 'm','a','i','n' };
	int result = 0;
	char message[50];

	for (int i = 0; i < 4; i++) {
		if (func[i] == userInput[i + 10]) {
			result++;
		}
	}
	if (result == 4) {
		void *address = getFunctionAddress();
		snprintf(message, sizeof(message), "[+] Address of main: %p", address);

		send(clientSock, message, strlen(message), 0);
	}
	else {
		char* message2 = "[-] Function not correct";
		send(clientSock, message2, strlen(message), 0);
	}
}

void Function3(char* userInput) {
	char buf[2000];
	ZeroMemory(buf, sizeof(buf));

	memcpy(buf, userInput, DEFAULT_BUFLEN);
}



void handleConnection(SOCKET clientSock) {

	char* VERSION = "REMOTE APP VERSION: 1.0\n";
	char* message = "Type something\n";
	char userInput[DEFAULT_BUFLEN];
	char response[DEFAULT_BUFLEN];
	ZeroMemory(userInput, DEFAULT_BUFLEN);
	int recvLen;

	if (send(clientSock, VERSION, strlen(VERSION), 0) == SOCKET_ERROR) {
		printf("[-] send failed with error code: %d\n", GetLastError());
		closesocket(clientSock);
		exit(1);
	}
	if (send(clientSock, message, strlen(message), 0) == SOCKET_ERROR) {
		printf("[-] send failed with error code: %d\n", GetLastError());
		closesocket(clientSock);
		exit(1);
	}
	else {
		printf("[+] send completed\n");
	}

	if ((recvLen = recv(clientSock, (char*)userInput, sizeof(userInput), 0)) == SOCKET_ERROR) {
		closesocket(clientSock);
		exit(1);
	}
	else {
		printf("[+] recv completed\n");
	}

	unsigned int userCanary;
	memcpy(&userCanary, &userInput, sizeof(unsigned int));
	printf("User Canary: %x\n", userCanary);

	if (CANARY != (userCanary ^ recvLen)) {
		char* message = "Stack cookies still alive :(\n";
		send(clientSock, message, strlen(message), 0);
		closesocket(clientSock);
		WSACleanup();
		exit(1);
	}
	else {
		char* message = "Stack cookies disabled :)\n";
		send(clientSock, message, strlen(message), 0);
	}


	if (userInput[4] != 'A') {
		char* message = "Second check doesn't passed :(\n";
		send(clientSock, message, strlen(message), 0);
		closesocket(clientSock);
		WSACleanup();
		exit(1);
	}
	else if (userInput[4] == 'A') {
		char* message = "Second check passed :)\n";
		send(clientSock, message, strlen(message), 0);
	}


	if (userInput[5] != 'B') {
		char* message = "Third check doesn't passed :(";
		send(clientSock, message, strlen(message), 0);
		closesocket(clientSock);
		WSACleanup();
		exit(1);
	}
	else if (userInput[5] == 'B') {
		char* message = "Third check passed :)";
		send(clientSock, message, strlen(message), 0);
	}

	unsigned int opcode = 0;
	memcpy(&opcode, (char*)userInput + 6, sizeof(unsigned int));
	printf("User opcode: %d\n", opcode);

	switch (opcode) {
	case 899:
		printf("Calling Function0\n");
		Function0(clientSock, (char*)userInput);
		send(clientSock, "[+] 899 option compeleted", 25, 0);
		break;
	case 900:
		printf("Calling Function1\n");
		Function1(clientSock, (char*)userInput);
		send(clientSock, "[+] 900 option compeleted", 25, 0);
		break;
	case 901:
		printf("Calling Function2\n");
		Function2(clientSock, (char*)userInput);
		send(clientSock, "[+] 901 option compeleted", 25, 0);
		break;
	case 902:
		printf("Calling Function3\n");
		Function3((char*)userInput);
		break;
	default:
		printf("[-] Wrong opcode\n");
		break;
	}
	

	closesocket(clientSock);
	printf("%d thread ended connecion!\n", GetCurrentThread());

}



int main(int argc, char* argv[])
{
	WSADATA wsa;
	SOCKET s, newSocket;
	struct sockaddr_in server, client;
	int c;

	printf("\n[*] Initialising Winsock...\n");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("[-] Failed. Error Code : %d\n", WSAGetLastError());
		return 1;
	}
	else {
		printf("[+] Initialised\n");
	}


	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		printf("[-] Could not create socket : %d\n", WSAGetLastError());
		return 1;
	}
	else {
		printf("[+] Socket created\n");
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(DEFAULT_PORT);

	if (bind(s, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
		printf("[-] Bind failed with error code: %d\n", WSAGetLastError());
		closesocket(s);
		WSACleanup();
		exit(1);
	}
	else {
		printf("[+] Bind done\n");
	}

	if (listen(s, SOMAXCONN) == SOCKET_ERROR) {
		printf("[-] listen failed with error code: %d\n", SOCKET_ERROR);
		closesocket(s);
		WSACleanup;
		exit(1);
	}
	else {
		printf("[+] Wainting for incoming connection....\n");
	}

	c = sizeof(struct sockaddr_in);


	while ((newSocket = accept(s, (struct sockaddr*)&client, &c)) != INVALID_SOCKET) {
		printf("[+] Client connected\n");
		_beginthread(&handleConnection, 0, newSocket);
	}

	if (newSocket == SOCKET_ERROR) {
		printf("[-] accept failed with error code: %d\n", WSAGetLastError());
		closesocket(s);
		closesocket(newSocket);
		WSACleanup();
	}

	closesocket(s);
	closesocket(newSocket);
	WSACleanup();


	return 0;
}

int getFunctionAddress() {
	void (*address)() = &main;

	return (int)address;
}