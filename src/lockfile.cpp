/*****************************************************************************
 * lockfile.cpp
 * May 29, 2015
 *
 *****************************************************************************/

#include "lockfile.h"
#include <QSharedMemory>
#include <QDateTime>
#include <QCoreApplication>
#include <QFileInfo>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#define DATA_SIZE       4096
#define LOCK_TRIES      5
#define LOCK_DELAY      10000
#define MAGIC           0x4c4f434b46494c45ull   // LOCKFILE

typedef struct {
        qint64          pid;
        qint64          time;
        char            name[1024];
        quint64         magic;
} LockData;

static QByteArray processName(qint64 pid) {
        QString fn = QString("/proc/%1/exe").arg(pid);
        QFileInfo info(fn);
        if(!info.isSymLink()) return QByteArray();
        return info.symLinkTarget().toLatin1();
}

static void writeLockData(LockData *data) {
        memset(data, 0, sizeof(*data));
        data->pid = QCoreApplication::applicationPid();
        data->time = QDateTime::currentDateTime().toMSecsSinceEpoch();
        QByteArray name = processName(data->pid);
        if(!name.isEmpty()) qstrncpy(data->name, name.data(), sizeof(data->name));
        data->magic = MAGIC;
        return;
}

static bool isStale(LockData *data) {
        QByteArray pname = processName(data->pid);
        if(pname.isEmpty()) return true;
        //printf("PID %lld is %s\n", data->pid, pname.data());
        //printf("Data is %s\n", data->name);
        return pname != data->name;
}

LockFile::LockFile(QObject *p) :
        QObject(p),
        _shm(NULL)
{

}

LockFile::LockFile(const QString &k, QObject *p) :
        QObject(p),
        _shm(NULL),
        _key(k)
{

}

LockFile::~LockFile() {
        unlock();
}

void LockFile::setKey(const QString &key) {
        _key = key;
        return;
}

bool LockFile::lock() {
        if(isLocked()) return true;
        if(_key.isEmpty()) return false;
        QSharedMemory *s = new QSharedMemory(_key, this);
        for(int i = 0; i < LOCK_TRIES; i++) {
                if(!s->create(DATA_SIZE)) {
                        if(!s->attach()) {
                                usleep(LOCK_DELAY);
                                continue;
                        }
                }
        }
        if(!s->isAttached()) {
                delete s;
                return false;
        }

        bool ret = false;
        LockData *data = (LockData *)s->data();
        s->lock();
        if(data->magic == MAGIC) {
                if(isStale(data)) {
                        writeLockData(data);
                        ret = true;
                }
        } else {
                writeLockData(data);
                ret = true;
        }
        s->unlock();
        if(ret) _shm = s;
        else delete s;
        return ret;
}

void LockFile::unlock() {
        if(!isLocked()) return;
        LockData *data = (LockData *)_shm->data();
        _shm->lock();
        memset(data, 0, sizeof(*data));
        _shm->unlock();
        _shm->detach();
        delete _shm;
        _shm = NULL;
        return;
}

