#include <stdio.h>
#include <winsock2.h>
#include <Windows.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <process.h>


#pragma comment(lib, "ws2_32.lib")


#define BUFFLEN 4096
#define VERSION "3.0"
#define PORT 9997
#define CANARY1 0xdeadbeef
#define CANARY2 0xc0decafe
#define CANARY3 0xd0d0face


void Function1(SOCKET clientSocket, char* userInput) {
	char func[4] = { 'm', 'a', 'i', 'n' };
	int result = 0;

	for (int i = 0; i < 4; i++) {
		if (func[i] == userInput[i + 32]) {
			result++;
		}
		else {
			result = 0;
			char* message = "[-] Not correct function | ";

			send(clientSocket, message, strlen(message), 0);
			break;
		}
	}

	if (result == 4) {
		void* address = getFunctionAddress();
		char message[50];

		snprintf(message, sizeof(message), "[+] Address of main: %p", address);

		send(clientSocket, message, strlen(message), 0);
	}
}


void Function2(SOCKET clientSocket, char* userInput) {
	HMODULE hModule;
	char dll[12] = { 'K', 'E', 'R', 'N', 'E', 'L', '3', '2', '.', 'd', 'l', 'l' };
	char* message;
	int result = 0;

	for (int i = 0; i < 12; i++) {
		if (dll[i] != userInput[i + 32]) {
			message = "[-] Incorrect dll | ";
			send(clientSocket, message, strlen(message), 0);
			break;
		}
		result++;
	}
	if (result == 12) {
		hModule = GetModuleHandleA("KERNEL32.dll");

		if (hModule == NULL) {
			message = "[-] Error while getting handle of dll | ";
			send(clientSocket, message, strlen(message), 0);
		}
		else {
			char message2[50];

			snprintf(message2, sizeof(message2), "[+] Address of KERNEL32.dll: %p", (void*)hModule);
			send(clientSocket, message2, strlen(message2), 0);
		}
	}
	else {
		message = "[-] Something went wrong | ";
		send(clientSocket, message, strlen(message), 0);
	}

}


void Function3(SOCKET clientSocket, char* userInput) {
	HMODULE hModule;
	char dll[10] = { 'W', 'S', '2', '_', '3', '2', '.', 'd', 'l', 'l' };
	char* message;
	int result = 0;

	for (int i = 0; i < 10; i++) {
		if (dll[i] != userInput[i + 32]) {
			message = "[-] Incorrect dll | ";
			send(clientSocket, message, strlen(message), 0);
			break;
		}
		result++;
	}
	if (result == 10) {
		hModule = GetModuleHandleA("WS2_32.dll");

		if (hModule == NULL) {
			message = "[-] Failed to get handle of dll | ";
			send(clientSocket, message, strlen(message), 0);
		}
		else {
			char message2[50];

			snprintf(message2, sizeof(message2), "[+] Address of WS2_32.dll: %p", (void*)hModule);
			send(clientSocket, message2, strlen(message2), 0);
		}
	}
	else {
		message = "[-] Something went wrong";
		send(clientSocket, message, sizeof(message), 0);
	}


}


void Function4(char* userInput) {

	char buf[2500];
	ZeroMemory(buf, sizeof(buf));

	memcpy(buf, userInput, BUFFLEN);

}


void Function5(SOCKET clientSocket, char* userInput) {

	send(clientSocket, userInput, sizeof(userInput), 0);

}


