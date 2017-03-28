#include "../common/net.h"
#include "../common/utility.h"
#include "logger.h"
#include "mysql/mysql.h"
#include "../common/json.hpp"
#include <string>
#include <memory>
#include <exception>
#include <map>
#include <algorithm>
#include <sys/sendfile.h>
#include "database.hpp"
using namespace std;
using namespace TMY;

extern char *srvrole;

const string StorePath = "/home/G2652/store";
const string ChunksStorePath = "/home/G2652/chunks";

typedef shared_ptr<Readbuf_> Rdbuf_ptr;

struct DHeader {
    string type;
    json header;
    int read(Rdbuf_ptr readbuf);
    int send(int fd);
};

int DHeader::read(Rdbuf_ptr readbuf) {

    type.clear();
    header.clear();

    static string msg;
    char buf[READBUFN];
    int n, m;

    msg = "";
    while((n = readbuf->readto(buf, '\n')) > 0) {
        logger("receive %d bytes", n);
        buf[n] = 0;
        msg += buf;
        if(buf[n-1] == '\n') break;
    }

    logger("read %s", msg.c_str());

    if(n < 0)
        return n;
    if(n == 0)
        return TCLOSE;
    
    // last byte is '\n'
    msg.resize(msg.size()-1);
    type = msg;

    /* header */
    msg = "";

    while((n = readbuf->readto(buf, '\0')) > 0) {
        buf[n] = 0;
        msg += buf;
        logger("msg.length()=%d", msg.length());
        if(buf[n-1] == '\0') break;
    }

    logger("read %s", msg.c_str());

    if(n < 0)
        return m;
    if(n == 0)
        return TCLOSE;

    logger("Parsing header...");
    header = json::parse(msg.c_str());
    logger("Parse header sucessfully");

    return 0;
}

int DHeader::send(int fd) {
    string msg = type + "\n";
    msg += header.dump();
    int n = msg.size() + 1, m;
    return sendn(fd, msg.c_str(), n);
}

MYSQL* connect_db();
/* GLOBAL status */
MYSQL* db;
shared_ptr<Readbuf_> readbuf;
int cfd;
string csession = "";

string newClient(const string& username) {
    string cid;
    for(int i = 0; i < 32; i++)
        cid += rand() % 26 + 'a';
    
    /* Update Table Client */

    int uid = getUidByName(username);

    if(uid < 0) {
        logger("Fatal: User %s not found", username.c_str());
        return "";
    }

    createNewClient(uid, cid);

    return cid;
}

int login(json header) {
    
    logger("Received Login request");

    string username = header["username"].get<string>();
    string session  = header["session"].get<string>();
    string passwd = header["password"].get<string>();
    string passwd_md5 = get_md5(passwd);

    char query[255];
    snprintf(query, sizeof(query), 
        "select username, passwd from user where username=\"%s\""
        , username.c_str());

    if (mysql_query(db, query)) 
    {
        finish_with_error(db);  
        return -1;
    }

    MYSQL_RES *result = mysql_store_result(db);
    
    if (result == NULL) 
    {
        finish_with_error(db);
    }

    int rcode;
    MYSQL_ROW row;
    if(!(row = mysql_fetch_row(result))) {
        /* user not found */
        DHeader dh = {
            "LoginRes", {
                {"code", 1},
                {"session", ""},
                {"message", "用户名不存在"}
            }
        };
        logger("User %s not found", username.c_str());
        if(dh.send(cfd) != 0) 
            rcode = -1;
        else rcode = 0;
        logger("Closing the socket");
        close(cfd);
    }
    else
        /* wrong password */
        if(string(row[1]) != passwd_md5) {
            DHeader dh = {
                "LoginRes", {
                    { "code", 2 },
                    { "session", "" },
                    { "message", "密码错误" }
                }
            };

            logger("User %s invalid password", username.c_str());

            if(dh.send(cfd) != 0)
                rcode = -1;
            else rcode =  0;

            logger("Closing the socket");
            close(cfd);
        }
    else {
        string session = header["session"].get<string>();
        int uid = getUidBySession(session);
        if(session.empty() || uid < 0) {
            session = newClient(username);
        }
        DHeader dh = {
            "LoginRes", {
                { "code", 0 },
                { "session", session } ,
                { "message", "登陆成功" } 
            }
        };

        logger("User %s login with session %s", username.c_str(), session.c_str());
        if(dh.send(cfd) != 0)
            rcode = -1;
        else rcode = 0;

        logger("Closing the socket");
        close(cfd);
    }
    mysql_free_result(result);

    return rcode;
}

