/*****************************************************************************
 * bookmarksyncer.cpp
 * May 19, 2015
 *
 *****************************************************************************/

#include <stdio.h>
#include <QFile>
#include <QFileInfo>
#include <QTimer>
#include "bookmarkrootitem.h"
#include "bookmarksyncer.h"
#include "xml.h"

BookmarkSyncer::BookmarkSyncer(QObject *p) :
        QObject(p),
        _root(NULL),
        _pendingSave(false),
        _loading(false),
        _allowSave(false),
        _lastSize(0)
{
        _timer = new QTimer(this);
        connect(_timer, SIGNAL(timeout()), this, SLOT(checkIfCurrent()));
        _timer->start(2000);
}

BookmarkSyncer::~BookmarkSyncer() {

}

void BookmarkSyncer::setFileName(const QString &val) {
        if(_fileName == val) return;
        _fileName = val;
        save();
        return;
}

void BookmarkSyncer::setAllowSave(bool val) {
        if(val == _allowSave) return;
        _allowSave = val;
        return;
}

void BookmarkSyncer::load(bool refresh) {
        if(_root == NULL) return;
        if(_loading) return;
        _loading = true;
        _pendingSave = false;
        XML xml = XML::fileToXML(_fileName);
        if(xml.isValid()) {
                updateLastState();
                if(!refresh) _root->clearChildren();
                _root->updateFromXML(xml.root());
                if(!isCurrent()) {
                        load(refresh);
                        return;
                }
                printf("Loaded %s\n", qPrintable(_fileName));
        }
        _loading = false;
        return;
}

void BookmarkSyncer::rootDestroyed(QObject *ptr) {
        Q_UNUSED(ptr);
        setBookmarkRoot(NULL);
        return;
}

void BookmarkSyncer::setBookmarkRoot(BookmarkRootItem *value) {
        if(_root == value) return;
        _pendingSave = false;
        _loading = false;
        if(_root != NULL) {
                disconnect(_root, NULL, this, NULL);
                disconnect(this, NULL, _root, NULL);
        }
        _root = value;
        if(_root != NULL) {
                connect(_root, SIGNAL(destroyed(QObject *)), this, SLOT(rootDestroyed(QObject *)));
                connect(_root, SIGNAL(changed()), this, SLOT(save()));
        }
        return;
}

void BookmarkSyncer::save() {
        if(_pendingSave || _loading || _root == NULL) return;
        _pendingSave = true;
        QTimer::singleShot(0, this, SIGNAL(saveData()));
        return;
}

void BookmarkSyncer::saveData() {
        if(!_pendingSave) return;
        if(!_allowSave) {
                QTimer::singleShot(100, this, SLOT(saveData()));
                return;
        }
        _pendingSave = false;
        if(_root == NULL) return;
        XML xml = _root->toXML();
        QFile file(_fileName);
        if(file.open(QFile::WriteOnly)) {
                file.write(xml.root().toString().toLatin1());
        }
        file.close();
        updateLastState();
        printf("Saved %s\n", qPrintable(_fileName));
        return;
}

void BookmarkSyncer::checkIfCurrent() {
        if(_pendingSave || _loading || _root == NULL || _fileName.isEmpty()) return;
        if(!isCurrent()) load(true);
        return;
}

bool BookmarkSyncer::isCurrent() const {
        if(_fileName.isEmpty()) return true;
        QFileInfo info(_fileName);
        if(!info.isFile()) return true;
        return info.size() == _lastSize && 
                info.lastModified() == _lastModified;
}

void BookmarkSyncer::updateLastState() {
        QFileInfo info(_fileName);
        if(info.isFile()) {
                _lastSize = info.size();
                _lastModified = info.lastModified();
        } else {
                _lastSize = -1;
                _lastModified = QDateTime();
        }
        return;
}

