/*****************************************************************************
 * bookmarkrootitem.cpp
 * May 15, 2015
 *
 *****************************************************************************/

#include <stdio.h>
#include <QUrl>
#include <QMimeData>
#include <QStringList>
//#include <QWebSettings>
#include "bookmarkrootitem.h"
#include "xml.h"
#include "faviconstore.h"

#define NODE_BOOKMARK           "bookmark"
#define NODE_BOOKMARKS          "bookmarks"
#define PROP_NAME               "name"
#define PROP_URL                "url"
#define PROP_UUID               "uuid"
#define PROP_TIMESTAMP          "ts"
#define MIMETYPE_UUIDLIST       "application/vnd.uuidlist"

static void itemToXML(XMLNode &node, const BookmarkItem *item, bool withChildren) {
        node.propSet(PROP_NAME, item->name());
        node.propSet(PROP_URL, item->url());
        node.propSet(PROP_UUID, item->uuid().toString());
        node.propSet(PROP_TIMESTAMP, QString::number(item->timeStamp()));

        if(withChildren) {
                const QList<BookmarkItem *> &children = item->children();
                for(int i = 0; i < children.size(); i++) {
                        XMLNode cnode = node.addChild(NODE_BOOKMARK);
                        itemToXML(cnode, children.at(i), true);
                }
        }
        return;
}

static void xmlToItem(const XMLNode &node, BookmarkItem *item, bool withChildren) {
        item->setHoldTimeStamp(true);
        QUuid uuid(node.propGet(PROP_UUID));
        qint64 ts = node.propGet(PROP_TIMESTAMP).toLongLong(); 
        if(uuid != item->uuid() || ts > item->timeStamp()) {
                item->setName(node.propGet(PROP_NAME));
                item->setURL(node.propGet(PROP_URL));
                item->setUUID(uuid);
                item->setTimeStamp(ts);
        }
        if(withChildren) {
                QList<QUuid> list;
                for(XMLNode child = node.firstChild(); child.isValid(); child.gotoNext()) {
                        if(child.name() != NODE_BOOKMARK) continue;
                        QUuid cuuid(child.propGet(PROP_UUID));
                        if(cuuid.isNull()) continue;
                        list += cuuid;
                        BookmarkItem *citem = item->findChildByUUID(cuuid);
                        if(citem == NULL) {
                                citem = new BookmarkItem(item, cuuid);
                                citem->setTimeStamp(0);
                        }
                        xmlToItem(child, citem, true);
                }

                // Remove any items that aren't in the XML
                QList<BookmarkItem *> children = item->children();
                for(int i = 0; i < children.size(); i++) {
                        BookmarkItem *citem = children.at(i);
                        if(!list.contains(citem->uuid())) delete citem;
                }

                // FIXME: Make sure item order is correct
        }
        item->setHoldTimeStamp(false);
        return;
}

BookmarkRootItem::BookmarkRootItem(QObject *p) :
        QAbstractItemModel(p),
        BookmarkItem()
{
        _faviconStore = new FaviconStore(this);
        connect(_faviconStore, SIGNAL(lookupDone(const QUuid &, const QUrl &, const QIcon &)),
                this, SLOT(faviconLoaded(const QUuid &, const QUrl &, const QIcon &)));
        //setSupportedDragActions(Qt::CopyAction | Qt::MoveAction);
}

XML BookmarkRootItem::toXML() const {
        XML xml(NODE_BOOKMARKS);
        XMLNode root = xml.root();
        itemToXML(root, this, true);
        return xml;
}

void BookmarkRootItem::updateFromXML(const XMLNode &node) {
        xmlToItem(node, this, true);
        emit updatedFromXML();
        return;
}

QStringList BookmarkRootItem::mimeTypes() const {
        QStringList ret;
        ret << "text/uri-list";
        ret << MIMETYPE_UUIDLIST;
        return ret;
}