int signup(json header) {
    logger("Received Signup request");

    string username = header["username"].get<string>();
    string passwd = header["password"].get<string>();
    string session;

    char query[255];
    snprintf(query, sizeof(query), 
        "select username, passwd from user where username=\"%s\""
        , username.c_str());

    if (mysql_query(db, query)) 
    {
        finish_with_error(db);  
        return -1;
    }

    MYSQL_RES *result = mysql_store_result(db);
    
    if (result == NULL) 
    {
        finish_with_error(db);
    }

    DHeader dh;
    bool ok;
    int rcode;
    MYSQL_ROW row;
    if(!(row = mysql_fetch_row(result))) {
        /* user not found */
        /* short password */
        if(passwd.size() < 6) {
            ok = false;
            dh = {
              "SignupRes", {
                    {"code", 1},
                    {"session", ""},
                    {"message", "密码太短" }
                }
            };
        }
        else if(passwd.size() >= 56) {
            ok = false;
            dh = {
              "SignupRes", {
                    {"code", 2},
                    {"session", ""},
                    {"message", "密码太长" }
                }
            };
        }
        else if(username.size() >= 56) {
            ok = false;
            dh = {
              "SignupRes", {
                    {"code", 2},
                    {"session", ""},
                    {"message", "用户名太长" }
                }
            };
        }
        else {
            ok = true;
            session = newUser(username, passwd);
            dh = {
              "SignupRes", {
                    {"code", 0},
                    {"session", session},
                    {"message", "新用户注册成功" }
                }
            };
        }
        
    }
    else {
        /* User exists */
        ok = false;
        dh = {
            "SignupRes", {
                { "code", 3 },
                { "session", "" },
                { "message", "用户名已存在" }
            }
        };
    }

    if(ok)
        logger("New User %s with session %s", username.c_str(), session.c_str());
    else 
        logger("Fail to signup user %s with passsword %s", username.c_str(), passwd.c_str());

    if(dh.send(cfd) != 0) 
        rcode = -1;
    else rcode = 0;
    logger("Closing the socket");
    close(cfd);

    mysql_free_result(result);

    return rcode;
}

const int MaxPushBytes = 1024 * 1024; // 100 KB

// TODO: test large file split
void chunkify(json pull, vector<json> &pushes) {
    int i = 0, n = pull.size(), bytes = 0;
    json push;
    while(i < n) {
        if(pull[i]["len"].get<int>() + bytes >= MaxPushBytes) {
            /* merge 1 */
            if(bytes != 0) {
                pushes.push_back(push);
                bytes = 0;
                push.clear();
            }
            else {
                /* bytes == 0, split */
				int offset = pull[i]["offset"];
                while(bytes < pull[i]["len"]) {
					int t = pull[i]["len"].get<int>();
                    int n = MaxPushBytes < t - bytes ? MaxPushBytes : t - bytes;
					json single = pull[i];
					single["offset"] = offset;
					single["len"] = n;
                    json arr;
                    arr.push_back(single);
                    pushes.push_back(arr);
					offset += n; bytes += n;
                }
                bytes = 0;
                push.clear();
                i++;
            }
        }
        else { /* merge 2 */
            bytes += pull[i]["len"].get<int>();
            push.push_back(pull[i]);
            i++;
        }
    }

    if(!push.empty()) {
        pushes.push_back(push);
    }
}

