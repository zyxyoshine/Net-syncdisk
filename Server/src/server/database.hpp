#pragma once

#include "mysql/mysql.h"
#include "../common/json.hpp"
#include <string>
#include <memory>
#include <openssl/md5.h>
using namespace std;

extern MYSQL* db;

struct FileScheme {
    int uid;
    string filename;
    int len;
    string path;
    string md5;
    bool complete;
    bool deleted;
    time_t modtime;
    json chunks;
};

#define finish_with_error(con) _finish_with_error(con, __LINE__) 

void _finish_with_error(MYSQL *con, int line)
{
    fprintf(stderr, "Line %d: %s\n", line, mysql_error(con));  
}

bool validateFile(const string &md5, int len) {
    return true;
}

string get_md5(const char *s, int len) {
    unsigned char bin[MD5_DIGEST_LENGTH];
    char res[2*MD5_DIGEST_LENGTH] = {0};
    MD5((unsigned char*) s, len, bin);
    for(int i = 0; i < MD5_DIGEST_LENGTH; i++)
        sprintf(&res[2*i], "%02x", bin[i]);
    return string(res);
}

string get_md5(const string &s) {
    unsigned char bin[MD5_DIGEST_LENGTH];
    char res[2*MD5_DIGEST_LENGTH] = {0};
    MD5((unsigned char*) s.c_str(), s.size(), bin);
    for(int i = 0; i < MD5_DIGEST_LENGTH; i++)
        sprintf(&res[2*i], "%02x", bin[i]);
    return string(res);
}

string queryByStr(const char *query, int col) {
    if (mysql_query(db, query)) 
    {
        finish_with_error(db);  
        return "";
    }

    MYSQL_RES *result = mysql_store_result(db);
    
    if (result == NULL) 
    {
        finish_with_error(db);
        return "";
    }

    int rcode;
    MYSQL_ROW row;
    if(row = mysql_fetch_row(result)) {
        string r = row[col];
        mysql_free_result(result);
        return r;
    }
    else 
        return "";
}

int getUidByName(const string &username) {
    char query[255];
    snprintf(query, sizeof(query), 
        "select uid from user where username='%s'"
        , username.c_str());

    return atoi(queryByStr(query, 0).c_str());
}

int createNewClient(int uid, const string &cid) {
    char query[255];
    snprintf(query, sizeof(query), 
        "insert into client values(%d, '%s', NULL)"
        , uid, cid.c_str());

    if (mysql_query(db, query)) {
        finish_with_error(db);
        return -1;
    }

    logger("Create client (uid, cid)=(%d,%s) in table", uid, cid.c_str());
    return 0;
}

int createNewUser(const string &username, const string &password) {
    string passwd_md5 = get_md5(password);
    char query[255];
    snprintf(query, sizeof(query), 
        "insert into user (username, passwd) values('%s', '%s')"
        , username.c_str(), passwd_md5.c_str());

    if (mysql_query(db, query)) {
        finish_with_error(db);
        return -1;
    }

    logger("Create user %s in table", username.c_str());
    return 0;
}


int setCommitidNull(const string &session)
{
    //update client set commitid=NULL where cid='punqdqyffyescbvgbbdecspiczfjdnsv'
    char query[500];
    snprintf(query, sizeof(query), 
        "update client set commitid=NULL where cid='%s'"
        , session.c_str());
    
    if (mysql_query(db, query)) {
        finish_with_error(db);
        return -1;
    }

    logger("Set session %s commitid NULL", session.c_str());
    
    return 0;
}


int getUidBySession(const string &session) {
    static char query[255];
    snprintf(query, sizeof(query), 
        "select uid from client where cid='%s'"
        , session.c_str());
    
    string s = std::move(queryByStr(query, 0));

    return s.empty() ? -1 : atoi(s.c_str());
}

int getCommitidBySession(const string &session) {
    char query[255];
    snprintf(query, sizeof(query), 
        "select commitid from client where cid='%s'"
        , session.c_str());

    return atoi(queryByStr(query, 0).c_str());
}


