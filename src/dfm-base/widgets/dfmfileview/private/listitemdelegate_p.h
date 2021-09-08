#ifndef LISTITEMDELEGATE_P_H
#define LISTITEMDELEGATE_P_H

#include "dfm_base_global.h"

#include <DListView>

#include <QObject>

DFMBASE_BEGIN_NAMESPACE

class ListItemDelegate;
class ListItemDelegatePrivate : public QObject
{
    Q_DECLARE_PUBLIC(ListItemDelegate)
    ListItemDelegate * const q_ptr;
    DTK_WIDGET_NAMESPACE::DListView *listView;
    QModelIndex editingIndex;
    int textLineHeight;

public:
    explicit ListItemDelegatePrivate(ListItemDelegate *qq);
    virtual ~ListItemDelegatePrivate();
};

DFMBASE_END_NAMESPACE

#endif // LISTITEMDELEGATE_P_H
