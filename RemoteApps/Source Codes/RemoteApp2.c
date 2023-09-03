#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <process.h>

#pragma comment(lib, "ws2_32.lib")

#define BUFLEN 4096
#define VERSION "2.0"
#define CANARY1 0xcccccccc
#define CANARY2 0xaabbccdd
#define CANARY3 0xeeff0102
#define PORT 9998


void Function0(SOCKET clientSocket, char* userInput) {
	char* message = "";
	char dll[10] = { 'W','S','2','_','3','2','.','d','l','l' };
	int result = 0;

	for (int i = 0; i < 10; i++) {
		if (userInput[i + 16] != dll[i]) {
			message = "[-] Incorrect dll";
			send(clientSocket, message, strlen(message), 0);
			break;
		}
		result++;
	}

	if (result == 10) {
		HMODULE hModule;

		hModule = GetModuleHandleA("WS2_32.dll");

		if (hModule == NULL) {
			printf("[-] Cannot get handle of dll");
		}
		else {
			char message2[50];

			snprintf(message2, 50, "[+] Address of WS2_32.dll: %p", (void*)hModule);
			send(clientSocket, message2, strlen(message2), 0);
		}
	}
	else {
		message = "[-] Something went wrong";
		send(clientSocket, message, strlen(message), 0);
	}

}


void Function1(SOCKET clientSocket, char* userInput) {
	char function[4] = { 'm', 'a', 'i', 'n' };
	int result = 0;
	char* message;
	int address;

	for (int i = 0; i < 4; i++) {
		if (userInput[i + 16] != function[i]) {
			message = "[-] Incorrect function name";
			send(clientSocket, message, strlen(message), 0);
			break;
		}
		result++;
	}
	if (result == 4) {
		address = getAddress();
		char message2[50];

		snprintf(message2, 50, "[+] Address of main: %p", (void*)address);
		send(clientSocket, message2, strlen(message2), 0);
	}

}

void Function2(SOCKET clientSocket, char* userInput) {
	char* message;
	char dll[12] = { 'K', 'E', 'R', 'N', 'E', 'L', '3', '2', '.', 'd', 'l', 'l' };
	int result = 0;

	for (int i = 0; i < 12; i++) {
		if (dll[i] != userInput[i + 16]) {
			message = "[-] Incorrect dll";
			send(clientSocket, message, strlen(message), 0);
			break;
		}
		result++;
	}
	if (result == 12) {
		HMODULE hModule = GetModuleHandleA("KERNEL32.dll");

		if (hModule == NULL) {
			message = "[-] Cannot handle of dll";
			send(clientSocket, message, strlen(message), 0);
		}
		char message2[50];

		snprintf(message2, 50, "[+] Address of KERNEL32.dll: %p", (void*)hModule);
		send(clientSocket, message2, strlen(message2), 0);
	}
}


void Function3(SOCKET clientSocket, char* userInput) {
	char buff[2000];
	ZeroMemory(buff, 2000);

	memcpy(buff, userInput, BUFLEN);
}


void Function4(SOCKET clientSocket, char* userInput) {
	char buff[2000];
	ZeroMemory(buff, 2000);

	memcpy(buff, userInput, 2000);
}

void handleConnection(SOCKET clientSocket) {
	char userInput[BUFLEN];
	char response[BUFLEN];
	char* message = "Type something: ";
	int recvLen = 0;
	unsigned int canary1 = 0;
	unsigned int canary2 = 0;
	unsigned int canary3 = 0;
	unsigned int opcode = 0;

	ZeroMemory(userInput, BUFLEN);
	ZeroMemory(response, BUFLEN);

	send(clientSocket, message, strlen(message), 0);

	recvLen = recv(clientSocket, response, BUFLEN, 0);

	memcpy(userInput, response, BUFLEN);
	memcpy(&canary1, (char*)&userInput, sizeof(unsigned int));

	printf("User canary: %x\n", canary1);
	if (canary1 != CANARY1) {
		message = "[-] Something went wrong :(";
		send(clientSocket, message, strlen(message), 0);
		exit(1);
	}

	memcpy(&canary2, userInput + 4, sizeof(unsigned int));
	printf("User Canary2: %x\n", canary2);
	if (canary2 != CANARY2) {
		message = "[-] Something went wrong :(";
		send(clientSocket, message, strlen(message), 0);
		exit(1);
	}

	memcpy(&canary3, userInput + 8, sizeof(unsigned int));
	printf("User Canary3: %x\n", canary3);
	if (canary3 != CANARY3) {
		message = "[-] Something went wrong :(";
		send(clientSocket, message, strlen(message), 0);
		exit(1);

	}

	message = "[+] All canary checks passed";
	send(clientSocket, message, strlen(message), 0);

	memcpy(&opcode, userInput + 12, sizeof(unsigned int));

	switch (opcode) {
	case 899:
		printf("[*] Calling Function0\n");
		Function0(clientSocket, (char*)userInput);
		message = "[+] Function0 successfully completed";
		send(clientSocket, message, strlen(message), 0);
		printf("[+] Function0 successfully completed\n");
		break;

	case 900:
		printf("[*] Calling Function1\n");
		Function1(clientSocket, (char*)userInput);
		message = "[+] Function1 successfully completed";
		send(clientSocket, message, strlen(message), 0);
		printf("[+] Function1 successfully completed\n");
		break;

	case 901:
		printf("[*] Calling Function2\n");
		Function2(clientSocket, (char*)userInput);
		message = "[+] Function2 successfully completed";
		send(clientSocket, message, strlen(message), 0);
		printf("[+] Function2 successfully completed\n");
		break;

	case 902:
		printf("[*] Calling Function3\n");
		Function3(clientSocket, (char*)userInput);
		message = "[+] Function3 successfully completed";
		send(clientSocket, message, strlen(message), 0);
		printf("[+] Function3 successfully completed\n");
		break;

	case 903:
		printf("[*] Calling Function4\n");
		Function4(clientSocket, (char*)userInput);
		message = "[+] Function4 successfully completed";
		send(clientSocket, message, strlen(message), 0);
		printf("[+] Function4 successfully completed\n");
		break;

	default:
		message = "[-] Incorrect opcode";
		send(clientSocket, message, strlen(message), 0);
		break;
	}

}


int main() {

	WSADATA wsa;
	SOCKET s, newSocket;
	struct sockaddr_in server, client;
	int c = sizeof(struct sockaddr_in);

	printf("[*] Initiaizing WS2_32.dll\n");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("[-] Initializing WS2_32.dll failed\n");
		exit(1);
	}
	printf("[+] WS2_32.dll successfully initialized\n");

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == SOCKET_ERROR) {
		printf("[-] Socket failed\n");
		WSACleanup();
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
	printf("[+] Listening incoming connection\n");

	while ((newSocket = accept(s, (struct sockaddr*)&client, &c)) != SOCKET_ERROR) {
		printf("[+] Client connected\n");
		_beginthread(&handleConnection, 0, newSocket);
	}
	if (newSocket == SOCKET_ERROR) {
		printf("[-] Accept failed\n");
		closesocket(s);
		closesocket(newSocket);
		WSACleanup();
		exit(1);
	}

	closesocket(s);
	closesocket(newSocket);
	WSACleanup();

	return 0;
}


int getAddress() {
	void* address = &main;

	return (int)address;
}