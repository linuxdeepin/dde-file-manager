/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "canvasviewhelper.h"

#include <dfmevent.h>
#include <dfileinfo.h>
#include <dfilesystemmodel.h>
#include <diconitemdelegate.h>

#include "canvasgridview.h"
#include "desktopitemdelegate.h"

CanvasViewHelper::CanvasViewHelper(CanvasGridView *parent): DFileViewHelper(parent)
{

}

CanvasGridView *CanvasViewHelper::parent() const
{
    return qobject_cast<CanvasGridView *>(DFileViewHelper::parent());
}

quint64 CanvasViewHelper::windowId() const
{
    return parent()->winId();
}

const DAbstractFileInfoPointer CanvasViewHelper::fileInfo(const QModelIndex &index) const
{
    return parent()->model()->fileInfo(index);
}

DStyledItemDelegate *CanvasViewHelper::itemDelegate() const
{
    return qobject_cast<DStyledItemDelegate *>(parent()->itemDelegate());
}

DFileSystemModel *CanvasViewHelper::model() const
{
    return parent()->model();
}

const DUrlList CanvasViewHelper::selectedUrls() const
{
    qDebug() << parent()->selectedUrls();
    return parent()->selectedUrls();
}

void CanvasViewHelper::select(const QList<DUrl> &list)
{
    qDebug() << "+++++++++++++++ ------------" << list;
    return parent()->select(list);
}

void CanvasViewHelper::edit(const DFMEvent &event)
{
    qDebug() << event.windowId() << windowId();
    if (event.windowId() != windowId() || event.fileUrlList().isEmpty()) {
        return;
    }

    DUrl fileUrl = event.fileUrlList().first();

    if (fileUrl.isEmpty()) {
        return;
    }

    const QModelIndex &index = model()->index(fileUrl);

    parent()->edit(index, QAbstractItemView::EditKeyPressed, 0);
}


void CanvasViewHelper::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    if (isSelected(index)) {
        option->state |= QStyle::State_Selected;
    } else {
        option->state &= QStyle::StateFlag(~QStyle::State_Selected);
    }

    option->palette.setColor(QPalette::Text, QColor("white"));
    option->palette.setColor(QPalette::Disabled, QPalette::Text, QColor("#797979"));
    if ((option->state & QStyle::State_Selected) && option->showDecorationSelected) {
        option->palette.setColor(QPalette::Inactive, QPalette::Text, QColor("#e9e9e9"));
    } else {
        option->palette.setColor(QPalette::Inactive, QPalette::Text, QColor("#797979"));
    }
    option->palette.setColor(QPalette::BrightText, Qt::white);
    option->palette.setBrush(QPalette::Shadow, QColor(0, 0, 0, 178));

    if ((option->state & QStyle::State_HasFocus) && option->showDecorationSelected && selectedIndexsCount() > 1) {
        option->backgroundBrush = QColor("#0076F9");
    } else {
        option->backgroundBrush = QColor("#2da6f7");
    }
}

int CanvasViewHelper::selectedIndexsCount() const
{
    return parent()->selectedIndexCount();
}

bool CanvasViewHelper::isSelected(const QModelIndex &index) const
{
    return parent()->isSelected(index);
}
