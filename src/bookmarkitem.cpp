/*****************************************************************************
 * bookmarkitem.cpp
 * May 15, 2015
 *
 *****************************************************************************/

#include <stdio.h>
#include <QDateTime>
#include "bookmarkitem.h"

int BookmarkItem::columnCount() {
        return 2;
}

QString BookmarkItem::columnName(int col) {
        QString ret;
        switch(col) {
                case ColumnName: ret = "Name"; break;
                case ColumnURL: ret = "URL"; break;
        }
        return ret;
}

BookmarkItem::BookmarkItem(BookmarkItem *p, const QUuid &u) :
        _parent(NULL),
        _uuid(u.isNull() ? QUuid::createUuid() : u),
        _holdTimeStamp(false)
{
        setParent(p);
        updateTimeStamp();
}

BookmarkItem::~BookmarkItem() {
        setParent(NULL);
        clearChildren();
}

void BookmarkItem::updateTimeStamp() {
        if(_holdTimeStamp) return;
        _timestamp = QDateTime::currentMSecsSinceEpoch();
        dataChanged(this, ColumnTimeStamp);
        return;
}

void BookmarkItem::setHoldTimeStamp(bool val) {
        _holdTimeStamp = val;
        return;
}

void BookmarkItem::setTimeStamp(qint64 val) {
        bool hts = _holdTimeStamp;
        _holdTimeStamp = true;
        _timestamp = val;
        dataChanged(this, ColumnTimeStamp);
        _holdTimeStamp = hts;
        return;
}

void BookmarkItem::clearChildren() {
        int ct = _children.size();
        if(ct == 0) return;
        aboutToRemoveChildren(this, 0, ct - 1);
        for(int i = 0; i < ct; i++) {
                BookmarkItem *item = _children.at(i);
                item->_parent = NULL;
                delete item;
        }
        _children.clear();
        childrenRemoved(this, 0, ct - 1);
        updateTimeStamp();
        return;
}

void BookmarkItem::setParent(BookmarkItem *val, int index) {
        if(val == this) return;
        if(val != NULL) if(val->isContainedBy(this)) return;
        if(_parent != NULL) _parent->removeChild(this);
        _parent = val;
        if(_parent != NULL) _parent->addChild(this, index);
        updateTimeStamp();
        return;
}

void BookmarkItem::setUUID(const QUuid &val) {
        if(_uuid == val) return;
        _uuid = val;
        dataChanged(this, ColumnUUID);
        return;
}

void BookmarkItem::setName(const QString &val) {
        if(_name == val) return;
        _name = val;
        dataChanged(this, ColumnName);
        return;
}

void BookmarkItem::setURL(const QString &val) {
        if(_url == val) return;
        _url = val;
        dataChanged(this, ColumnURL);
        return;
}

void BookmarkItem::removeChild(BookmarkItem *item) {
        int index = _children.indexOf(item);
        if(index == -1) return;
        aboutToRemoveChildren(this, index, index);
        _children.removeAt(index);
        childrenRemoved(this, index, index);
        return;
}

void BookmarkItem::addChild(BookmarkItem *item, int index) {
        if(index < 0) index = _children.size();
        aboutToAddChildren(this, index, index);
        _children.insert(index, item);
        childrenAdded(this, index, index);
        return;
}

void BookmarkItem::dataChanged(BookmarkItem *item, int column) {
        if(_parent != NULL) _parent->dataChanged(item, column);
        if(column != ColumnTimeStamp) updateTimeStamp();
        return;
}

void BookmarkItem::aboutToRemoveChildren(BookmarkItem *parent, int startIndex, int endIndex) {
        if(_parent != NULL) _parent->aboutToRemoveChildren(parent, startIndex, endIndex);
        return;
}

void BookmarkItem::childrenRemoved(BookmarkItem *parent, int startIndex, int endIndex) {
        if(_parent != NULL) _parent->childrenRemoved(parent, startIndex, endIndex);
        updateTimeStamp();
        return;
}

void BookmarkItem::aboutToAddChildren(BookmarkItem *parent, int startIndex, int endIndex) {
        if(_parent != NULL) _parent->aboutToAddChildren(parent, startIndex, endIndex);
        return;
}

void BookmarkItem::childrenAdded(BookmarkItem *parent, int startIndex, int endIndex) {
        if(_parent != NULL) _parent->childrenAdded(parent, startIndex, endIndex);
        updateTimeStamp();
        return;
}


