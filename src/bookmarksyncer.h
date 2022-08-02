/*****************************************************************************
 * bookmarksyncer.h
 * May 19, 2015
 *
 *****************************************************************************/

#ifndef _BOOKMARKSYNCER_H_
#define _BOOKMARKSYNCER_H_

#include <QObject>
#include <QDateTime>

class QTimer;
class BookmarkRootItem;

class BookmarkSyncer : public QObject {
        Q_OBJECT
        public:
                BookmarkSyncer(QObject *parent = NULL);
                ~BookmarkSyncer();

                bool isCurrent() const;

        public slots:
                void setFileName(const QString &filename);
                void load(bool refresh = false);
                void save();
                void setBookmarkRoot(BookmarkRootItem *value);
                void setAllowSave(bool val);

        private slots:
                void checkIfCurrent();
                void rootDestroyed(QObject *);
                void saveData();
                void updateLastState();

        private:
                BookmarkRootItem        *_root;
                QTimer                  *_timer;
                bool                    _pendingSave;
                bool                    _loading;
                bool                    _allowSave;
                QString                 _fileName;
                QDateTime               _lastModified;
                qint64                  _lastSize;

};

#endif /* ifndef _BOOKMARKSYNCER_H_ */

