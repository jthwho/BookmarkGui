/*****************************************************************************
 * mainobject.h
 * May 26, 2015
 *
 *****************************************************************************/

#ifndef _MAINOBJECT_H_
#define _MAINOBJECT_H_

#include <QObject>
#include <QSystemTrayIcon>

class BookmarkWindow;
class MenuView;
class BookmarkRootItem;
class BookmarkSyncer;
class QModelIndex;

class MainObject : public QObject {
        Q_OBJECT
        public:
                MainObject(QObject *parent = NULL);
                ~MainObject();

        public slots:
                void openURL(const QString &str);

        private slots:
                void loadBookmarks();
                void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
                void bookmarkMenuItemClicked(const QModelIndex &index);

        private:
                QSystemTrayIcon         *_trayIcon;
                BookmarkWindow          *_bookmarkWindow;
                MenuView                *_menuView;
                BookmarkRootItem        *_root;
                BookmarkSyncer          *_syncer;

};

#endif /* ifndef _MAINOBJECT_H_ */


