/*****************************************************************************
 * mainobject.cpp
 * May 26, 2015
 *
 *****************************************************************************/

#include <QSystemTrayIcon>
#include <QIcon>
#include <QTimer>
#include <QModelIndex>
#include <QUrl>
#include <QProcess>
#include <QDir>
#include "mainobject.h"
#include "bookmarkwindow.h"
#include "menuview.h"
#include "bookmarkrootitem.h"
#include "bookmarksyncer.h"
#include "xml.h"
#include "faviconstore.h"
#include "bookmarkitem.h"

#ifdef MODEL_TEST
#include "modeltest.h"
#endif

MainObject::MainObject(QObject *p) :
        QObject(p)
{
        _root = new BookmarkRootItem(this);
#ifdef MODEL_TEST
        new ModelTest(_root, this);
#endif
        QString homePath = QDir::homePath();
        QString iconPath = QString("%1/.envgui/icons").arg(homePath);
        QString bookmarkPath = QString("%1/sync/etc").arg(homePath);
        QDir::root().mkpath(iconPath);
        QDir::root().mkpath(bookmarkPath);
        _root->faviconStore()->setIconPath(iconPath);

        _syncer = new BookmarkSyncer(this);
        _syncer->setBookmarkRoot(_root);
        _syncer->setFileName(QString("%1/bookmarks.xml").arg(bookmarkPath));
        QTimer::singleShot(0, this, SLOT(loadBookmarks()));

        _menuView = new MenuView;
        _menuView->setModel(_root);
        connect(_menuView, SIGNAL(itemClicked(const QModelIndex &)), this, SLOT(bookmarkMenuItemClicked(const QModelIndex &)));

        _trayIcon = new QSystemTrayIcon(this);
        _trayIcon->setIcon(QIcon(":/res/gear.png"));
        _trayIcon->setVisible(true);
        _trayIcon->setContextMenu(_menuView);
        connect(_trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));

        _bookmarkWindow = new BookmarkWindow;
        _bookmarkWindow->setAttribute(Qt::WA_DeleteOnClose, false);
        _bookmarkWindow->setAttribute(Qt::WA_QuitOnClose, false);
        _bookmarkWindow->setBookmarkRoot(_root);
        connect(_bookmarkWindow, SIGNAL(requestOpenURL(const QString &)), this, SLOT(openURL(const QString &)));
}

MainObject::~MainObject() {
        _trayIcon->setContextMenu(NULL);
        delete _menuView;
        delete _bookmarkWindow;
}

void MainObject::trayIconActivated(QSystemTrayIcon::ActivationReason reason) {
        if(reason == QSystemTrayIcon::Trigger) _bookmarkWindow->setVisible(!_bookmarkWindow->isVisible());
        return;
}

void MainObject::loadBookmarks() {
        _syncer->load();
        _syncer->setAllowSave(true);
        return;
}

void MainObject::openURL(const QString &str) {
        QUrl url(str);
        if(!url.isValid()) return;
        if(url.scheme() == "http" || url.scheme() == "https") {
                QProcess::startDetached(QString("%1/env/bin/web %2").arg(QDir::homePath()).arg(url.toString()));
        }
        return;
}

void MainObject::bookmarkMenuItemClicked(const QModelIndex &index) {
        BookmarkItem *item = _root->itemFromIndex(index);
        if(item == NULL) return;
        openURL(item->url());
        return;
}

