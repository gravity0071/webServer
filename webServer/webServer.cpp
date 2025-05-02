#include "webServer.h"
#include "Iocp.h"

#include <winsock2.h>
#include <windows.h>
#include <mswsock.h>
#include <iostream>
#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    auto iocp1 = Iocp(1);

    return 0;
}