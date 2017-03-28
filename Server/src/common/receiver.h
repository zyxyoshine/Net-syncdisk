#pragma once

#include "receiver.h"
#include "controller.h"
#include "packets.h"
#include <functional>
// #include "progressinfo.h"

namespace TMY {

class Receiver {
	friend class Controller;
private:
	int fd;
	std::string session;
public:

	int waitDirInfo(DirInfo&);
	int sendPull(const PullReq&);
	int waitPush(PushReq&);
	~Receiver();
};

typedef std::shared_ptr<Receiver> Receiver_ptr;

}