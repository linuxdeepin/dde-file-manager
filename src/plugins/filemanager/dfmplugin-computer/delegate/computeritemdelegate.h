// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPUTERITEMDELEGATE_H
#define COMPUTERITEMDELEGATE_H

#include "dfmplugin_computer_global.h"

#include <QStyledItemDelegate>
#include <QLineEdit>

namespace dfmplugin_computer {

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
    virtual bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index) override;

    void closeEditor(ComputerView *view);

private:
    void paintSplitter(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paintCustomWidget(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paintSmallItem(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paintLargeItem(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void prepareColor(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void drawDeviceIcon(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawDeviceLabelAndFs(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawDeviceDetail(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    QPixmap renderBlurShadow(const QSize &sz, const QColor &color, int blurRadius) const;
    QPixmap renderBlurShadow(const QPixmap &pm, int blurRadius) const;
    QColor getProgressTotalColor() const;

private:
    ComputerView *view { nullptr };
    mutable QLineEdit *renameEditor { nullptr };
    mutable QModelIndex editingIndex;
};

}

#endif   // COMPUTERITEMDELEGATE_H
