#pragma once
#include "tsock.h"
#include "packets.h"
#include "sender.h"
#include "receiver.h"
#include <functional>
#include <string>

namespace TMY {

/* 初次，增量，恢复 */
enum TunnelMode { INIT, INCR, RECV };
enum TunnelRole { SENDER = 0, RECEIVER = 1 };

class Controller;
typedef std::shared_ptr<Controller> Controller_ptr;

class Controller {
	friend class Listener;	
private:
	int fd;
	SA addr;
	//Session session;
	std::string session;
public:
	/* 以auth为凭证连接addr，同时设置对方要求建立receiver时的handler
	 * 失败返回 nullptr 
	 */

	Controller(int fd_, const SA* addr_, socklen_t addrlen_);
	static int connect(const SA* addr, Controller_ptr&, bool blocking = 0, timeval timeout = { 1, 0 });

	/* 重新连接 
	 * 失败返回 -1
	 */
	int reconnect();

	/*-------- Client用函数 ---------*/

	/* 注册 
	 * struct SignupRes {
	 *	 int code,
	 *	 string session,
	 *	 string message
	 * }
	 * 见protocol.md
	 */
	/* 小心：register是保留字 */
	int signup(const SignupReq&, SignupRes&);
	int login(const LoginReq&, LoginRes&);

	/*----------- Server用函数-------------*/

	int waitLogin(LoginReq&);
	int waitSignup(SignupReq&);
	// type == 0 => Login, type == 1 => signup
	int waitLoginSignup(LoginReq&, int &type);
	int sendLoginRes(const LoginRes&);
	int sendSignupRes(const SignupRes&);

	/*-----------两端都用函数---------------*/

	/* 客户机/服务器的实现有区别。因为客户机在子网中，只能由服务器监听，客户机连接  */
	int makeSender(TunnelMode mode, Sender_ptr&);
	int makeReceiver(TunnelMode mode, Receiver_ptr&);
	
	/* 返回sockaddr_in */
	SA* Addr();
	/* 默认timeout = 1s */
	timeval timeout;
	bool blocking;
};

}