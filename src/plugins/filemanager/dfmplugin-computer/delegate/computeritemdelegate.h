/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#ifndef COMPUTERITEMDELEGATE_H
#define COMPUTERITEMDELEGATE_H

#include "dfmplugin_computer_global.h"

#include <QStyledItemDelegate>

DPCOMPUTER_BEGIN_NAMESPACE

class ComputerView;
class ComputerItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ComputerItemDelegate(QObject *parent = nullptr);
    virtual ~ComputerItemDelegate() override;

public:
    // QAbstractItemDelegate interface
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void closeEditor(ComputerView *view);

private:
    void paintSplitter(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paintCustomWidget(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paintUserDirectory(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paintDevice(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void prepareColor(QPainter *painter, const QStyleOptionViewItem &option) const;

    void drawDeviceIcon(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawDeviceLabelAndFs(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawDeviceDetail(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    ComputerView *view { nullptr };
    mutable QLineEdit *renameEditor { nullptr };
    mutable QModelIndex editingIndex;
};

DPCOMPUTER_END_NAMESPACE

#endif   // COMPUTERITEMDELEGATE_H
