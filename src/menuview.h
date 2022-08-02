/*****************************************************************************
 * menuview.h
 * May 26, 2015
 *
 *****************************************************************************/

#ifndef _MENUVIEW_H_
#define _MENUVIEW_H_

#include <QMenu>
#include <QHash>
#include <QPersistentModelIndex>

class QAction;
class QAbstractItemModel;
class QModelIndex;

class MenuView : public QMenu {
        Q_OBJECT
        public:
                MenuView(QWidget *parent = NULL);
                ~MenuView();

                QAbstractItemModel *model() const;

        public slots:
                void setModel(QAbstractItemModel *model);

        signals:
                void itemClicked(const QModelIndex &index);

        private slots:
                void modelDestroyed(QObject *obj);
                void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
                void rowsInserted(const QModelIndex &parent, int start, int end);
                void rowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, 
                               const QModelIndex &destParent, int destStart);
                void rowsRemoved(const QModelIndex &index, int start, int end);
                void modelReset();
                void refreshMenus();
                void _refreshMenus();
                void actionTriggered();

        private:
                QAbstractItemModel                              *_model;
                bool                                            _pendingRefresh;
                QHash<QPersistentModelIndex, QAction *>         _indexToAction;

                void refreshMenu(QMenu *menu, const QModelIndex &index);
                void updateAction(QAction *action, const QModelIndex &index);
                QMenu *indexToMenu(const QModelIndex &index, bool includeRoot);
};

inline QAbstractItemModel *MenuView::model() const {
        return _model;
}

#endif /* ifndef _MENUVIEW_H_ */


