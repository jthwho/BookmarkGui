/*****************************************************************************
 * faviconstore.h
 * May 20, 2015
 *
 *****************************************************************************/

#ifndef _FAVICONSTORE_H_
#define _FAVICONSTORE_H_

#include <QObject>
#include <QHash>
#include <QList>
#include <QUuid>
#include <QUrl>
#include <QIcon>

class QWebPage;
class QTimer;
class FaviconFetch;

class FaviconStore : public QObject {
        Q_OBJECT
        public:
                static QIcon defaultIcon();
                static QIcon folderIcon();

                FaviconStore(QObject *parent = NULL);
                ~FaviconStore();

                QString iconPath() const;
                QIcon cachedIcon(const QUuid &id) const;
                void clearCache(const QUuid &id, bool clearFile = false);

        public slots:
                void setIconPath(const QString &val);
                void lookup(const QUuid &uuid, const QUrl &url);

        signals:
                void lookupDone(const QUuid &uuid, const QUrl &url, const QIcon &icon);

        private slots:
                void fetchDone(const QUuid &uuid, const QUrl &url, const QIcon &icon);

        private:
                FaviconFetch                    *_fetch;
                QHash<QUuid, QIcon>             _cache;
                QString                         _iconPath;
                QList<QUuid>                    _inProgress;

                QString iconFile(const QUuid &uuid) const;
                void saveIcon(const QUuid &uuid, const QIcon &icon) const;
                QIcon loadIcon(const QUuid &uuid) const;
};

inline QString FaviconStore::iconPath() const {
        return _iconPath;
}

inline QIcon FaviconStore::cachedIcon(const QUuid &val) const {
        return _cache[val];
}


#endif /* ifndef _FAVICONSTORE_H_ */


