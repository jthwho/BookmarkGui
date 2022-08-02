/*****************************************************************************
 * lockfile.h
 * May 29, 2015
 *
 *****************************************************************************/

#ifndef _LOCKFILE_H_
#define _LOCKFILE_H_

#include <QObject>

class QSharedMemory;

class LockFile : public QObject {
        Q_OBJECT
        public:
                LockFile(QObject *parent = NULL);
                LockFile(const QString &key, QObject *parent = NULL);
                ~LockFile();

                void setKey(const QString &key);
                bool isLocked() const;
                bool lock();
                void unlock();

        private:
                QSharedMemory   *_shm;
                QString         _key;
};

inline bool LockFile::isLocked() const {
        return _shm != NULL;
}

#endif /* ifndef _LOCKFILE_H_ */