int senderInit(const string &session) {

    srvrole = "Sender";

    int uid = getUidBySession(session);
    logger("session %s uid %d", session.c_str(), uid);
    if(setCommitidNull(session)) {
        logger("set commitid null for session %s failed", session.c_str());
    }

    /* send dirinfo */
    json dirinfo = scanUserFile(uid);
    string msg = "DirInfo\n";
    msg += dirinfo.empty() ? "[]" : dirinfo.dump();

    logger("Sent DirInfo %s", msg.c_str());

    int n = sendn(cfd, msg.c_str(), msg.size()+1);
    if(n != 0) {
        logger("Send Dirinfo error");
        return -1;
    }

    /* wait for pull */
    
    while(1) {
        /* 1. C --> PullReq --> S
         * 2. C <-- PushReq <-- S
         * 3. back to 1
         */ 
        
        DHeader dh;
        shared_ptr<Readbuf_> readbuf = shared_ptr<Readbuf_>(new Readbuf_(cfd));
        logger("Parsing DHeader");
        if((n = dh.read(readbuf)) != 0) {
            logger("Error code: %d", n);
            return -1;
        }

        if(dh.type != "Pull") {
            logger("Expected Pull but received %s", dh.type.c_str());
            return -1;
        }

        logger("Receive Pull %s", dh.header.dump().c_str());

        vector<json> pushes;
        /* merge small chunks & split large chunks into multiple PushReq */
        logger("Start chunkify");
        chunkify(dh.header, pushes);
        logger("Chunkify end");

        //logger("Prepared chunks, waiting to be sent");

        for(auto &pushReq: pushes) {
            string msg = "Push\n";
            msg += pushReq.dump();

            logger("Pushreq %s", msg.c_str());

            if((n = sendn(cfd, msg.c_str(), msg.size()+1)) < 0)
                return -1;

            logger("Sent header");
            
            for(auto &e: pushReq) {
                string md5 = getMD5ByUid(uid, e["filename"], e["path"]);
                int ffd = open((StorePath + "/" + md5).c_str(), O_RDONLY);
                off_t offset_ = e["offset"].get<int>();
                n = sendfile(cfd, ffd, &offset_, e["len"].get<int>());
                if(n <= 0) {
                    return -1;
                }
                logger("Sent %d bytes for file %s", n, e["filename"].get<string>().c_str());
            }
        }

        string msg = "done\n";

        sendn(cfd, msg.c_str(), msg.size());

        /* update commitid */
        updateCommitid(uid, session);
    }
}

int senderIncr(const string &session) {
    int uid = getUidBySession(session);
    int commitid = getCommitidBySession(session);

    #define TIMEOUT 5
    /* wait timeout to check for new commit */
    while(1) { //select * from journal where uid = 1 and commitid > 1

        sleep(TIMEOUT);
        json ncommits = std::move(selectNewCommits(uid, commitid));
        if(ncommits.empty()) continue;

        /* C --> DirInfo --> S
         * S <-- PullReq <-- C
         * S --> PushReq --> C
         */
        string msg = "DirInfo\n";
        msg += ncommits.dump();

        int n = sendn(cfd, msg.c_str(), msg.size()+1);
        if(n != 0) {
            logger("Send Dirinfo error");
            return -1;
        }

        while(1) {
            DHeader dh;
            shared_ptr<Readbuf_> readbuf = shared_ptr<Readbuf_>(new Readbuf_(cfd));
            if((n = dh.read(readbuf)) != 0) {
                logger("Error code: %d", n);
                return -1;
            }
            if(dh.type != "PullReq") {
                logger("Expected PullReq but received %s", dh.type.c_str());
                return -1;
            }

            vector<json> pushes;
            /* merge small chunks & split large chunks into multiple PushReq */
            chunkify(ncommits, pushes);

            for(auto &pushReq: pushes) {
                string msg = "PushReq\n";
                msg += pushReq.dump();

                if((n = sendn(cfd, msg.c_str(), msg.size()+1)) < 0)
                    return -1;

                for(auto &e: pushReq) {
                    string md5 = getMD5ByUid(uid, e["filename"], e["path"]);
                    int ffd = open((StorePath + md5).c_str(), O_RDONLY);
                    off_t offset_ = e["offset"].get<int>();
                    n = sendfile(cfd, ffd, &offset_, e["len"].get<int>());
                    if(n <= 0) {
                        return -1;
                    }
                }
            }
        }


    }
}

int sender(json header) {

    logger("As sender");

    int mode = header["mode"];
    string session = header["session"];

    int n;

    if(mode == 0)
        logger("Init sync");
    else if(mode == 1)
        logger("Incr sync");
    else if(mode == 2)
        logger("Recv sync");

    /* Init or Recv Mode */
    if(mode == 0 || mode == 2) 
        senderInit(session);
    else 
        senderIncr(session);
}

