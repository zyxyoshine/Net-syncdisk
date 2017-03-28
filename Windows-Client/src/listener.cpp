
#include "net.h"
#include "controller.h"

namespace TMY {

int Listener::listen(int port) {

    SA4 addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }

    if(bind(fd, (const SA*) &addr, sizeof(addr)) < 0) {
        return -1;
    }

    #define BACKLOG 100
    ::listen(fd, BACKLOG);

    return 0;
}

Controller_ptr Listener::waitClient() {
    SA4 cliaddr;
    socklen_t len;
    int clifd = accept(fd, (SA*)&cliaddr, &len);
    std::make_shared<Controller>(clifd, (const SA*)&cliaddr, len);
    return nullptr;
}

void Listener::close() {
    ::tclose(fd);
}

}