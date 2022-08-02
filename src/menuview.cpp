/*****************************************************************************
 * menuview.cpp
 * May 26, 2015
 *
 *****************************************************************************/

#include <stdio.h>
#include <QIcon>
#include <QAbstractItemModel>
#include <QTimer>
#include "menuview.h"

#define NAME_COLUMN     0

Q_DECLARE_METATYPE(QPersistentModelIndex);

MenuView::MenuView(QWidget *p) :
        QMenu(p),
        _model(NULL),
        _pendingRefresh(false)
{

}

MenuView::~MenuView() {
        
}

void MenuView::setModel(QAbstractItemModel *model) {
        if(_model != NULL) {
                disconnect(_model, NULL, this, NULL);
                disconnect(this, NULL, _model, NULL);
        }
        _model = model;
        if(_model != NULL) {
                connect(_model, SIGNAL(destroyed(QObject *)), this, SLOT(modelDestroyed(QObject *)));
                connect(_model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
                        this, SLOT(dataChanged(const QModelIndex &, const QModelIndex &)));
                connect(_model, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
                        this, SLOT(rowsInserted(const QModelIndex &, int, int)));
                connect(_model, SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
                        this, SLOT(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
                connect(_model, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
                        this, SLOT(rowsRemoved(const QModelIndex &, int, int)));
                connect(_model, SIGNAL(modelReset()), this, SLOT(modelReset()));
        }
        refreshMenus();
        return;
}

void MenuView::modelDestroyed(QObject *obj) {
        Q_UNUSED(obj);
        setModel(NULL);
        return;
}

void MenuView::refreshMenus() {
        if(_pendingRefresh) return;
        _pendingRefresh = true;
        QTimer::singleShot(0, this, SLOT(_refreshMenus()));
        return;
}

void MenuView::_refreshMenus() {
        _pendingRefresh = false;
        clear();
        _indexToAction.clear();
        if(_model == NULL) return;
        refreshMenu(this, QModelIndex());
        return;
}

void MenuView::refreshMenu(QMenu *menu, const QModelIndex &parent) {
        if(_model == NULL) return;
        int ct = _model->rowCount(parent);
        for(int i = 0; i < ct; i++) {
                QAction *action = NULL;
                QModelIndex index = _model->index(i, NAME_COLUMN, parent);
                if(_model->hasChildren(index)) {
                        QMenu *childMenu = new QMenu;
                        action = menu->addMenu(childMenu);
                        refreshMenu(childMenu, index);
                } else {
                        action = new QAction(menu);
                        action = menu->addAction("");
                }
                connect(action, SIGNAL(triggered()), this, SLOT(actionTriggered()));
                updateAction(action, index);
        }
        return;
}

void MenuView::actionTriggered() {
        QAction *action = qobject_cast<QAction *>(sender());
        if(action == NULL) return;
        QPersistentModelIndex index = action->data().value<QPersistentModelIndex>();
        if(index.isValid()) emit itemClicked(index);
        return;
}

void MenuView::updateAction(QAction *action, const QModelIndex &index) {
        if(action == NULL) return;
        QString name = _model->data(index).toString();
        QIcon icon = _model->data(index, Qt::DecorationRole).value<QIcon>();
        name.replace("&", "&&");
        action->setText(name);
        action->setIcon(icon);
        action->setIconVisibleInMenu(true);
        QVariant data;
        data.setValue(QPersistentModelIndex(index));
        action->setData(data);
        _indexToAction[index] = action;
        return;
}

QMenu *MenuView::indexToMenu(const QModelIndex &index, bool includeRoot) {
        if(!index.isValid()) return includeRoot ? this : NULL;
        QAction *action = _indexToAction[index];
        if(action == NULL) return includeRoot ? this : NULL;
        return action->menu();
}

void MenuView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight) {
        if(topLeft.column() > NAME_COLUMN) return;
        QModelIndex parent = topLeft.parent();
        QMenu *menu = indexToMenu(parent, true);
        if(menu == NULL) return;
        QList<QAction *> actionList = menu->actions();
        for(int i = topLeft.row(); i <= bottomRight.row() && i < actionList.size(); i++) {
                QModelIndex index = _model->index(i, NAME_COLUMN, parent);
                updateAction(actionList[i], index);
        }
        return;
}

void MenuView::rowsInserted(const QModelIndex &parent, int start, int end) {
        Q_UNUSED(parent);
        Q_UNUSED(start);
        Q_UNUSED(end);
        refreshMenus();
        return;
}

void MenuView::rowsMoved(const QModelIndex &srcParent, int srcStart, int srcEnd, const QModelIndex &destParent, int destStart) {
        Q_UNUSED(srcParent);
        Q_UNUSED(srcStart);
        Q_UNUSED(srcEnd);
        Q_UNUSED(destParent);
        Q_UNUSED(destStart);
        refreshMenus();
        return;
}

void MenuView::rowsRemoved(const QModelIndex &index, int start, int end) {
        Q_UNUSED(index);
        Q_UNUSED(start);
        Q_UNUSED(end);
        refreshMenus();
        return;
}

void MenuView::modelReset() {
        refreshMenus();
        return;
}

