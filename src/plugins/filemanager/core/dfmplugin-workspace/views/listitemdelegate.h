// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LISTITEMDELEGATE_H
#define LISTITEMDELEGATE_H

#include "baseitemdelegate.h"

#include <dfm-base/dfm_global_defines.h>

#include <QStyledItemDelegate>

namespace dfmplugin_workspace {

class ListItemEditor;
class ListItemDelegatePrivate;
class ListItemDelegate : public BaseItemDelegate
{
    Q_OBJECT
public:
    explicit ListItemDelegate(FileViewHelper *parent = nullptr);
    virtual ~ListItemDelegate() override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    QList<QRect> paintGeomertys(const QStyleOptionViewItem &option, const QModelIndex &index, bool sizeHintMode = false) const override;
    void updateItemSizeHint() override;

    QRectF itemIconRect(const QRectF &itemRect) const override;
    QRect getRectOfItem(RectOfItemType type, const QModelIndex &index) const override;

protected:
    bool eventFilter(QObject *object, QEvent *event) override;
    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index) override;

private slots:
    void editorFinished();

private:
    void paintItemBackground(QPainter *painter, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const;
    QRectF paintItemIcon(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const;
    void paintItemColumn(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index, const QRectF &iconRect) const;
    void paintFileName(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const int &role, const QRectF &rect, const int &textLineHeight, const QUrl &url) const;

    bool setEditorData(ListItemEditor *editor);

    int dataWidth(const QStyleOptionViewItem &option, const QModelIndex &index, int role) const;

    bool expandable { false };
    Q_DECLARE_PRIVATE_D(d, ListItemDelegate)
};

}

#endif   // DLISTITEMDELEGATE_H
