/*****************************************************************************
 * mainwindow.cpp
 * May 19, 2015
 *
 *****************************************************************************/

#include <stdio.h>
#include <QTimer>
#include <QSettings>
#include <QModelIndex>
#include <QProcess>

#include "bookmarkwindow.h"
#include "bookmarktreeview.h"
#include "bookmarkrootitem.h"

#define SETTING_GEOMETRY        "%1/Geometry"
#define SETTING_WINDOW_STATE    "%1/State"
#define SETTING_VIEW_STATE      "%1/View"

static QString objectName(const QObject *obj) {
        QString ret;
        while(obj != NULL) {
                const QObject *parent = obj->parent();
                QString name = obj->objectName();
                if(name.isEmpty()) {
                        name = obj->metaObject()->className();
                        if(parent != NULL) name += QString::number(parent->children().indexOf((QObject *)obj));
                }
                if(!ret.isEmpty()) ret += ".";
                ret += name;
                obj = parent;
        }
        return ret;
}

BookmarkWindow::BookmarkWindow(QWidget *p) :
        QMainWindow(p),
        _root(NULL)
{
        setWindowTitle("Bookmarks");
        _view = new BookmarkTreeView(this);
        connect(_view, SIGNAL(requestOpenURL(const QString &)), this, SIGNAL(requestOpenURL(const QString &)));
        setCentralWidget(_view);
        readState();
}

void BookmarkWindow::setBookmarkRoot(BookmarkRootItem *root) {
        if(_root != NULL) {
                disconnect(_root, NULL, this, NULL);
                disconnect(this, NULL, _root, NULL);
        }
        _root = root;
        _view->setModel(root);
        if(_root != NULL) {
                connect(_root, SIGNAL(updatedFromXML()), this, SLOT(readState()));
        }
        readState();
        return;
}

void BookmarkWindow::readState() {
        QString name = ::objectName(this);
        QSettings s;
        restoreGeometry(s.value(QString(SETTING_GEOMETRY).arg(name)).toByteArray());
        restoreState(s.value(QString(SETTING_WINDOW_STATE).arg(name)).toByteArray());
        _view->restoreState(s.value(QString(SETTING_VIEW_STATE).arg(name)).toByteArray());
        return;
}

void BookmarkWindow::writeState() {
        QString name = ::objectName(this);
        QSettings s;
        s.setValue(QString(SETTING_GEOMETRY).arg(name), saveGeometry());
        s.setValue(QString(SETTING_WINDOW_STATE).arg(name), saveState());
        s.setValue(QString(SETTING_VIEW_STATE).arg(name), _view->saveState());
        return;
}

void BookmarkWindow::closeEvent(QCloseEvent *event) {
        writeState();
        QMainWindow::closeEvent(event);
        return;
}


