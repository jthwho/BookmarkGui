/*****************************************************************************
 * bookmarkitem.h
 * May 15, 2015
 *
 *****************************************************************************/

#ifndef _BOOKMARKITEM_H_
#define _BOOKMARKITEM_H_

#include <QUuid>
#include <QString>
#include <QList>

class BookmarkItem {
        public:
                enum Column {
                        // Columns w/ negative numbers don't show up
                        ColumnTimeStamp         = -2,
                        ColumnUUID              = -1,
                        ColumnName              = 0,
                        ColumnURL               = 1
                };

                static int columnCount();
                static QString columnName(int column);

                BookmarkItem(BookmarkItem *parent = NULL, const QUuid &uuid = QUuid());
                virtual ~BookmarkItem();

                BookmarkItem *parent() const;
                qint64 timeStamp() const;
                QUuid uuid() const;
                QString name() const;
                QString url() const;
                int indexInParent() const;
                bool isFolder() const;
                bool mayBeFolder() const;

                bool hasChildren() const;
                void clearChildren();
                BookmarkItem *findChildByUUID(const QUuid &val) const;
                BookmarkItem *findNodeByUUID(const QUuid &val) const;
                const QList<BookmarkItem *> &children() const;
                bool isContainedBy(BookmarkItem *val) const;

                void setHoldTimeStamp(bool value);
                void setTimeStamp(qint64 value);
                void setParent(BookmarkItem *item, int index = -1);
                void setUUID(const QUuid &uuid);
                void setName(const QString &name);
                void setURL(const QString &url);


        protected:
                void updateTimeStamp();
                void removeChild(BookmarkItem *item);
                void addChild(BookmarkItem *item, int index);

                virtual void dataChanged(BookmarkItem *item, int column);
                virtual void aboutToRemoveChildren(BookmarkItem *parent, int startIndex, int endIndex);
                virtual void childrenRemoved(BookmarkItem *parent, int startIndex, int endIndex);
                virtual void aboutToAddChildren(BookmarkItem *parent, int startIndex, int endIndex);
                virtual void childrenAdded(BookmarkItem *parent, int startIndex, int endIndex);

        private:
                BookmarkItem            *_parent;
                QUuid                   _uuid;
                QString                 _name;
                QString                 _url;
                QList<BookmarkItem *>   _children;
                qint64                  _timestamp;
                bool                    _holdTimeStamp;
};

inline BookmarkItem *BookmarkItem::parent() const {
        return _parent;
}

inline qint64 BookmarkItem::timeStamp() const {
        return _timestamp;
}

inline QUuid BookmarkItem::uuid() const {
        return _uuid;
}

inline QString BookmarkItem::name() const {
        return _name;
}

inline QString BookmarkItem::url() const {
        return _url;
}

inline bool BookmarkItem::isFolder() const {
        return hasChildren() && _url.isEmpty();
}

inline bool BookmarkItem::mayBeFolder() const {
        return _url.isEmpty();
}

inline bool BookmarkItem::hasChildren() const {
        return _children.size() > 0;
}

inline const QList<BookmarkItem *> &BookmarkItem::children() const {
        return _children;
}

inline int BookmarkItem::indexInParent() const {
        return _parent == NULL ? -1 : _parent->_children.indexOf((BookmarkItem *)this);
}

inline BookmarkItem *BookmarkItem::findChildByUUID(const QUuid &val) const {
        for(int i = 0; i < _children.size(); i++) {
                BookmarkItem *item = _children.at(i);
                if(item->uuid() == val) return item;
        }
        return NULL;
}

inline BookmarkItem *BookmarkItem::findNodeByUUID(const QUuid &val) const {
        if(_uuid == val) return (BookmarkItem *)this;
        for(int i = 0; i < _children.size(); i++) {
                BookmarkItem *item = _children.at(i)->findNodeByUUID(val);
                if(item != NULL) return item;
        }
        return NULL;
}

inline bool BookmarkItem::isContainedBy(BookmarkItem *val) const {
        if(val == NULL) return false;
        const BookmarkItem *item = this;
        while(item != NULL) {
                if(item == val) return true;
                item = item->parent();
        }
        return false;
}

#endif /* ifndef _BOOKMARKITEM_H_ */


