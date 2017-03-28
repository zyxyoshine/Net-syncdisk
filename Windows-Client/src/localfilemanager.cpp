#include "localfilemanager.h"
#include <QFileInfo>
#include <exception>

#define log(s) TMY::logger.log(s)
#define CHUNKSUPDATEMACCNT 10

TMY::LocalFileWriter::LocalFileWriter(QString syncPath_)
{
    syncPath = syncPath_;
}

TMY::LocalFileReader::LocalFileReader(QString syncPath_)
{
    syncPath = syncPath_;
}

void TMY::LocalFileWriter::setFile(const FilePath filepath_, const int len_)
{
    QFileInfo infoo(metaPath);
    if (infoo.exists())
    {


   std::ofstream metafile(metaPath.toStdString());
        if (metafile.fail())
        {
            log("Writer: metafile not found when writing chunks when changing file");
        }
        else
        {
        json j = chunks.toJSON();
        metafile << j;
        metafile.close();
        log("Writer: chunks wrote");
        }
    }

    filePath = filepath_;
    fulllen = len_;
    path = syncPath;
    path += TMY::convert(filePath);
    log("Writer: set file " + path);
    metaPath = path;
    metaPath += METATAIL;

    std::ifstream imetafile(metaPath.toStdString());
    if (imetafile.fail())
    {
        log("Writer: imetafile not found, location : " + metaPath);
    }
    else
    {
    log("Writer: getting chunks from imetafile..");
    json j;
    try
    {
    imetafile >> j;
    }
    catch (std::exception ex)
    {
        j = json();
    }
    imetafile.close();
    chunks.fromJSON(j);
    chunksWriteCnt = 0;
    log("Writer: got chunks from metafile");
    }
}

void TMY::LocalFileReader::setFilePath(const FilePath filepath_)
{
    filePath = filepath_;
    path = syncPath;
    path += TMY::convert(filePath);
    log("Reader: set file " + path);
}

TMY::PullReq TMY::LocalFileWriter::generatePullReqEntry()
{

    std::ifstream metafile(metaPath.toStdString());
    if (metafile.fail())
    {
        log("Writer: metafile not found, location : " + metaPath);
        return TMY::PullReq();
    }
    log("Writer: getting chunks from metafile..");
    json j;
    try
    {
    metafile >> j;
    }
    catch (std::exception ex)
    {
        j = json();
    }

    chunks.fromJSON(j);

    chunksWriteCnt = 0;
    log("Writer: got chunks from metafile");

    file.setFileName(path);
    file.open(QIODevice::ReadOnly);

    PullReq pullReq;

    pullReq.clear();
    PullReqEntry p;
    p.filePath = filePath;
    p.offset = 0;
    int tail = 0;
    for (int i = 0; i < chunks.size(); ++i)
    {
        tail = chunks[i].offset;
        if (p.offset != tail)
        {
            p.len = tail - p.offset;
            pullReq.push_back(p);
        }
        tail += chunks[i].len;
        p.offset = tail;
    }
    p.len = fulllen - p.offset;
    pullReq.push_back(p);
    log("Writer: pulls generated");

    return pullReq;
}

void TMY::LocalFileWriter::writePush(PushReqEntry &pushReq)
{
    log("Writer: push received");
    file.setFileName(path);
    file.open(QIODevice::Append);
    file.seek(pushReq.offset);
    file.write(pushReq.buffer, pushReq.len);
    file.close();

    delete[] pushReq.buffer;

    int i;
    for (i = 0; i < chunks.size(); ++i)
        if (chunks[i].offset > pushReq.offset)
            break;
    log("Writer: push wrote");
    Chunk chunk;
    chunk.len = pushReq.len;
    chunk.offset = pushReq.offset;
    chunks.insert(chunks.begin() + i, chunk);
    updateChunks();
    emit sendDownMsg("Downloading " + TMY::convert(filePath), (float)(chunks[0].len) / fulllen);
}


void TMY::LocalFileWriter::updateChunks()
{
    log("Writer: updating chunks");
	for (int i = 0; i < chunks.size() - 1; ++i)
	{
		int tail = chunks[i].offset + chunks[i].len;
		int head = chunks[i + 1].offset;
		if (tail == head)
		{
			chunks[i].len += chunks[i+1].len;
			chunks.erase(chunks.begin() + i + 1);
		}
	}
    if (chunks[0].len == fulllen)
	{
        if (QFile::remove(metaPath))
            log("Writer: file finished, metafile removed");
        else
            log("Writer: remove meta file error");
    }
    ++chunksWriteCnt;
    if (chunksWriteCnt > CHUNKSUPDATEMACCNT)
    {
        std::ofstream metafile(metaPath.toStdString());
        if (metafile.fail())
        {
            log("Writer: metafile not found when writing chunks, location : " + metaPath);
            return;
        }
        json j = chunks.toJSON();
        metafile << j;
        metafile.close();
        log("Writer: chunks wrote");
    }
    log("Writer: chunks updated");
}

TMY::PushReq TMY::LocalFileReader::generatePushReqEntry(PullReqEntry entry)
{
    PushReq pushes;
    file.setFileName(path);

    file.open(QIODevice::ReadOnly);
        while (entry.len != 0)
        {
            PushReqEntry push;
            push.filePath = filePath;
            push.offset = entry.offset;
            int len = entry.len > 1024 * 1024 ? 1024 * 1024 : entry.len;
            push.len = len;
            push.buffer = new char[len];
            char asdf[50];
            sprintf(asdf, "%x \n", push.buffer);

            log("Reader: CHECK BUFFER" + QString(asdf));
            log("Reader: generate push  offset=" + QString::number(push.offset) + "  len=" + QString::number(push.len));

            file.seek(entry.offset);
            file.read(push.buffer, len);
            pushes.push_back(push);

            entry.offset += len;
            entry.len -= len;
        }
    return pushes;
}
