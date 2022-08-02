/*****************************************************************************
 * faviconstore.cpp
 * May 20, 2015
 *
 *****************************************************************************/

#include <QDebug>
#include <QStringList>
#include <QDateTime>
#include <QFile>
#include <QDataStream>

#include "faviconstore.h"
#include "faviconfetch.h"

#define TIMEOUT         10000

static QString uuidToFileNameString(const QUuid &val) {
        QString ret = val.toString();
        ret.replace("{", "");
        ret.replace("}", "");
        ret.replace("-", "");
        return ret.toLower();
}

QIcon FaviconStore::defaultIcon() {
        static QIcon icon(":/res/default.ico");
        return icon;
}

QIcon FaviconStore::folderIcon() {
        static QIcon icon(":/res/folder.png");
        return icon;
}

FaviconStore::FaviconStore(QObject *p) :
        QObject(p)
{
        _fetch = new FaviconFetch(this);
        connect(_fetch, SIGNAL(fetchDone(const QUuid &, const QUrl &, const QIcon &)),
                this, SLOT(fetchDone(const QUuid &, const QUrl &, const QIcon &)));

}

FaviconStore::~FaviconStore() {
        
}

void FaviconStore::setIconPath(const QString &val) {
        if(val == _iconPath) return;
        _iconPath = val;
        return;
}

void FaviconStore::lookup(const QUuid &uuid, const QUrl &url) {
        if(!url.isValid()) return;
        if(_inProgress.contains(uuid)) return;

        QIcon icon = loadIcon(uuid);
        if(!icon.isNull()) {
                _cache[uuid] = icon;
                emit lookupDone(uuid, url, icon);
                return;
        }

        _inProgress += uuid;
        _fetch->fetch(uuid, url);
        return;
}


void FaviconStore::fetchDone(const QUuid &uuid, const QUrl &url, const QIcon &icon) {
        _inProgress.removeOne(uuid);
        QIcon finalIcon = icon.isNull() ? defaultIcon() : icon;
        _cache[uuid] = finalIcon;
        if(!icon.isNull()) saveIcon(uuid, icon);
        emit lookupDone(uuid, url, finalIcon);
        return;
}

QString FaviconStore::iconFile(const QUuid &uuid) const {
        return QString("%1/%2.favicon").arg(_iconPath).arg(uuidToFileNameString(uuid));
}

void FaviconStore::saveIcon(const QUuid &uuid, const QIcon &icon) const {
        QFile file(iconFile(uuid));
        if(!file.open(QFile::WriteOnly)) return;
        qint32 version = 1;
        QDataStream ds(&file);
        ds << version;
        ds << icon;
        return;
}

QIcon FaviconStore::loadIcon(const QUuid &uuid) const {
        QFile file(iconFile(uuid));
        if(!file.open(QFile::ReadOnly)) return QIcon();
        qint32 version;
        QIcon icon;
        QDataStream ds(&file);
        ds >> version;
        if(version != 1) return QIcon();
        ds >> icon;
        return icon;
}

void FaviconStore::clearCache(const QUuid &val, bool clearFile) {
        _cache.remove(val);
        if(clearFile) QFile::remove(iconFile(val));
        return;
}