string getMD5ByUid(int uid, const string &filename, const string &path) {
    //select md5 from files where uid=1 and filename='v.mp4' and path='/video'
    char query[255];
    snprintf(query, sizeof(query), 
        "select md5 from files where uid=%d and filename='%s' and path='%s'"
        , uid, filename.c_str(), path.c_str());

    return queryByStr(query, 0);
}

json selectNewCommits(int uid, int commitid) {
    /* return {
        "filename": ,
        "path": ,
        "md5": ,
        "len": ,
        "modtime": , 
        "deleted": 
        }
        // select filename, path, md5, len, modtime, deleted from journal where uid = 1 and commitid > 1
    */
    char query[512];
    snprintf(query, sizeof(query), 
        "select filename, path, 'md5', len, modtime from files where uid=%d and complete=1"
        , uid);

    if (mysql_query(db, query)) 
    {
        finish_with_error(db);  
        return -1;
    }

    MYSQL_RES *result = mysql_store_result(db);
    
    if (result == NULL) 
    {
        finish_with_error(db);
        return -1;
    }

    int rcode;
    MYSQL_ROW row;
    json dirInfo;
    while(row = mysql_fetch_row(result)) {
        dirInfo.push_back({
            {"filename", row[0]},
            {"path", row[1]},
            {"md5", row[2]},
            {"len", atoi(row[3])},
            {"modtime", atoi(row[4])}
        });
    }

    mysql_free_result(result);
    return dirInfo;
}

bool userOnReceiving(int uid) {

    return 0;
    char query[255];
    snprintf(query, sizeof(query), 
        "select syncing from user where uid='%d'"
        , uid);

    return queryByStr(query, 0) == "1";
}

int setUserOnReceiving(const string &session, bool set) {

    int uid = getUidBySession(session);
    static char query[256];
    snprintf(query, sizeof(query), 
        "update user set syncing=%d where uid='%d'"
        , set ? 1 : 0, uid);

    if (mysql_query(db, query)) {
        finish_with_error(db);
        return -1;
    }

    logger("%s session %s UserOnReceiving", set ? "Set" : "Unset", session.c_str());
    
    return 0;
}

string newUser(const string &username, const string &password) {
    string cid;
    for(int i = 0; i < 32; i++)
        cid += rand() % 26 + 'a';
    
    /* update Table User */
    int n;
    if((n = createNewUser(username, password))) {
        logger("Create user %s failed", username.c_str());
        return "";
    }
    int uid = getUidByName(username);
    logger("uid %d", uid);
    createNewClient(uid, cid);

    return cid;
}

json scanUserFile(int uid) {
    // select filename, path, 'md5', len, modtime from files where uid=1 and complete=1
    char query[255];
    snprintf(query, sizeof(query), 
        "select filename, path, md5, len, modtime from files where uid=%d and complete=1"
        , uid);

    if (mysql_query(db, query)) 
    {
        finish_with_error(db);  
        return -1;
    }

    MYSQL_RES *result = mysql_store_result(db);
    
    if (result == NULL) 
    {
        finish_with_error(db);
        return -1;
    }

    int rcode;
    MYSQL_ROW row;
    json dirInfo;
    while(row = mysql_fetch_row(result)) {
        dirInfo.push_back({
            {"filename", row[0]},
            {"path", row[1]},
            {"md5", row[2]},
            {"len", atoi(row[3])},
            {"modtime", row[4]}
        });
    }
    mysql_free_result(result);
    return dirInfo;
}

MYSQL* connect_db() {
    MYSQL *con = mysql_init(NULL);
    if (con == NULL)
        errExit("mysql_init() failed");
    if (mysql_real_connect(con, "localhost", "G2652", "G2652", "G2652", 0, NULL, 0) == NULL) 
        finish_with_error(con);
    return con;
} 

