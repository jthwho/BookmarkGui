/*****************************************************************************
 * bookmarkwindow.h
 * May 19, 2015
 *
 *****************************************************************************/

#ifndef _BOOKMARKWINDOW_H_
#define _BOOKMARKWINDOW_H_

#include <QMainWindow>

class BookmarkTreeView;
class BookmarkRootItem;

class BookmarkWindow : public QMainWindow {
        Q_OBJECT
        public:
                BookmarkWindow(QWidget *parent = NULL);

        public slots:
                void setBookmarkRoot(BookmarkRootItem *root);
                void readState();
                void writeState();

        signals:
                void requestOpenURL(const QString &url);

        private:
                BookmarkRootItem        *_root;
                BookmarkTreeView        *_view;

                void closeEvent(QCloseEvent *event);
};

#endif /* ifndef _BOOKMARKWINDOW_H_ */


