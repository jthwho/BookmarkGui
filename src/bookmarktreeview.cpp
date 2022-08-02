/*****************************************************************************
 * bookmarktreeview.cpp
 * May 15, 2015
 *
 *****************************************************************************/

#include <stdio.h>
#include <QByteArray>
#include <QDataStream>
#include <QMenu>
#include <QMessageBox>
#include <QDropEvent>
#include <QProxyStyle>
#include <QPainter>
#include <QClipboard>
#include <QUrl>
#include <QApplication>
#include "bookmarktreeview.h"
#include "bookmarkitem.h"
#include "bookmarkrootitem.h"

#define BOOKMARKTREEVIEW_STATE_VERSION   1

class BookmarkTreeProxyStyle : public QProxyStyle {
        public:
                void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const {
                        if(element == QStyle::PE_IndicatorItemViewItemDrop) {
                                painter->setRenderHint(QPainter::Antialiasing, true);
                                QPalette palette;
                                QColor c(90,108,217);
                                //QColor c(palette.highlightedText().color());
                                QPen pen(c);
                                pen.setWidth(2);
                                c.setAlpha(50);
                                QBrush brush(c);
                                painter->setPen(pen);
                                painter->setBrush(brush);
                                if(option->rect.height() == 0) {
                                        painter->drawEllipse(option->rect.topLeft(), 3, 3);
                                        painter->drawLine(QPoint(option->rect.topLeft().x()+3, option->rect.topLeft().y()), option->rect.topRight());
                                } else {
                                        painter->drawRoundedRect(option->rect, 5, 5);
                                }
                        } else {
                                QProxyStyle::drawPrimitive(element, option, painter, widget);
                        }
                        return;
                }
};

BookmarkTreeView::BookmarkTreeView(QWidget *p) : 
        QTreeView(p) 
{
        setStyle(new BookmarkTreeProxyStyle);
        setAlternatingRowColors(true);
        setContextMenuPolicy(Qt::CustomContextMenu);
        connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(showContextMenu(const QPoint &)));
        setDragDropMode(DragDrop);
        setDragEnabled(true);
        viewport()->setAcceptDrops(true);
        setDropIndicatorShown(true);
        setSelectionMode(ExtendedSelection);
}

BookmarkRootItem *BookmarkTreeView::rootItem() const {
        return dynamic_cast<BookmarkRootItem *>(model());
}

BookmarkItem *BookmarkTreeView::pasteItem(const QModelIndex &index) {
        QString text = QApplication::clipboard()->text();
        if(text.isEmpty()) return NULL;

        BookmarkItem *item = addItem(index);
        if(QUrl(text).isValid()) {
                item->setURL(text);
        } else {
                item->setName(text);
        }
        return item;
}

void BookmarkTreeView::copyURL(const QModelIndex &index) const {
        if(model() == NULL) return;
        BookmarkItem *item = rootItem()->itemFromIndex(index);
        if(item == NULL) return;
        QApplication::clipboard()->setText(item->url());
        return;
}

BookmarkItem *BookmarkTreeView::addItem(const QModelIndex &index) {
        if(model() == NULL) return NULL;
        BookmarkItem *parent = rootItem()->itemFromIndex(index);
        if(parent == NULL) return NULL;

        BookmarkItem *item = new BookmarkItem(parent);
        item->setName("New Item");
        return item;
}

void BookmarkTreeView::removeItem(const QModelIndex &index, bool force) {
        if(model() == NULL) return;
        BookmarkItem *item = rootItem()->itemFromIndex(index);
        if(item == NULL || item == rootItem()) return;
        if(!force) {
                int ret = QMessageBox::question(this, 
                        "Remove Item?",
                        QString("Are you certain you want to remove '%1'").arg(item->name()),
                        QMessageBox::Yes | QMessageBox::No, QMessageBox::No
                );
                if(ret != QMessageBox::Yes) return;
        }
        delete item;
        return;
}

