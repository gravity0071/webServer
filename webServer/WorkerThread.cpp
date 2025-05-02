#include <unordered_map>
#include <string>
#include <winsock2.h>
#include <windows.h>
#include <iostream>

class WorkerThread {
public: 
    DWORD WINAPI work(HANDLE iocp) {
        std::cout << "worked!" << std::endl;
        DWORD bytesTransferred;
        ULONG_PTR key;
        PER_IO_DATA* ioData;

        while (true) {
            BOOL success = GetQueuedCompletionStatus(iocp, &bytesTransferred, &key, (LPOVERLAPPED*)&ioData, INFINITE);
            if (!success || (bytesTransferred == 0 && key == NULL && ioData == nullptr)) {
                std::cout << "Worker thread received shutdown signal.\n";
                break;  // Exit the worker thread cleanly
            }

            if (!success || bytesTransferred == 0) {
                // I/O failed or client disconnected
                if (ioData) {
                    closesocket(ioData->clientSocket);
                    connectionStates.erase(ioData->clientSocket);
                    delete ioData;
                }
                continue;
            }

            SOCKET clientSock = ioData->clientSocket;
            ConnectionState& state = connectionStates[clientSock];
            state.recvBuffer.append(ioData->buffer, bytesTransferred);

            while (true) {
                if (!state.headerParsed) {
                    if (headerComplete(state.recvBuffer)) {
                        state.headerParsed = true;
                        size_t headerEnd = state.recvBuffer.find("\r\n\r\n") + 4;
                        std::string headerPart = state.recvBuffer.substr(0, headerEnd);
                        state.contentLength = parseContentLength(headerPart);

                        if (state.contentLength == 0) {
                            // GET ??,? body
                            std::cout << "????HTTP GET??:\n" << headerPart << std::endl;
                            state.recvBuffer.erase(0, headerEnd);
                            state.headerParsed = false; // ??,???????
                            continue;
                        }
                    }
                    else {
                        // ??????,???
                        break;
                    }
                }

                if (state.headerParsed) {
                    size_t headerEnd = state.recvBuffer.find("\r\n\r\n") + 4;
                    size_t bodySize = state.recvBuffer.size() - headerEnd;

                    if (bodySize >= state.contentLength) {
                        // Body ????
                        std::string headerPart = state.recvBuffer.substr(0, headerEnd);
                        std::string bodyPart = state.recvBuffer.substr(headerEnd, state.contentLength);

                        std::cout << "????HTTP POST??:\n";
                        std::cout << headerPart;
                        std::cout << bodyPart << std::endl;

                        // ????????,???????
                        state.recvBuffer.erase(0, headerEnd + state.contentLength);
                        state.headerParsed = false;
                        state.contentLength = 0;
                        continue;
                    }
                    else {
                        // Body ????
                        break;
                    }
                }
            }

            // ???????,??? PostRecv
            PostRecv(clientSock, iocp);

            delete ioData;
        }

        return 0;
    }
private: 
    struct PER_IO_DATA {
        OVERLAPPED overlapped;
        char buffer[1024];
        WSABUF wsabuf;
        SOCKET clientSocket;
     };

    // ?????????
    struct ConnectionState {
        std::string recvBuffer; // ???
        bool headerParsed = false;
        size_t contentLength = 0;
    };

    std::unordered_map<SOCKET, ConnectionState> connectionStates;

    bool headerComplete(const std::string& buffer) {
        return buffer.find("\r\n\r\n") != std::string::npos;
    }

    size_t parseContentLength(const std::string& header) {
        size_t pos = header.find("Content-Length:");
        if (pos == std::string::npos) return 0;

        size_t start = pos + strlen("Content-Length:");
        while (start < header.size() && (header[start] == ' ' || header[start] == '\t')) start++;

        size_t end = start;
        while (end < header.size() && isdigit(header[end])) end++;

        return std::stoul(header.substr(start, end - start));
    }

    // ????? WSARecv
    void PostRecv(SOCKET clientSock, HANDLE iocp) {
        PER_IO_DATA* ioData = new PER_IO_DATA();
        ZeroMemory(&ioData->overlapped, sizeof(OVERLAPPED));
        ioData->clientSocket = clientSock;
        ioData->wsabuf.buf = ioData->buffer;
        ioData->wsabuf.len = sizeof(ioData->buffer);

        DWORD flags = 0, bytes = 0;
        WSARecv(clientSock, &ioData->wsabuf, 1, &bytes, &flags, &ioData->overlapped, NULL);
    }
};
