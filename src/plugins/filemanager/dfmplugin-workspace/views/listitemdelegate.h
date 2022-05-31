/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef LISTITEMDELEGATE_H
#define LISTITEMDELEGATE_H

#include "baseitemdelegate.h"

#include "services/filemanager/workspace/workspace_defines.h"
#include "dfm-base/dfm_global_defines.h"

#include <QStyledItemDelegate>

DPWORKSPACE_BEGIN_NAMESPACE

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

    Q_DECLARE_PRIVATE_D(d, ListItemDelegate)
};

DPWORKSPACE_END_NAMESPACE

#endif   // DLISTITEMDELEGATE_H
