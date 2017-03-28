#include "net.h"
#include "receiver.h"
#include "utility.h"
#include "logger.h"
#include "packets.h"
using namespace TMY;
using namespace std;

int Receiver::waitDirInfo(DirInfo& dirinfo) {
	unique_ptr<Readbuf_> readbuf(new Readbuf_(fd));
    static string msg = "";
    char buf[READBUFN+1];

	int n, m;
    msg.clear();
	while ((m = readbuf->readto(buf, '\n')) > 0) {
		buf[m] = 0;
		msg += buf;
		if (buf[m - 1] == '\n') break;
	}

	if (m == 0)
		return TCLOSE;
	if (m < 0)
		return TTIMEOUT;

	if (msg != "DirInfo\n") {
		logger("Received %s but expect DirInfo", msg.c_str());
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
    if (*msg.rbegin() == BRKCHR) *msg.rbegin() = 0;

	/* parse */

	dirinfo.clear();
	json header = json::parse(msg.c_str());

	if (!header.is_array())
		throw new exception();

	for (auto &e : header) {
		DirInfoEntry h;
		h.len = e["len"];
		h.md5 = e["md5"].get<string>();
		h.modtime = str2time(e["modtime"].get<string>());
		h.filePath.filename = e["filename"].get<string>();
		h.filePath.pathArr = move(str2PathArr(e["path"].get<string>()));
		dirinfo.push_back(h);
	}

    return 0;
}

int Receiver::sendPull(const PullReq& pullreq) {
	string msg = "Pull\n";
	json header;

	for (auto &e : pullreq) {
		json path = e.filePath.toJSON();
		json h = {
			{ "filename", path["filename"] },
			{ "path", path["path"] },
			{ "len", e.len },
			{ "offset", e.offset }
		};
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

int Receiver::waitPush(PushReq& pushreq) {
	unique_ptr<Readbuf_> readbuf(new Readbuf_(fd));
	string msg = "";
    char buf[READBUFN + 1];

	int n, m;
	while ((m = readbuf->readto(buf, '\n')) > 0) {
		buf[m] = 0;
		msg += buf;
		if (buf[m - 1] == '\n') break;
	}

	if (m == 0)
		return TCLOSE;
	if (m < 0)
		return TTIMEOUT;

	if (msg != "Push\n") {
        if(msg == "done\n") {
            logger("Peer close socket");
            tclose(fd);
            return -1;
        }
		logger("Received %s but expect Push", msg.c_str());
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
    if (*msg.rbegin() == BRKCHR) *msg.rbegin() = 0;

	pushreq.clear();
	json header = json::parse(msg.c_str());

	if (!header.is_array())
		throw new exception();

	for (auto &e : header) {
		PushReqEntry h;
		h.len = e["len"];
		h.offset = e["offset"];
		h.filePath.filename = e["filename"].get<string>();
		h.filePath.pathArr = move(str2PathArr(e["path"].get<string>()));
		h.buffer = new char[h.len];

		int n;
		if ((n = readbuf->readn(h.buffer, h.len)) != 0) {
			return n;
		}

		pushreq.push_back(h);
	}

    return 0;
}

Receiver::~Receiver() {
	tclose(fd);
}
