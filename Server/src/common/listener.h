#pragma once
#include "controller.h"
#include "listener.h"
#include <functional>

namespace TMY {

//typedef std::function<int(Auth, Session&)> AuthService;

class Listener {
private:
    int fd;
public:
    Listener(): fd(-1) {}

	Controller_ptr waitClient();
	int listen(int port);
	
    void close();
};

}
