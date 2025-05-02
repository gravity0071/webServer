#include "Iocp.h"

Iocp::Iocp(int runningWorkers) {
	iocpHandle_ =CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	std::cout << "starting workers" << std::endl;
	startWorkers(runningWorkers);
}

Iocp::~Iocp() {
    std::cout << "Shutting down IOCP..." << std::endl;

    // Send one shutdown signal per thread
    for (int i = 0; i < workerCount; ++i) {
        PostQueuedCompletionStatus(iocpHandle_, 0, NULL, nullptr);
    }

    // Wait for threads to finish
    if (!workerHandles_.empty()) {
        WaitForMultipleObjects(workerHandles_.size(), workerHandles_.data(), TRUE, INFINITE);
    }

    // Close thread handles
    for (HANDLE h : workerHandles_) {
        CloseHandle(h);
    }

    workerHandles_.clear();

    // Close IOCP handle
    if (iocpHandle_) {
        CloseHandle(iocpHandle_);
        iocpHandle_ = nullptr;
    }

    std::cout << "IOCP shutdown complete." << std::endl;
}


void Iocp::acceptNewSock(SOCKET clientSock) {

}

void Iocp::startWorkers(int numThreads) {
    workerCount = numThreads;
    for (int i = 0; i < numThreads; ++i) {
        HANDLE hThread = CreateThread(
            nullptr,
            0,
            Iocp::workerThread,
            this,
            0,
            nullptr
        );

        if (!hThread) {
            std::cerr << "CreateThread failed: " << GetLastError() << std::endl;
        }
        else {
            workerHandles_.push_back(hThread);
        }
    }
}


DWORD WINAPI Iocp::workerThread(LPVOID lpParam) {
    HANDLE iocp = static_cast<Iocp*>(lpParam)->iocpHandle_;

    WorkerThread* w = new WorkerThread();

    std::cout << "ready to work" << std::endl;
    w->work(iocp);

    delete w;
    std::cout << "worker thread exiting\n";
    return 0;
}