bool chkFilePath(int uid, const string &filename, const string &path) {
    char query[355];
    snprintf(query, sizeof(query), 
        "select filename from files where uid=%d and filename='%s' and path='%s'"
        , uid, filename.c_str(), path.c_str());

    if (mysql_query(db, query)) 
    {
        finish_with_error(db);  
        return -1;
    }

    MYSQL_RES *result = mysql_store_result(db);
    
    if (result == NULL) 
    {
        finish_with_error(db);
        return -1;
    }

    int rcode;
    MYSQL_ROW row;
    if(row = mysql_fetch_row(result)) {
        return true;
    }
    else return false;
}

/* FileScheme */
json getFileScheme(int uid, const string &filename, const string &path) {
    char query[512];
    snprintf(query, sizeof(query), 
        "select uid, filename, len, path, md5, complete, modtime, chunks from files where uid=%d and filename='%s' and path='%s'"
        , uid, filename.c_str(), path.c_str());

    if (mysql_query(db, query)) 
    {
        finish_with_error(db);  
        return -1;
    }

    MYSQL_RES *result = mysql_store_result(db);
    
    if (result == NULL) 
    {
        finish_with_error(db);
        return -1;
    }

    int rcode;
    MYSQL_ROW row;
    json file;
    if(row = mysql_fetch_row(result)) {
        file = {
            { "uid", atoi(row[0]) },
            { "filename", row[1] },
            { "len", atoi(row[2]) },
            { "path", row[3] },
            { "md5", row[4] },
            { "complete", atoi(row[5]) },
            { "modtime", row[6] },
            { "chunks", json::parse(row[7] ? row[7] : "[]") }    
        };
    }

    mysql_free_result(result);
    return file;

}

int fileRename(int uid, const string &filename, const string &path, const string &nfilename) {
    // UPDATE `tmy`.`files` SET `filename`='c.mp4' WHERE  `md5`='9e107d9d372bb6626bd81d3542a419d6';
    logger("filename");
    char query[500];
    snprintf(query, sizeof(query), 
        "UPDATE files SET filename='%s' WHERE uid=%d and filename='%s' and path='%s'"
        , nfilename.c_str(), uid, filename.c_str(), path.c_str());

    logger("Query: %s", query);

    if (mysql_query(db, query)) {
        finish_with_error(db);
        return -1;
    }

    logger("uid %d file %s change to %s", uid, filename.c_str(), nfilename.c_str());
    
    return 0;
}

enum JournalAction { ADD, DELETE };
int journalCommit(JournalAction action, json commit) {
    logger("Commit %s", commit.dump(4).c_str());
    char query[500];
    snprintf(query, sizeof(query), 
        "INSERT INTO journal (`uid`, `filename`, `path`, `action`, `md5`, `len`, `modtime`) VALUES (%d, '%s', '%s', '%s', '%s', %d, '%s')"
        , commit["uid"].get<int>()
        , commit["filename"].get<string>().c_str()
        , commit["path"].get<string>().c_str()
        , action == ADD ? "add" : "delete"
        , commit["md5"].get<string>().c_str()
        , commit["len"].get<int>()
        , commit["modtime"].get<string>().c_str()
    );

    if (mysql_query(db, query)) {
        finish_with_error(db);
        return -1;
    }

    logger("SQL OK: %s", query);
    
    return 0;
}

bool chkFileInodeExist(const string &md5) {
    char query[355];
    snprintf(query, sizeof(query), 
        "select refcount from inode where md5='%s'"
        , md5.c_str());

    if (mysql_query(db, query)) 
    {
        finish_with_error(db);  
        return -1;
    }

    MYSQL_RES *result = mysql_store_result(db);
    
    if (result == NULL) 
    {
        finish_with_error(db);
        return -1;
    }

    int rcode;
    MYSQL_ROW row;
    if(row = mysql_fetch_row(result)) {
        return true;
    }
    else return false;
}

