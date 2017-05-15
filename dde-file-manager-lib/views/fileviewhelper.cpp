/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "fileviewhelper.h"
#include "dfileview.h"
#include "dfmevent.h"
#include "app/define.h"
#include "app/filesignalmanager.h"
#include "controllers/appcontroller.h"
#include "dfilesystemmodel.h"
#include "singleton.h"
#include "usershare/usersharemanager.h"

FileViewHelper::FileViewHelper(DFileView *parent)
    : DFileViewHelper(parent)
    , lastEvent(this, DUrl())
{
    connect(parent, &DFileView::triggerEdit, this, &DFileViewHelper::triggerEdit);
    connect(parent, &DFileView::rootUrlChanged, this, &FileViewHelper::onCurrentUrlChanged);

    connect(fileSignalManager, &FileSignalManager::requestViewSelectAll,
            this, &FileViewHelper::selectAll);
    connect(fileSignalManager, &FileSignalManager::requestSelectFile,
            this, &FileViewHelper::handleSelectEvent);
    connect(fileSignalManager, &FileSignalManager::requestFoucsOnFileView,
            this, &FileViewHelper::setFoucsOnFileView);
    connect(fileSignalManager, &FileSignalManager::requestFreshFileView,
            this, &FileViewHelper::refreshFileView);
}

DFileView *FileViewHelper::parent() const
{
    return qobject_cast<DFileView*>(DFileViewHelper::parent());
}

quint64 FileViewHelper::windowId() const
{
    return parent()->windowId();
}

bool FileViewHelper::isSelected(const QModelIndex &index) const
{
    return parent()->isSelected(index);
}

bool FileViewHelper::isDropTarget(const QModelIndex &index) const
{
    return parent()->isDropTarget(index);
}

int FileViewHelper::selectedIndexsCount() const
{
    return parent()->selectedIndexCount();
}

int FileViewHelper::rowCount() const
{
    return parent()->rowCount();
}

int FileViewHelper::indexOfRow(const QModelIndex &index) const
{
    return parent()->indexOfRow(index);
}

const DAbstractFileInfoPointer FileViewHelper::fileInfo(const QModelIndex &index) const
{
    return parent()->model()->fileInfo(index);
}

DStyledItemDelegate *FileViewHelper::itemDelegate() const
{
    return parent()->itemDelegate();
}

DFileSystemModel *FileViewHelper::model() const
{
    return parent()->model();
}

const DUrlList FileViewHelper::selectedUrls() const
{
    return parent()->selectedUrls();
}

DUrl FileViewHelper::currentUrl() const
{
    return parent()->rootUrl();
}

QList<int> FileViewHelper::columnRoleList() const
{
    return parent()->columnRoleList();
}

int FileViewHelper::columnWidth(int columnIndex) const
{
    return parent()->columnWidth(columnIndex);
}

void FileViewHelper::select(const QList<DUrl> &list)
{
    parent()->select(list);
}

void FileViewHelper::preHandleCd(const DFMUrlBaseEvent &event)
{
    if (event.windowId() != windowId())
        return;

    if (event.fileUrl().isNetWorkFile()) {
        emit fileSignalManager->requestFetchNetworks(event);
        return;
    } else if (event.fileUrl().isSMBFile()) {
        emit fileSignalManager->requestFetchNetworks(event);
        return;
    }

    lastEvent = event;
    parent()->cd(event.fileUrl());
    lastEvent = DFMUrlBaseEvent(this, DUrl());
}

void FileViewHelper::cd(const DFMUrlBaseEvent &event)
{
    if (event.windowId() != windowId())
        return;

    lastEvent = event;
    parent()->cd(event.fileUrl());
    lastEvent = DFMUrlBaseEvent(this, DUrl());
}

void FileViewHelper::cdUp(const DFMUrlBaseEvent &event)
{
    if (event.windowId() != windowId())
        return;

    lastEvent = event;
    parent()->cdUp();
    lastEvent = DFMUrlBaseEvent(this, DUrl());
}

void FileViewHelper::handleSelectEvent(const DFMUrlListBaseEvent &event)
{
    if (event.windowId() != windowId()) {
        return;
    }

    select(event.urlList());
}

void FileViewHelper::selectAll(quint64 windowId)
{
    if(windowId != this->windowId())
        return;

    parent()->selectAll();
}

void FileViewHelper::setFoucsOnFileView(quint64 winId)
{
    if (winId == windowId()) {
        parent()->setFocus();
    }
}

void FileViewHelper::refreshFileView(quint64 winId)
{
    if (winId != windowId()) {
        return;
    }

    model()->refresh();
}

void FileViewHelper::onCurrentUrlChanged(const DUrl &url)
{
    DFMUrlBaseEvent e(lastEvent);
    e.setWindowId(windowId());
    e.setData(url);
    emit fileSignalManager->currentUrlChanged(e);
}