/* write to ./data/chunks for (offset, len) with full size as totlen */
int writeToChunks(const char *buf, const string &md5, int offset, int len, int totlen) {
    /* 1. file exist / or not */

    /* Directly put into Store Dir */

    //string path = ChunksStorePath + "/";
    string path = StorePath + "/";
    path += md5;
    int fd;
    if( access(path.c_str(), F_OK) == -1 ) {
        /* file does not exist then create and preallocate*/
        logger("Create new file %s prealloc %d bytes", path.c_str(), totlen);
        if((fd = open(path.c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR)) < 0) {
            if((fd = creat(path.c_str(), O_WRONLY) < 0))
                errExit("Can't create %s, exiting..", md5.c_str());
        }
        if(fallocate(fd, 0, 0, totlen) < 0) {
            logger("Prealloc failed, exiting...");
            exit(0);
        }
    }
    else {
        fd = open(path.c_str(), O_WRONLY | O_CREAT);
    }

    if(lseek(fd, offset, SEEK_SET) < 0) {
        logger("lseek failed, exiting...");
        exit(0);
    }     

    if(write(fd, buf, len) < 0) {
        logger("wrote failed, exiting...");
        exit(0);
    }
}

/* if full file received return 1 if not 0 */
int insertIntoChunks(json &chunks, int offset, int len, int totlen) {
    logger("Before Chunks: %s", chunks.dump(4).c_str());

    vector<pair<int,int> > vc;
    for(auto &chunk: chunks) {
        vc.push_back({ chunk["offset"].get<int>(), chunk["len"].get<int>() });
    }
    vc.push_back({ offset, len });
    sort(vc.begin(), vc.end());

    vector<pair<int, int> > vc2;
    int i = 1, head = vc[0].first, tail = vc[0].first + vc[0].second;
    while(1) {
        while(i < vc.size() && vc[i].first <= tail) {
            tail = vc[i].first + vc[i].second;
            i++;
        }
        vc2.push_back({ head, tail });
        if(i < vc.size()) {
            head = vc[i].first;
            tail = vc[i].second;
        }
        else break;
    }

    chunks.clear();
    for(auto &e: vc2) {
        chunks.push_back({
            { "offset", e.first },
            { "len", e.second }
        });
    }

    if(vc2.size() == 1 && vc2[0].first == 0 && vc2[0].second == totlen)
        return true;
    else 
        return false;
}

int moveToStore(const string &md5) {
    /* Do nothing for now chunks are in Store Dir */
}