void handleConnection(SOCKET clientSocket) {
	
	char* message = "Welcome to RemoteApp3 | ";
	char userInput[BUFFLEN];
	char response[BUFFLEN];
	ZeroMemory(userInput, BUFFLEN);
	ZeroMemory(response, BUFFLEN);
	int recvLen;
	char badchars[] = {0x0a, 0x0b, 0x0c, 0x0d, 0xa1, 0xa2, 0xa3, 0xa4, 0xb1, 0xb2, 0xb3, 0xb4, 0xf1, 0xf2, 0xf3, 0xf4};


	if (send(clientSocket, message, strlen(message), 0) == SOCKET_ERROR) {
		printf("[-] Send failed: %d\n", GetLastError());
		closesocket(clientSocket);
		exit(1);
	}
	printf("[+] Send completed\n");

	if ((recvLen = recv(clientSocket, userInput, sizeof(userInput), 0)) == SOCKET_ERROR) {
		printf("[-] Recv failed: %d\n", GetLastError());
		closesocket(clientSocket);
		exit(1);
	}
	printf("[+] Received %d bytes from client\n", recvLen);

	for (int i = 0; i < recvLen; i++) {
		for (int j = 0; j < sizeof(badchars); j++) {
			if ((int)userInput[i] == badchars[j]) {
				ZeroMemory(userInput, recvLen);
				break;
			}
		}
	}

	unsigned int userCanary1, userCanary2, userCanary3;

	memcpy(&userCanary1, (char*)&userInput, sizeof(unsigned int));
	printf("User canary1: %x\n", userCanary1);

	memcpy(&userCanary2, (char*)&userInput + 4, sizeof(unsigned int));
	printf("User canary2: %x\n", userCanary2);

	memcpy(&userCanary3, (char*)&userInput + 8, sizeof(unsigned int));
	printf("User canary3: %x\n", userCanary3);

	if (userCanary1 == CANARY1 && userCanary2 == CANARY2 && userCanary3 == CANARY3) {
		message = "[+] Stack canary disabled | ";
		send(clientSocket, message, strlen(message), 0);
	}
	else {
		message = "[-] Stack canary still alive | ";
		send(clientSocket, message, strlen(message), 0);
		closesocket(clientSocket);
	}

	unsigned int check1, check2;

	memcpy(&check1, (char*)&userInput + 12, sizeof(unsigned int));
	memcpy(&check2, (char*)&userInput + 16, sizeof(unsigned int));
	printf("check1: %x\n", check1);
	printf("check2: %x\n", check2);
	

	if (check1 != 0x41414141 && check2 != 0x42424242) {
		message = "[-] Authentication failed | ";
		send(clientSocket, message, strlen(message), 0);
		closesocket(clientSocket);
	}
	else {
		message = "[+] Authentication bypassed | ";
		send(clientSocket, message, strlen(message), 0);
	}

	unsigned int check3, check4;

	memcpy(&check3, (char*)&userInput + 20, sizeof(unsigned int));
	memcpy(&check4, (char*)&userInput + 24, sizeof(unsigned int));
	printf("check1: %x\n", check3);
	printf("check2: %x\n", check4);

	if (check3 == 0x43434343 && check4 == 0x44444444) {
		message = "Third check passed | ";
		send(clientSocket, message, strlen(message), 0);
	}
	else {
		message = "Third didn't pass | ";
		send(clientSocket, message, strlen(message), 0);
	}

	unsigned int opcode;
	memcpy(&opcode, (char*)&userInput + 28, sizeof(unsigned int));


	switch (opcode) {

	case 900:
		printf("[*] Calling Function1\n");
		message = "[+] Function1 completed | ";
		Function1(clientSocket, (char*)userInput);
		send(clientSocket, message, strlen(message), 0);
		break;
		
	case 901:
		printf("[*] Calling Function2\n");
		message = "[+] Function2 completed | ";
		Function2(clientSocket, (char*)userInput);
		send(clientSocket, message, strlen(message), 0);
		break;

	case 902:
		printf("[*] Calling Functoin3\n");
		message = "[+] Function3 completed | ";
		Function3(clientSocket, (char*)userInput);
		send(clientSocket, message, strlen(message), 0);
		break;

	case 903:
		printf("[*] Calling Function4\n");
		message = "[+] Function4 completed | ";
		Function4((char*)userInput);
		send(clientSocket, message, strlen(message), 0);
		break;

	case 905:
		printf("[*] Calling Function5\n");
		message = "[+] Function5 completed | ";
		Function5(clientSocket, (char*)userInput);
		send(clientSocket, message, strlen(message), 0);
		break;


	default:
		message = "[-] Opcode is not correct | ";
		send(clientSocket, message, strlen(message), 0);
		closesocket(clientSocket);
		break;
	}

		

}


int main() {

	WSADATA wsa;
	SOCKET s, newSocket;
	struct sockaddr_in server, client;
	int c;

	printf("[*] Initializing WS2_32.dll....\n");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("[-] Initializing WS2_32.dll failed\n");
		exit(1);
	}
	printf("[+] WS2_32.dll initialized successfully\n");

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
	printf("[+] Listening incoming connection\n");

	c = sizeof(struct sockaddr_in);

	while ((newSocket = accept(s, (struct sockaddr*)&client, &c)) != SOCKET_ERROR) {
		printf("[+] Client connected\n");
		_beginthread(&handleConnection, 0, newSocket);
	}
	if (newSocket == SOCKET_ERROR) {
		printf("Accept failed\n");
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


int getFunctionAddress() {
	void* address = &main;

	return (int)address;
}