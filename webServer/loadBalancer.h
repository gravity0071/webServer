#include "Iocp.h"
#include<vector>
#include <winsock2.h>


class LoadBalancer {
public:
	LoadBalancer(const std::vector<Iocp>& iocp);
	~LoadBalancer();

	void acceptNewClient(SOCKET clientSock);

private:
	std::vector<Iocp> iocps;
	size_t nextIndex = 0;

};