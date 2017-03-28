#pragma once

#include "sender.h"
#include "packets.h"
#include <string>

namespace TMY {

class Controller;
class Sender {
	friend class Controller;
private:
	int fd;
	std::string session;
public:
	~Sender();

	int push(const PushReq&);	
	int sendDirInfo(const DirInfo&);
	int waitPull(PullReq&);
};

typedef std::shared_ptr<Sender> Sender_ptr;

}
