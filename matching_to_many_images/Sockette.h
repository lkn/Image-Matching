// windows socket wrapper for tcp
#include <winsock2.h>

#ifndef SOCKETTE_H__
#define SOCKETTE_H__

class Sockette {
public:
	Sockette(u_short port);
	Sockette(SOCKET socket);

	~Sockette();
	
	void StartListening();
	bool Listen(char **data);
	const SOCKET& handle() const { return handle_; }

private:
	void Init();

	SOCKET handle_;
	u_short port_;
	unsigned long address_;
};

#endif