#include "packets.h"
#include "sender.h"
#include "utility.h"
#include "logger.h"
#include "tsock.h"
#include <cstdio>
#include <string>
#include <QDebug>

using namespace TMY;
using namespace std;

int Sender::push(const PushReq& pushReq) {

	string msg = "Push\n";
	json header;
    qDebug()<<"PUSH 1";
	for (auto &e:pushReq) {
		auto h = e.filePath.toJSON();
		h["offset"] = e.offset;
		h["len"] = e.len;
		header.push_back(h);
	}
    qDebug()<<"PUSH 2";

    msg += header.dump();
    //msg += BRKCHR;

	int n;
    if ((n = sendn(fd, msg.c_str(), msg.size() + 1) < 0))
		return n;
    qDebug()<<"PUSH 3";

	/* Binary data */
	for (auto &e : pushReq) {
		if ((n = sendn(fd, e.buffer, e.len)) < 0)
			return n;
        char lll[50];
        sprintf(lll, "BUFFER: %x \n", (void *)(e.buffer));
        qDebug() << QString(lll);
        delete[] e.buffer;
	}

    qDebug()<<"PUSH 4";
    return 0;
}

int Sender::sendDirInfo(const DirInfo& dirinfo) {

	string msg = "DirInfo\n";
	json header;

	for (auto &e:dirinfo) {
		json h = e.filePath.toJSON();
		h["modtime"] = time2str(e.modtime);
		h["md5"] = e.md5;
		h["len"] = e.len;
		if (e.chunks.size() > 0) {
			h["chunks"] = e.chunks.toJSON();
		}
		header.push_back(h);
	}

	msg += header.dump();
	msg += "\0";

	int n;
	if ((n = sendn(fd, msg.c_str(), msg.size() + 1)) != 0) {
		return n;
	}

    return 0;
}

int Sender::waitPull(PullReq& pullreq) {

	unique_ptr<Readbuf_> readbuf(new Readbuf_(fd));
	string msg = "";
	char buf[READBUFN];
qDebug() << 1;
	int n, m;
	while ((m = readbuf->readto(buf, '\n')) > 0) {
		buf[m] = 0;
		msg += buf;
		if (buf[m - 1] == '\n') break;
	}
qDebug() << 2;

	if (m == 0)
		return TCLOSE;
	if (m < 0)
		return m;

	if (msg != "Pull\n") {
		logger("Received %s but expect Pull", msg.c_str());
		return TERROR;
	}

	msg = "";
	while ((m = readbuf->readto(buf, BRKCHR)) > 0) {
		buf[m] = 0;
		msg += buf;
		if (buf[m - 1] == BRKCHR) break;
	}
qDebug() << 3;

	if (m == 0)
		return TCLOSE;
	if (m < 0)
		return m;
    if (*msg.rbegin() == BRKCHR) *msg.rbegin() = 0;

	/* parse */

	pullreq.clear();
	json header = json::parse(msg.c_str());

	if (!header.is_array())
		throw new exception();

	for (auto &e : header) {
		PullReqEntry h;
		h.len = e["len"];
		h.offset = e["offset"];
		h.filePath.filename = e["filename"].get<string>();
		h.filePath.pathArr = move(str2PathArr(e["path"].get<string>()));
		pullreq.push_back(h);
	}

    return 0;
}

Sender::~Sender() {
	tclose(fd);
}