void BookmarkTreeView::showContextMenu(const QPoint &pt) {
        if(model() == NULL) return;
        QModelIndex index = indexAt(pt);
        BookmarkItem *item = rootItem()->itemFromIndex(index);
        QString url;
        if(item != NULL) url = item->url();

        QMenu menu(this);
        QAction *actionOpenItem = url.isEmpty() ? NULL : menu.addAction("Open Item");
        QAction *actionAddItem = url.isEmpty() ? menu.addAction("Add Item") : NULL;
        QAction *actionRemoveItem = index.isValid() ? menu.addAction("Remove Item") : NULL;
        QAction *actionPasteItem = url.isEmpty() ? menu.addAction("Paste Item") : NULL;
        QAction *actionCopyURL = url.isEmpty() ? NULL : menu.addAction("Copy URL");

        if(actionPasteItem != NULL && QApplication::clipboard()->text().isEmpty()) {
                actionPasteItem->setEnabled(false);
        }
        QAction *action = menu.exec(mapToGlobal(pt), actionOpenItem);
        if(action == NULL) return;
        if(action == actionAddItem) addItem(index);
        else if(action == actionRemoveItem) removeItem(index, false);
        else if(action == actionOpenItem) emit requestOpenURL(url);
        else if(action == actionPasteItem) pasteItem(index);
        else if(action == actionCopyURL) copyURL(index);
        return;
}

void BookmarkTreeView::addToExpandList(QStringList &list, const BookmarkItem *item) {
        if(item == NULL) return;
        if(item->children().isEmpty()) return;
        if(item != rootItem()) {
                if(!isExpanded(rootItem()->indexFromItem((BookmarkItem *)item, 0))) return;
                list += item->uuid();
        }
        for(int i = 0; i < item->children().size(); i++) {
                addToExpandList(list, item->children().at(i));
        }
        return;
}

void BookmarkTreeView::setExpandFromList(const QList<QUuid> &list, const BookmarkItem *item) {
        if(item == NULL) return;
        if(item->children().isEmpty()) return;
        if(list.contains(item->uuid())) {
                setExpanded(rootItem()->indexFromItem((BookmarkItem *)item, 0), true);
        }
        for(int i = 0; i < item->children().size(); i++) {
                setExpandFromList(list, item->children().at(i));
        }
        return;
}

bool BookmarkTreeView::restoreState(const QByteArray &data) {
        QDataStream s(data);
        quint32 version;
        s >> version;
        if(version != BOOKMARKTREEVIEW_STATE_VERSION) return false;
        // Load the column widths
        {
                qint32 cols, size;
                s >> cols;
                int actual = model() == NULL ? 0 : model()->columnCount();
                for(int i = 0; i < cols; i++) {
                        s >> size;
                        if(i < actual) setColumnWidth(i, size);
                }
        }
        // Load the expand list
        {
                QStringList expandStrList;
                s >> expandStrList;
                QList<QUuid> expandList;
                for(int i = 0; i < expandStrList.size(); i++) {
                        QUuid uuid = QUuid(expandStrList.at(i));
                        if(uuid.isNull()) continue;
                        expandList += uuid;
                }
                setExpandFromList(expandList, rootItem());
        }
        return true;
}

QByteArray BookmarkTreeView::saveState() {
        QByteArray ret;
        QDataStream s(&ret, QIODevice::WriteOnly);
        quint32 version = BOOKMARKTREEVIEW_STATE_VERSION;
        s << version;
        // Store the column widths
        {
                qint32 cols = model() == NULL ? 0 : model()->columnCount();
                s << cols;
                for(int i = 0; i < cols; i++) {
                        qint32 val = columnWidth(i);
                        s << val;
                }
        }
        // Store the expand list
        {
                QStringList expandList;
                addToExpandList(expandList, rootItem());
                s << expandList;
        }
        return ret;
}

#if 0
void BookmarkTreeView::startDrag(Qt::DropActions supportedActions) {
        //QTreeView::startDrag(supportedActions);
        QModelIndexList list = selectionModel()->selectedRows(0);
        if(list.isEmpty()) return;
        QMimeData *mimeData = model()->mimeData(list);
        if(mimeData == NULL) return;

        QDrag *drag = new QDrag(this);
        drag->setMimeData(mimeData);
        drag->exec(supportedActions);
        return;
}

void BookmarkTreeView::dropEvent(QDropEvent *event) {
        //QTreeView::dropEvent(event);
        if(event->source() == this) {
                QModelIndexList list = selectionModel()->selectedRows(0);
                QList<BookmarkItem *> items;
                for(int i = 0; i < list.size(); i++) {
                        BookmarkItem *item = rootItem()->itemFromIndex(list.at(i));
                        if(item == NULL) return;
                        items += item;
                        printf("Drop: %s\n", qPrintable(item->name()));
                }
                if(items.isEmpty()) return;
        }
        event->acceptProposedAction();
        return;
}
#endif

