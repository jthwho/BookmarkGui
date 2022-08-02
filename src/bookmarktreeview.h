/*****************************************************************************
 * bookmarktreeview.h
 * May 15, 2015
 *
 *****************************************************************************/

#ifndef _BOOKMARKTREEVIEW_H_
#define _BOOKMARKTREEVIEW_H_

#include <QTreeView>

class QUuid;
//class QStringList;
class BookmarkItem;
class BookmarkRootItem;

class BookmarkTreeView : public QTreeView {
        Q_OBJECT
        public:
                BookmarkTreeView(QWidget *parent = NULL);

                BookmarkRootItem *rootItem() const;
                QByteArray saveState();
                bool restoreState(const QByteArray &state);

        signals:
                void requestOpenURL(const QString &url);

        protected:
                //void startDrag(Qt::DropActions supportedActions);
                //void dropEvent(QDropEvent *event);

        private slots:
                void showContextMenu(const QPoint &pt);

         private:
                BookmarkItem *addItem(const QModelIndex &parent);
                BookmarkItem *pasteItem(const QModelIndex &parent);
                void copyURL(const QModelIndex &index) const;
                void removeItem(const QModelIndex &index, bool force);
                void addToExpandList(QStringList &list, const BookmarkItem *item);
                void setExpandFromList(const QList<QUuid> &list, const BookmarkItem *item);
};

#endif /* ifndef _BOOKMARKTREEVIEW_H_ */