QMimeData *BookmarkRootItem::mimeData(const QModelIndexList &list) const {
        QList<QUrl> urls;
        QByteArray uuids;
        for(int i = 0; i < list.size(); i++) {
                BookmarkItem *item = itemFromIndex(list.at(i));
                if(item == NULL) continue;
                QString url = item->url();
                if(!url.isEmpty()) urls += url;
                if(!uuids.isEmpty()) uuids += " ";
                uuids += item->uuid().toString().toLatin1();
        }
        QMimeData *ret = new QMimeData;
        if(!urls.isEmpty()) ret->setUrls(urls);
        ret->setData(MIMETYPE_UUIDLIST, uuids);
        return ret;
}

bool BookmarkRootItem::dropMimeData(const QMimeData *data, Qt::DropAction, int row, int col, const QModelIndex &parent) {
        printf("Drop %s\n", qPrintable(data->formats().join(",")));
        Q_UNUSED(col);
        BookmarkItem *parentItem = itemFromIndex(parent);
        if(parentItem == NULL) return false;
        if(!parentItem->mayBeFolder()) return false;
        if(data->hasFormat(MIMETYPE_UUIDLIST)) {
                QStringList list = QString(data->data(MIMETYPE_UUIDLIST)).split(" ");
                for(int i = 0; i < list.size(); i++) {
                        BookmarkItem *item = findNodeByUUID(QUuid::fromString(list.at(i)));
                        if(item == NULL) continue;
                        item->setParent(parentItem, row - 1);
                        row++;
                }
        } else {
                QList<QUrl> list = data->urls();
                for(int i = 0; i < list.size(); i++) {
                        BookmarkItem *item = new BookmarkItem;
                        item->setURL(list.at(i).toString());
                        item->setParent(parentItem, row);
                        row++;
                }
        }
        return true;
}

int BookmarkRootItem::columnCount(const QModelIndex &parent) const {
        Q_UNUSED(parent);
        return BookmarkItem::columnCount();
}

BookmarkItem *BookmarkRootItem::itemFromIndex(const QModelIndex &index) const {
        return index.isValid() ?
                static_cast<BookmarkItem *>(index.internalPointer()) :
                (BookmarkItem *)this;
}

QModelIndex BookmarkRootItem::indexFromItem(BookmarkItem *item, int col) const {
        if(item == NULL) return QModelIndex();
        BookmarkItem *p = item->parent();
        if(p == NULL) return QModelIndex();
        int row = p->children().indexOf(item);
        return createIndex(row, col, item);
}

Qt::ItemFlags BookmarkRootItem::flags(const QModelIndex &index) const {
        if(!index.isValid()) return Qt::NoItemFlags;
        BookmarkItem *p = itemFromIndex(index);
        if(p == NULL) return Qt::NoItemFlags;
        Qt::ItemFlags ret = Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
        if(index.column() == ColumnName) {
                ret |= Qt::ItemIsEditable | Qt::ItemIsDropEnabled;
        } else {
                if(!p->isFolder()) ret |= Qt::ItemIsEditable;
        }
        return ret;
}

QVariant BookmarkRootItem::data(const QModelIndex &index, int role) const {
        BookmarkItem *item = itemFromIndex(index);
        if(item == NULL) return QVariant();

        QVariant ret;
        if(role == Qt::DisplayRole || role == Qt::EditRole) {
                switch(index.column()) {
                        case ColumnName: ret = item->name(); break;
                        case ColumnURL: ret = item->url(); break;
                }
        } else if(role == Qt::DecorationRole && index.column() == ColumnName) {
                QIcon icon;
                if(item->isFolder()) {
                        icon = FaviconStore::folderIcon();
                } else {
                        icon = _faviconStore->cachedIcon(item->uuid());
                        if(icon.isNull()) {
                                faviconLookup(item);
                                icon = FaviconStore::defaultIcon();
                        }
                }
                ret = icon;
        }
        return ret;
}