int receiver(json header) {

    srvrole = "Receiver";
    logger("As receiver");

    int mode =  header["mode"];
    int uid = getUidBySession(header["session"]);

    if(uid < 0) {
        logger("Uid %s not valid, exiting...", header["session"].get<string>().c_str());
        exit(0);
    }

    csession = header["session"];
    if(userOnReceiving(uid)) {
        logger("Already in receiving status, quit. ");
        close(cfd);
        return -1;
    }

    if(mode == 1 && !chkCommitidNull(uid, csession)) {
        logger("Forbid Incr sync when sync commidid = NULL");
        exit(0);
    }

    setUserOnReceiving(header["session"], 1);
    
//try {

    logger("About to receive Dirinfo");

    /* receive DirInfo */
    DHeader dh;
    shared_ptr<Readbuf_> readbuf(new Readbuf_(cfd));
    dh.read(readbuf);

    if(dh.type != "DirInfo") {
        logger("Expect DirInfo but receive %s", dh.type.c_str());
        setUserOnReceiving(header["session"], 0);
        return -1;
    }

    logger("receive DirInfo done");
    /* generate topull */
    json topull;
    for(auto &e: dh.header) {
        bool samename = chkFilePath(uid, e["filename"], e["path"]);

        logger("chkFilePath");

        if(!samename) {
            logger("%s is a new file", e["filename"].get<string>().c_str());
            topull.push_back(e);
        } else {
            logger("%s exist!", e["filename"].get<string>().c_str());
            json file_loc = std::move(getFileScheme(uid, e["filename"], e["path"]));

            logger("getFileScheme %s", file_loc.dump(4).c_str());

            /* Incr / Recv sync */
            if(e["modtime"].get<string>() > file_loc["modtime"].get<string>() && e["md5"] != file_loc["md5"]) {
                logger("Client file newer than server");
                /* always pull */
                topull.push_back(e);
                /* for init mode, modify local file to .old */
                if(mode == 0 && e["md5"] != file_loc["md5"]) {
                    fileRename(uid, e["filename"], e["path"], e["filename"].get<string>() + ".old");
                    logger("Renaming %s to %s", e["filename"].get<string>().c_str(), (e["filename"].get<string>() + ".old").c_str());
                    e["filename"] = e["filename"].get<string>() + ".old";
                    //journalCommit(ADD, uid, e);
                }
            }
            else {
                if(e["modtime"] == file_loc["modtime"] && !file_loc["complete"].get<int>()) {
                    topull.push_back(e);
                }
                else 
                    logger("Avoid sync over older file %s from client", e["filename"].get<string>().c_str());
                // Client guarante avoidance of this branch 
            }
        }
    }

    
    if(topull.empty()) {
        logger("Nothing to pull, closing socket...");
        string msg = "Pull\n[]";
        int n = sendn(cfd, msg.c_str(), msg.size()+1);
        if(n != 0) {
            logger("Send Pullreq error");
            return -1;
        }
        setUserOnReceiving(header["session"], 0);
        exit(0);
    }
    logger("Generating pullreq");

    /* check for inode & chunks to generate PullReq */
    json pullreq;
    map<pair<string, string>, json> fmap;
    for(auto &file: topull) {
        bool fileInodeExist = chkFileInodeExist(file["md5"]);

        file["uid"] = uid;

        logger("chkFileInodeExist");

        if(fileInodeExist) {
            incRefCount(file["md5"]);
            json nfile = file;
            nfile["complete"] = 1;
            addFile(nfile);
            journalCommit(ADD, nfile);
        }
        else {
            /* check if pull is to resume */
            bool fileChunksExist = chkFilePath(uid, file["filename"], file["path"]);

            logger("chkFilePath");

            if(!fileChunksExist) {
                json nfile = file;
                nfile["complete"] = 0; /* nfile["chunks"] = NULL */
                logger("About to add file %s", file.dump((4)).c_str());
                addFile(nfile);

                logger("Add file to table Files");
            }
            else {
                json file_loc = std::move(getFileScheme(uid, file["filename"], file["path"]));
                if(file_loc["md5"] != file["md5"]) {
                    logger("replace uncomplete file %s/%s", file_loc["path"].get<string>().c_str(), file_loc["filename"].get<string>().c_str());
                    removeFile(uid, file["filename"], file["path"]);
                    json nfile = file;
                    nfile["complete"] = 0;
                    /* These chunks are deprecated */
                    nfile.erase("chunks");
                    addFile(nfile);
                }
            }

            /* add to Pullreq accord to chunks */
            json file_loc = std::move(getFileScheme(uid, file["filename"], file["path"]));

            logger("file_loc = %s", file_loc.dump(4).c_str());

            json chunks = file_loc["chunks"];
            int offset = 0, len = file_loc["len"], i = 0;

            /* TODO: fix chunks.size() */
            logger("len=%d, chunks.size()=%d, chunnks=%s", len, chunks.size(), chunks.dump().c_str());

            while(offset < len && i < chunks.size()) {
                if(i < chunks.size() && offset < chunks[i]["offset"].get<int>()) {
                    pullreq.push_back({
                        {"filename", file_loc["filename"]},
                        {"path", file_loc["path"]},
                        {"offset", offset},
                        {"len", chunks[i]["offset"].get<int>() - offset}
                    });
                }                
                offset = chunks[i]["offset"].get<int>() + chunks[i]["len"].get<int>();
                i++;
            }
            if(offset < len) {
                pullreq.push_back({
                    {"filename", file_loc["filename"]},
                    {"path", file_loc["path"]},
                    {"offset", offset},
                    {"len", len - offset}
                });
            }
            fmap[{file["filename"].get<string>(), file["path"].get<string>()}] = std::move(file_loc);
        }
    }

    if(pullreq.empty()) {
        string msg = "Pull\n[]";
        int n = sendn(cfd, msg.c_str(), msg.size()+1);
        if(n != 0) {
            logger("Send Pullreq error");
            return -1;
        }
        setUserOnReceiving(header["session"], 0);
        exit(0);
    }

    string msg = "Pull\n" + pullreq.dump();
    int n = sendn(cfd, msg.c_str(), msg.size()+1);
    if(n != 0) {
        logger("Send Pullreq error");
        return -1;
    }

    logger("Pull dump %s", msg.c_str());
    logger("Pulling...");
    /* fmap, map from (file,path) to that in Table files
     */

    /* Receive PushReq */
    while(1) {
        dh.read(readbuf);
        logger("here");
        if(dh.type != "Push") {
            logger("Expect Push but receive %s", dh.type.c_str());
            fatal("Exiting process %d...", getpid());
            setUserOnReceiving(header["session"], 0);
            exit(0);
        }

        logger("Header %s", dh.header.dump().c_str());
        #define BUFN 1024

        int buflen = BUFN;
        char *buf = (char*)malloc(BUFN);

        for(auto &push:dh.header) {
            int offset = push["offset"];
            int len = push["len"];
            if(len > buflen) {
                buf = (char*)realloc(buf, len+1);
                buflen = len;
            }
            
            /* Need to judge if md5 doesn not exists */
            auto fh_ = fmap.find({push["filename"].get<string>(), push["path"].get<string>()});
            if(fh_ == fmap.end()) {
                logger("Receive md5 does not match that from DirInfo");
                logger("Exiting...");
                exit(0);
            }
            auto &fh = fh_->second;
            string md5 = fh["md5"];
            int n;

            if((n = readbuf->readn(buf, len)) != 0) {
                logger("Error when reading Push");
                setUserOnReceiving(header["session"], 0);
                exit(0);
            }
            buf[len] = 0;
            // logger("Receive %s", buf);

            /* write to ./data/chunks file */
            writeToChunks(buf, md5, offset, len, fh["len"]);

            logger("wrote to chunks");
            json chunks = fh["chunks"];
            /* modify chunks */
            bool full = insertIntoChunks(chunks, offset, len, fh["len"]);
            if(full) {
                /* move file from ./data/chunks to ./data/store */
                logger("Receive Full file %s", fh["filename"].get<string>().c_str());
                if(validateFile(md5, fh["len"])) {
                    logger("File %s md5 not valid!", fh["filename"].get<string>().c_str());
                }
                moveToStore(md5);
                addInodeFile(md5);
                completeFile(uid, md5);
                journalCommit(ADD, fh); 
            }
            else {
                logger("Chunks: %s", chunks.dump(4).c_str());
                updateChunks(uid, md5, chunks);
                fh["chunks"] = chunks;
            }
        }

        free(buf);
    }
//}
//catch(exception e) { 
//    setUserOnReceiving(header["session"], 0); 
//    throw e;
//}
    setUserOnReceiving(header["session"], 0);
}

