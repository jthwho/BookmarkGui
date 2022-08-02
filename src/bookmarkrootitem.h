/*****************************************************************************
 * bookmarkrootitem.h
 * May 15, 2015
 *
 *****************************************************************************/

#ifndef _BOOKMARKROOTITEM_H_
#define _BOOKMARKROOTITEM_H_

#include <QAbstractItemModel>
#include "bookmarkitem.h"

class QUuid;
class QUrl;
class QIcon;
class XML;
class XMLNode;
class FaviconStore;

class BookmarkRootItem : public QAbstractItemModel, public BookmarkItem {
        Q_OBJECT
        public:
                BookmarkRootItem(QObject *parent = NULL);
                XML toXML() const;
                void updateFromXML(const XMLNode &node);
                FaviconStore *faviconStore() const;

                BookmarkItem *itemFromIndex(const QModelIndex &index) const;
                QModelIndex indexFromItem(BookmarkItem *item, int column) const;

                int columnCount(const QModelIndex &parent) const;
                QVariant data(const QModelIndex &index, int role) const;
                QModelIndex index(int row, int column, const QModelIndex &parent) const;
                QModelIndex parent(const QModelIndex &index) const;
                int rowCount(const QModelIndex &parent) const;
                QVariant headerData(int section, Qt::Orientation o, int role) const;
                Qt::ItemFlags flags(const QModelIndex &index) const;
                bool setData(const QModelIndex &index, const QVariant &value, int role);
                QStringList mimeTypes() const;
                QMimeData *mimeData(const QModelIndexList &list) const;
                bool dropMimeData(const QMimeData *data, Qt::DropAction, int row, int col, const QModelIndex &parent);

        signals:
                void changed();
                void updatedFromXML();

        protected:
                void dataChanged(BookmarkItem *item, int column);
                void aboutToRemoveChildren(BookmarkItem *parent, int startIndex, int endIndex);
                void childrenRemoved(BookmarkItem *parent, int startIndex, int endIndex);
                void aboutToAddChildren(BookmarkItem *parent, int startIndex, int endIndex);
                void childrenAdded(BookmarkItem *parent, int startIndex, int endIndex);

        private slots:
                void faviconLoaded(const QUuid &uuid, const QUrl &url, const QIcon &icon);

        private:
                FaviconStore            *_faviconStore;
                
                void faviconLookup(const BookmarkItem *item) const;
};

inline FaviconStore *BookmarkRootItem::faviconStore() const {
        return _faviconStore;
}


#endif /* ifndef _BOOKMARKROOTITEM_H_ */


