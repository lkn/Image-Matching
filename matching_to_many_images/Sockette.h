// windows socket wrapper for tcp
#include <winsock2.h>
#include <string>

#ifndef SOCKETTE_H__
#define SOCKETTE_H__

class Sockette {
public:
	Sockette(u_short port);
	Sockette(SOCKET socket);

	~Sockette();
	
	void StartListening();
	bool Listen(char **data);
	bool Send(std::string data);

	const SOCKET& handle() const { return handle_; }
	unsigned long address() const { return address_; }
	u_short port() const { return port_; }

private:
	void Create();

	SOCKET handle_;
	u_short port_;
	unsigned long address_;
};

#endif