int addFile(json file) {

    logger("add file: %s", file.dump(4).c_str());

    static char query[1024];
    snprintf(query, sizeof(query), 
        "INSERT INTO `files` (`uid`, `filename`, `path`, `complete`, `modtime`, `chunks`, len, md5) VALUES (%d, '%s', '%s', %d, '%s', NULL, %d, '%s')"
        , file["uid"].get<int>(), file["filename"].get<string>().c_str(), file["path"].get<string>().c_str(), file["complete"].get<int>(), file["modtime"].get<string>().c_str(), file["len"].get<int>(), file["md5"].get<string>().c_str());

    logger("Query: %s", query);

    if (mysql_query(db, query)) {
        finish_with_error(db);
        return -1;
    }
    
    return 0;
}


int addInodeFile(const string md5) {
    char query[500];
    snprintf(query, sizeof(query), 
        "INSERT INTO inode (md5, refcount) VALUES ('%s', 1)"
        , md5.c_str());

    if (mysql_query(db, query)) {
        finish_with_error(db);
        return -1;
    }

    logger("Add inode file %s", md5.c_str());
    
    return 0;
}

int completeFile(int uid, const string &md5) {
    // UPDATE `tmy`.`files` SET `complete`='0' WHERE  `md5`='9e107d9d372bb6626bd81d3542a419d6';
    char query[500];
    snprintf(query, sizeof(query), 
        "UPDATE files SET complete=1, chunks=NULL WHERE md5='%s'"
        , md5.c_str());

    if (mysql_query(db, query)) {
        finish_with_error(db);
        return -1;
    }

    logger("Add file %s complete!", md5.c_str());
    
    return 0;
}

int updateChunks(int uid, const string &md5, json chunks) {
    char query[500];
    snprintf(query, sizeof(query), 
        "UPDATE files SET chunks='%s' WHERE uid=%d and md5='%s'"
        , chunks.dump().c_str(), uid, md5.c_str());

    if (mysql_query(db, query)) {
        finish_with_error(db);
        return -1;
    }

    logger("Add uid %d md5 %s updated!", uid, md5.c_str());
    
    return 0;
}

int incRefCount(const string &md5) {
    // update inode set refcount = refcount+1 where md5='123qweqewqeqweqweq'
    static char query[256];
    snprintf(query, sizeof(query), 
        "update inode set refcount=refcount+1 where md5='%s'"
        , md5.c_str());

    if (mysql_query(db, query)) {
        finish_with_error(db);
        return -1;
    }

    logger("incrment file %s refcount by 1", md5.c_str());
    
    return 0;
} 

int removeFile(int uid, const string &filename, const string &path) {
    // DELETE FROM `tmy`.`files` WHERE  `md5`='9e107d9d372bb6826bd81d3542a419d6';
    static char query[256];
    snprintf(query, sizeof(query), 
        "DELETE FROM files WHERE filename='%s' and path='%s'"
        , filename.c_str(), path.c_str());

    if (mysql_query(db, query)) {
        finish_with_error(db);
        return -1;
    }

    logger("remove uid %d %s/%s", uid, filename.c_str(), path.c_str());
    
    return 0;
}

bool chkCommitidNull(int uid, const string &cid) {
    char query[355];
    snprintf(query, sizeof(query), 
        "select commitid from client where uid=%d and cid='%s'"
        , uid, cid.c_str());

    if (mysql_query(db, query)) 
    {
        finish_with_error(db);  
        return -1;
    }

    MYSQL_RES *result = mysql_store_result(db);
    
    if (result == NULL) 
    {
        finish_with_error(db);
        return -1;
    }

    int rcode;
    MYSQL_ROW row;
    if(row = mysql_fetch_row(result)) {
        mysql_free_result(result);
        return true;
    }
    else {
        mysql_free_result(result);
        return false;
    }
}

int updateCommitid(int uid, const string &session) {
    static char query[355];
    snprintf(query, sizeof(query), 
        "select max(commitid) from journal where uid=%d"
        , uid);
        
    int commitid = atoi(queryByStr(query, 0).c_str());
    snprintf(query, sizeof(query),
        "UPDATE client SET commitid=%d WHERE cid='%s'"
        , commitid, session.c_str());
    
    if (mysql_query(db, query)) {
        finish_with_error(db);
        return -1;
    }
}