bool BookmarkRootItem::setData(const QModelIndex &index, const QVariant &value, int role) {
        BookmarkItem *item = itemFromIndex(index);
        if(item == NULL) return false;
        bool ret = false;
        if(role == Qt::EditRole) {
                switch(index.column()) {
                        case ColumnName: item->setName(value.toString()); ret = true; break;
                        case ColumnURL: item->setURL(value.toString()); ret = true; break;
                }
        }
        return ret;
}

QModelIndex BookmarkRootItem::index(int row, int column, const QModelIndex &parent) const {
        if(row < 0 || column < 0) return QModelIndex();
        BookmarkItem *parentItem = itemFromIndex(parent);
        if(parentItem == NULL) return QModelIndex();
        BookmarkItem *item = parentItem->children().value(row, NULL);
        return (item == NULL) ? 
                QModelIndex() : 
                createIndex(row, column, item);
}

QModelIndex BookmarkRootItem::parent(const QModelIndex &index) const {
        BookmarkItem *item = itemFromIndex(index);
        if(item == NULL) return QModelIndex();
        return indexFromItem(item->parent(), 0);
}

int BookmarkRootItem::rowCount(const QModelIndex &index) const {
        if(index.column() > 0) return 0;
        BookmarkItem *item = itemFromIndex(index);
        return (item == NULL) ? 0 : item->children().size();
}

QVariant BookmarkRootItem::headerData(int section, Qt::Orientation o, int role) const {
        Q_UNUSED(o);
        QVariant ret;
        if(role == Qt::DisplayRole) ret = BookmarkItem::columnName(section);
        return ret;
}

void BookmarkRootItem::dataChanged(BookmarkItem *item, int column) {
        if(column < 0) return;
        if(column == ColumnURL) {
                _faviconStore->clearCache(item->uuid());
                faviconLookup(item);
        }
        QModelIndex index = indexFromItem(item, column);
        emit QAbstractItemModel::dataChanged(index, index);
        BookmarkItem::dataChanged(item, column);
        emit changed();
        return;
}

void BookmarkRootItem::aboutToRemoveChildren(BookmarkItem *parent, int startIndex, int endIndex) {
        for(int i = startIndex; i <= endIndex; i++) {
                const BookmarkItem *item = parent->children().at(i);
                _faviconStore->clearCache(item->uuid());
        }
        QModelIndex index = indexFromItem(parent, 0);
        beginRemoveRows(index, startIndex, endIndex);
        BookmarkItem::aboutToRemoveChildren(parent, startIndex, endIndex);
        return;
}

void BookmarkRootItem::childrenRemoved(BookmarkItem *parent, int startIndex, int endIndex) {
        endRemoveRows();
        BookmarkItem::childrenRemoved(parent, startIndex, endIndex);
        emit changed();
        return;
}

void BookmarkRootItem::aboutToAddChildren(BookmarkItem *parent, int startIndex, int endIndex) {
        QModelIndex index = indexFromItem(parent, 0);
        beginInsertRows(index, startIndex, endIndex);
        BookmarkItem::aboutToAddChildren(parent, startIndex, endIndex);
        return;
}

void BookmarkRootItem::childrenAdded(BookmarkItem *parent, int startIndex, int endIndex) {
        endInsertRows();
        BookmarkItem::childrenAdded(parent, startIndex, endIndex);
        emit changed();
        return;
}

void BookmarkRootItem::faviconLookup(const BookmarkItem *item) const {
        if(item == NULL) return;
        if(item->url().isEmpty()) return;
        _faviconStore->lookup(item->uuid(), item->url());
        return;
}

void BookmarkRootItem::faviconLoaded(const QUuid &uuid, const QUrl &url, const QIcon &icon) {
        Q_UNUSED(url);
        if(icon.isNull()) return;
        BookmarkItem *item = findNodeByUUID(uuid);
        if(item == NULL) return;
        QModelIndex index = indexFromItem(item, ColumnName);
        emit QAbstractItemModel::dataChanged(index, index);
        return;
}


