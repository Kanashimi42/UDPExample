
#define WIN32_LEAN_AND_MEAN

#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <string>
using namespace std;

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 4096

#define SERVER_IP "127.0.0.1"
#define DEFAULT_PORT "8888"

COORD point{ 0, 0 }; //  оордината точки
HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE); // ѕолучаем адрес собственного окна

SOCKET client_socket;
bool fFirst = true;
struct Info {
    char name[DEFAULT_BUFLEN];
    int color;
};
Info info;

DWORD WINAPI Sender(void* param) {
    point.Y = 13;
    SetConsoleCursorPosition(h, point);

    while (true) {
        char str[DEFAULT_BUFLEN] = "";

        if (fFirst) {
            fFirst = false;

            strcat_s(str, sizeof(str), "\n\tNew user: ");
            strcat_s(str, sizeof(str), info.name);

            send(client_socket, str, strlen(str), 0);
            cout << endl;
        }

        char query[DEFAULT_BUFLEN];

        cout << "Enter msg: ";
        cin.getline(query, DEFAULT_BUFLEN);

        send(client_socket, str, strlen(str), 0);
        cout << endl;
    }
}

DWORD WINAPI Receiver(void* param) {

    while (true) {
        char response[DEFAULT_BUFLEN];
        int result = recv(client_socket, response, DEFAULT_BUFLEN, 0);
        response[result] = '\0';

        cout << response << endl;
    }
}

BOOL ExitHandler(DWORD whatHappening) {
    switch (whatHappening)
    {
    case CTRL_C_EVENT: 
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
      return(TRUE);
        break;
    default:
        return FALSE;
    }
}

int main()
{
    // обработчик закрыти€ окна консоли

    system("title Client");
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }
    addrinfo hints = {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // разрешить адрес сервера и порт
    addrinfo* result = nullptr;
    iResult = getaddrinfo(SERVER_IP, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 2;
    }

    addrinfo* ptr = nullptr;
    // пытатьс€ подключитьс€ к адресу, пока не удастс€
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        // создать сокет на стороне клиента дл€ подключени€ к серверу
        client_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

        if (client_socket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 3;
        }
        iResult = connect(client_socket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(client_socket);
            client_socket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (client_socket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 5;
    }

    cout << "Enter your name: ";
    cin.getline(info.name, DEFAULT_BUFLEN);

    cout << "Enter your color(number): ";
    cin >> info.color;

    CreateThread(0, 0, Sender, 0, 0, 0);
    CreateThread(0, 0, Receiver, 0, 0, 0);

    Sleep(INFINITE);
}