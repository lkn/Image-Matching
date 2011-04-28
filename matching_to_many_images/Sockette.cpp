#include <iostream>
#include <winsock2.h>
#include "Sockette.h"

// Create the socket
Sockette::Sockette(u_short port) : port_(port) {	
	Init();
}

Sockette::Sockette(SOCKET s) {
	handle_ = s;
	SOCKADDR_IN sadi;
	int size = sizeof(sadi);

	if (getpeername(handle_, (SOCKADDR *) &sadi, &size) == SOCKET_ERROR) {
		std::cerr << "Error constructing Sockette from SOCKET!!\n";	
		switch (WSAGetLastError()) {
			// Something went wrong, and we can't construct 
			case WSANOTINITIALISED:
				std::cerr << "->1\n";
				break;
			case WSAENETDOWN:
				std::cerr << "->2\n";
				break;
			case WSAEFAULT:
				std::cerr << "->3\n";
				break;
			case WSAEINPROGRESS:
				std::cerr << "->4\n";
				break;
			case WSAENOTCONN:
				std::cerr << "->5\n";
				break;
			case WSAENOTSOCK:
				std::cerr << "->6\n";
				break;
		}
		WSACleanup();
		system("PAUSE");
	}

	address_ = ntohl(inet_addr((char *) sadi.sin_addr.S_un.S_addr));
	port_ = sadi.sin_port;
	BOOL tru = TRUE;
	setsockopt(handle_, IPPROTO_TCP, TCP_NODELAY, (char *) &tru, sizeof(tru));
}

// Creates and binds to socket
void Sockette::Init() {
	handle_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (handle_ == INVALID_SOCKET)	{
		std::cout << "Socket creation failed.\n";
		WSACleanup();
		system("PAUSE");
	}
}

void Sockette::StartListening() {
	SOCKADDR_IN serverInf;
	serverInf.sin_family = AF_INET;
	serverInf.sin_addr.s_addr = INADDR_ANY;
	serverInf.sin_port = htons(port_);
	if (bind(handle_, (SOCKADDR *) (&serverInf), sizeof(serverInf)) == SOCKET_ERROR) {
		std::cout << "Unable to bind socket!\n";
		WSACleanup();
		system("PAUSE");
	}
	listen(handle_, 5);  // TODO: make configurable
}

// Whoever sent in 'data' must deallocate it (this function allocates it)
bool Sockette::Listen(char **data) {
	unsigned char * tmp = (unsigned char *) calloc(10, sizeof(unsigned char));
	if (tmp == NULL) std::cerr << "calloc returned NULL";

	short expectedSize = 0;
	while (true) {
		int n = recv(handle_, (char *) tmp, 2, 0);
		if (n == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAEWOULDBLOCK) continue;
			std::cerr << "Error getting packet size: " << WSAGetLastError() << std::endl;
			return false;
		}
		
		expectedSize = tmp[0] << 8 | tmp[1];  // assumes big endian I think...
		std::cout << "Expected size of data: " << expectedSize << std::endl;
		break;
	}

	*data = new char[expectedSize];  // specifically this to keep OpenCV happy
	int numBytesRead = 0;
	while (numBytesRead < expectedSize) {
		int r = recv(handle_, (char *)(*data + numBytesRead), expectedSize - numBytesRead, 0);

		if (r == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAEWOULDBLOCK) continue;
			std::cerr << "Couldn't receive, error: " << WSAGetLastError();
			return false;
		}
		numBytesRead += r;
	}
	std::cout << "Actually read " << numBytesRead << std::endl;
	delete tmp;
	return true;
}

Sockette::~Sockette() {
	closesocket(handle_);
}
