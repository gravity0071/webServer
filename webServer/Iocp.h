#include "WorkerThread.cpp"
#include <winsock2.h>
#include <windows.h>
#include <mswsock.h>
#include <iostream>
#pragma comment(lib, "ws2_32.lib")


class Iocp {
public:
	Iocp(int runningWorkers);
	~Iocp();

	void acceptNewSock(SOCKET clientSock);

private:
	HANDLE iocpHandle_;
	std::vector<HANDLE> workerHandles_;
	int workerCount = 0;

	void startWorkers(int numThreads);
	static DWORD WINAPI workerThread(LPVOID lpParam);
};