void onexit() {
    logger("Cleaning!");
    try {
        close(cfd);
        setUserOnReceiving(csession, 0);
        mysql_close(db);
    } catch(exception e) {}
}

void client(int fd) {
    srand(time(NULL));
    cfd = fd;
    logger("In child!");

    db = connect_db();

    atexit(onexit);

    auto exitRes = [&]() {
        exit(0);
    };

    DHeader dh;
    readbuf = shared_ptr<Readbuf_>(new Readbuf_(fd));
    
    int n;
    if((n = dh.read(readbuf)) != 0) {
        logger("Error code: %d", n);
        exitRes();      
        return;
    }

    logger("Type: %s, size: %d", dh.type.c_str(), dh.type.size());
    logger("Header: %s", dh.header.dump(4).c_str());

    if(dh.type == "Login")
        n = login(dh.header);
    else if(dh.type == "Signup") 
        n = signup(dh.header);
    else if(dh.type == "Tunnel") {
        json &header = dh.header;

        if(header.find("session") == header.end() || header["session"].empty()) {
            logger("Tunnel without session!");            
            exitRes();
        }

        if(dh.header["role"].get<int>() == 0)
            receiver(header);
        else {
            logger("header[\"role\"]=%d", dh.header["role"].get<int>());
            sender(header);
        }
    }
    exitRes();
}

#define DEFAULT_PORT 2222

void chldext_handler(int __useless) {
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

int main(int argc, char** argv)
{
    signal(SIGCHLD,chldext_handler); 

    if (mysql_library_init(0, NULL, NULL)) {
        fprintf(stderr, "could not initialize MySQL library\n");
        exit(1);
    }

    int port;
    if(argc == 1) port = DEFAULT_PORT;
    else port = atoi(argv[1]);
    
    SA4 addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    int fd;
    if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }

    int optval = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
            errExit("setsockopt");

    if(bind(fd, (const SA*) &addr, sizeof(addr)) < 0) {
        return -1;
    }

    #define BACKLOG 100
    listen(fd, BACKLOG);

    SA4 cliaddr;
    socklen_t len;

    char buf[300];
    int n;

    char ipbuf[20];
    while(1) {
        int clifd = accept(fd, (SA*)&cliaddr, &len);
        //client(clifd);
        
        switch(fork()) {
            case 0:
                close(fd);
                client(clifd);
                return 0;
            case -1:
                break;
            default:
                logger("in_addr:%d\n", cliaddr.sin_addr);
                logger("New client from %s:%d\n", inet_ntop(AF_INET, &cliaddr.sin_addr, ipbuf, sizeof(ipbuf)), port);
                break;
        }
        
    }

     mysql_library_end();
     return 0;
}