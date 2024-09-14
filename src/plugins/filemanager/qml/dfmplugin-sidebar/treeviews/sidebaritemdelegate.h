// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SIDEBARITEMDELEGATE_H
#define SIDEBARITEMDELEGATE_H

#include "dfmplugin_sidebar_global.h"

#include <dfm-base/interfaces/fileinfo.h>

#include <DStyledItemDelegate>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE
DPSIDEBAR_BEGIN_NAMESPACE

class SideBarItemDelegate : public DStyledItemDelegate
{
    Q_OBJECT
public:
    explicit SideBarItemDelegate(QAbstractItemView *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setModelData(QWidget *editor,
                      QAbstractItemModel *model,
                      const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index) override;
public Q_SLOTS:
    void onEditorTextChanged(const QString &text, const FileInfoPointer &info) const;

private:
    void drawIcon(const QStyleOptionViewItem &option, QPainter *painter,
                  const QRect &itemRect, bool isEjectable, QSize iconSize, QIcon::Mode iconMode, QPalette::ColorGroup cg) const;

    void drawMouseHoverBackground(QPainter *painter, const DPalette &palette, const QRect &r, const QColor &widgetColor) const;
    void drawMouseHoverExpandButton(QPainter *painter, const QRect &r, bool isExpanded) const;

Q_SIGNALS:
    void rename(const QModelIndex &index, QString newName) const;
    void changeExpandState(const QModelIndex &index, bool expand);
};

DPSIDEBAR_END_NAMESPACE

#endif   // SIDEBARITEMDELEGATE_H
