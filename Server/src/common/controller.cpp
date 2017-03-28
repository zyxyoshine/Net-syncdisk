#include "controller.h"
#include "packets.h"
#include "receiver.h"
#include "sender.h"
#include "logger.h"
#include "utility.h"
#include <cstring>
#include <exception>
#include <memory>
using namespace std;

namespace TMY {

Controller::Controller(int fd_, const SA* addr_, socklen_t addrlen_) {
    if(addrlen_ != sizeof(SA4))
        throw new std::exception();

	fd = fd_;
    memcpy(&addr, addr_, sizeof(SA));

    /* 默认1s超时 */
    timeout = { 1, 0 };
    blocking = false;
}

static int connectto(int &fd, const SA* addr, bool blocking = 0, timeval timeout = { 1, 0 } ) {
	fd = socket(AF_INET, SOCK_STREAM, 0);
	int n;

	setnb(fd);

	fd_set rset, wset;

	if ((n = ::connect(fd, addr, sizeof(SA))) < 0)
		if (terrno() != TEWOULDBLOCK)
			return -1;

	if (n == 0) /* done */
		return 0;

	FD_ZERO(&rset); FD_SET(fd, &rset);
	wset = rset;

	if ((n = select(fd + 1, &rset, &wset, NULL, blocking ? NULL : &timeout)) == 0)
		return TTIMEOUT;

	if (n == 0)
		return TERROR;

	if (FD_ISSET(fd, &rset) || FD_ISSET(fd, &wset)) {
		char error[255];
		int len = sizeof(error);
		if (getsockopt(fd, SOL_SOCKET, SO_ERROR, error, (socklen_t *)&len) < 0)
			return TERROR;
	}
	else
		return TERROR;

	setnb(fd, true);
	return 0;
}

/* TODO */
int Controller::connect(const SA* addr, Controller_ptr& controller, bool blocking, timeval timeout) {
	int n, fd;
	if ((n = connectto(fd, addr, blocking, timeout)) != 0)
		return n;

	controller = make_shared<Controller>(fd, addr, sizeof(SA));
	
	return 0;
}


int Controller::reconnect() {
	tclose(fd);

	int n;
	if ((n = connectto(fd, &addr, blocking, timeout)) != 0)
		return n;

	return 0;
}

int Controller::signup(const SignupReq& signupReq, SignupRes& signupRes) {
	/* Send Sign */
	string msg = "Signup\n";
	json header = {
		{ "username", signupReq.username },
		{ "password", signupReq.password },
		{ "session", signupReq.session }
	};
	msg += header.dump();
	int n = 0, m = 0;

	while (n < msg.size()) {
		if (!(blockt(fd, TWRITE, &timeout) | TWRITE))
			return TTIMEOUT;
		if ((m = send(fd, msg.c_str() + n, msg.length() - n, 0)) < 0)
			return -1;
		n += m;
	}

	blockt(fd, TWRITE, &timeout);
	if ((m = send(fd, "\0", 1, 0)) < 0)
		return -1;

	logger("Sent Signup");

	/* Wait for response */
	unique_ptr<Readbuf_> readbuf(new Readbuf_(fd));
	msg = "";
	char buf[READBUFN];
	while ((m = readbuf->readto(buf, '\n')) > 0) {
		buf[m] = 0;
		msg += buf;
		if (buf[m - 1] == '\n') break;
	}

	if (m < 0)
		return m;

	if (m == 0)
		return TCLOSE;

	if (msg != "SignupRes\n") {
		logger("Receive %s but not Signup", msg.c_str());
		return TERROR;
	}

	msg = "";

	while ((m = readbuf->readto(buf, BRKCHR)) > 0) {
		buf[m] = 0;
		msg += buf;
		if (buf[m - 1] == BRKCHR) break;
	}

	if (m == 0)
		return TCLOSE;

	msg[m - 1] = 0;
	json Signres_ = json::parse(msg.c_str());
	signupRes = { Signres_["code"], Signres_["message"], Signres_["session"] };

	/* Controller自己也记录 session 用于 makeSender 和 makeReceiver */
	session = signupRes.session;

	return 0;
}

int Controller::login(const LoginReq& loginReq, LoginRes& loginRes) {
	
	/* Send Login */
	string msg = "Login\n";
	json header = {
		{"username", loginReq.username},
		{"password", loginReq.password},
		{"session", loginReq.session}
	};
	msg += header.dump();
	int n = 0, m = 0;

	while (n < msg.size()) {
		if (!(blockt(fd, TWRITE, &timeout) | TWRITE))
			return TTIMEOUT;
		if ((m = send(fd, msg.c_str() + n, msg.length() - n, 0)) < 0)
			return -1;
		n += m;
	}

	blockt(fd, TWRITE, &timeout);
	if ((m = send(fd, "\0", 1, 0)) < 0)
		return -1;

	logger("Sent login");

	/* Wait for response */
	unique_ptr<Readbuf_> readbuf(new Readbuf_(fd));
	msg = "";
	char buf[READBUFN];
	while ((m = readbuf->readto(buf, '\n')) > 0) {
		buf[m] = 0;
		msg += buf;
		if (buf[m - 1] == '\n') break;
	}

	if (m < 0)
		return m;

	if (m == 0)
		return TCLOSE;

	if (msg != "LoginRes\n") {
		logger("Receive %s but not Login", msg.c_str());
		return TERROR;
	}

	msg = "";

	while ((m = readbuf->readto(buf, BRKCHR)) > 0) {
		buf[m] = 0;
		msg += buf;
		if (buf[m - 1] == BRKCHR) break;
	}

	if (m == 0)
		return TCLOSE;

	if (!*msg.rbegin() == BRKCHR) *msg.rbegin() = 0;
	json loginres_ = json::parse(msg.c_str());
	loginRes = { loginres_["code"], loginres_["message"], loginres_["session"] };

	/* Controller自己也记录 session 用于 makeSender 和 makeReceiver */
	session = loginRes.session;

	return 0;
}

int Controller::waitLogin(LoginReq& loginReq) {
	int type;
	return waitLoginSignup(loginReq, type);
}

int Controller::waitSignup(SignupReq& signupReq) {
	int type;
	return waitLoginSignup(signupReq, type);
}
// type == 0 => Login, type == 1 => signup
int Controller::waitLoginSignup(LoginReq& loginReq, int &type) {
	unique_ptr<Readbuf_> readbuf(new Readbuf_(fd));
	string msg;
	char buf[READBUFN];
	int n, m;

	while ((m = readbuf->readto(buf, '\n')) > 0) {
		buf[m] = 0;
		msg += buf;
		if (buf[m - 1] == '\n') break;
	}

	if (m == 0)
		return TCLOSE;
	if (m < 0)
		return m;

	if (msg == "Login\n") {
		type = 0;
	}
	else if (msg == "Signup\n") {
		type = 1;
	}
	else {
		logger("Receive %s while expect Login", msg.c_str());
		return TERROR;
	}

	msg = "";
	while ((m = readbuf->readto(buf, BRKCHR)) > 0) {
		buf[m] = 0;
		msg += buf;
		if (buf[m - 1] == BRKCHR) break;
	}

	if (m == 0)
		return TCLOSE;
	if (m < 0)
		return m;

	if (!*msg.rbegin() == BRKCHR) *msg.rbegin() = 0;
	auto loginReq_ = json::parse(msg.c_str());
	loginReq = {
		loginReq_["username"],
		loginReq_["password"]
	};

	return 0;
}

int Controller::sendLoginRes(const LoginRes& loginres) {
	string msg = "LoginRes\n";
	json header = {
		{"code", loginres.code },
		{"session", loginres.session } ,
		{"message", loginres.message }
	};
	msg += header.dump();
	msg += "\0";

	int n = 0, m = 0, len = msg.size() + 1;
	while (n < len) {
		if (!(blockt(fd, TWRITE, &timeout) | TWRITE))
			return TTIMEOUT;
		if ((m = send(fd, msg.c_str() + n, len - n, 0)) < 0)
			return -1;
		n += m;
	}

	return 0;
}

int Controller::sendSignupRes(const SignupRes& signupRes) {

	return 0;
}

int Controller::makeSender(TunnelMode mode, Sender_ptr& sender) {

	if (session.empty())
		return TNOSESSION;

	int nfd, n;
	if ((n = connectto(nfd, &addr)) != 0)
		return n;

	string msg = "Tunnel\n";
	json header = {
		{"role", SENDER},
		{"mode", mode},
		{"session", session}
	};
	msg += header.dump();
	msg += "\0";

	n = 0;
	int m = 0, len = msg.size() + 1;
	while (n < len) {
		if (!(blockt(fd, TWRITE, &timeout) | TWRITE))
			return TTIMEOUT;
		if ((m = send(fd, msg.c_str() + n, len - n, 0)) < 0)
			return -1;
		n += m;
	}

	sender = Sender_ptr(new Sender());

	sender->fd = nfd;
	sender->session = session;

	return 0;
}

int Controller::makeReceiver(TunnelMode mode, Receiver_ptr& receiver) {

	if (session.empty())
		return TNOSESSION;

	int nfd, n;
	if ((n = connectto(nfd, &addr)) != 0)
		return n;

	string msg = "Tunnel\n";
	json header = {
		{ "role", RECEIVER },
		{ "mode", mode },
		{ "session", session }
	};
	msg += header.dump();
	msg += "\0";

	n = 0;
	int m = 0, len = msg.size() + 1;
	while (n < len) {
		if (!(blockt(fd, TWRITE, &timeout) | TWRITE))
			return TTIMEOUT;
		if ((m = send(fd, msg.c_str() + n, len - n, 0)) < 0)
			return -1;
		n += m;
	}

	receiver = Receiver_ptr(new Receiver());

	receiver->fd = nfd;
	receiver->session = session;

	return 0;
}

SA* Controller::Addr() {
	return &addr;
}


}