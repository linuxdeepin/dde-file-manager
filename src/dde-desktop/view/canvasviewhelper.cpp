/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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

#include "canvasviewhelper.h"

#include <dfmevent.h>
#include <dfileinfo.h>
#include <singleton.h>
#include <dfilesystemmodel.h>
#include <diconitemdelegate.h>
#include <app/define.h>
#include <app/filesignalmanager.h>

#include "canvasgridview.h"
#include "desktopitemdelegate.h"

CanvasViewHelper::CanvasViewHelper(CanvasGridView *parent): DFileViewHelper(parent)
{
    connect(fileSignalManager, &FileSignalManager::requestSelectFile,
            this, &CanvasViewHelper::handleSelectEvent);

    //交由CanvasView处理，修复项目经过自动排列后编辑框显示问题
    disconnect(fileSignalManager, SIGNAL(requestSelectRenameFile(DFMUrlBaseEvent)),
              this, SLOT(_q_selectAndRename(DFMUrlBaseEvent)));
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

DFMStyledItemDelegate *CanvasViewHelper::itemDelegate() const
{
    return qobject_cast<DFMStyledItemDelegate *>(parent()->itemDelegate());
}

DFileSystemModel *CanvasViewHelper::model() const
{
    return parent()->model();
}

const DUrlList CanvasViewHelper::selectedUrls() const
{
    return parent()->selectedUrls();
}

void CanvasViewHelper::select(const QList<DUrl> &list)
{
    return parent()->select(list);
}

void CanvasViewHelper::edit(const DFMEvent &event)
{
    if (event.windowId() != windowId() || event.fileUrlList().isEmpty()) {
        return;
    }

    DUrl fileUrl = event.fileUrlList().first();

    if (fileUrl.isEmpty()) {
        return;
    }

    const QModelIndex &index = model()->index(fileUrl);

    parent()->edit(index, QAbstractItemView::EditKeyPressed, nullptr);
}

void CanvasViewHelper::onRequestSelectFiles(const QList<DUrl> &urls)
{
    select(urls);
}

void CanvasViewHelper::handleSelectEvent(const DFMUrlListBaseEvent &event)
{
    // TODO: should check fileSignalManager->requestSelectFile() and ensure sender correct.
    if (event.sender() != this->parent()
            && event.sender() != this->model()) {
        return;
    }

    select(event.urlList());
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

    bool transp = isTransparent(index);

    if (transp) {
        option->backgroundBrush = QColor("#BFE4FC");
    }

    if ((option->state & QStyle::State_HasFocus) && option->showDecorationSelected && selectedIndexsCount() > 1) {
        option->palette.setColor(QPalette::Background, QColor("#0076F9"));

        if (!transp)
            option->backgroundBrush = QColor("#0076F9");
    } else {
        option->palette.setColor(QPalette::Background, QColor("#2da6f7"));

        if (!transp)
            option->backgroundBrush = QColor("#2da6f7");
    }

    option->textElideMode = Qt::ElideLeft;
}

int CanvasViewHelper::selectedIndexsCount() const
{
    return parent()->selectedIndexCount();
}

bool CanvasViewHelper::isSelected(const QModelIndex &index) const
{
    return parent()->isSelected(index);
}

void CanvasViewHelper::viewFlicker()
{
    auto fileModel = model();
    auto view = parent();
    if (fileModel && view) {
        //刷新时的闪烁效果
        m_paintFile = false;
        view->repaint(); //依赖Qt重绘ui，若本次repaint不进入paintevent，则不会闪烁
        view->update();
        m_paintFile = true;

        //刷新model
        fileModel->refresh();
    }
}

bool CanvasViewHelper::isPaintFile() const
{
    return m_paintFile;
}
