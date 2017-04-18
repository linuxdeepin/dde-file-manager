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
#include "widgets/singleton.h"
#include "usershare/usersharemanager.h"

FileViewHelper::FileViewHelper(DFileView *parent)
    : DFileViewHelper(parent)
    , lastEventSender(this)
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

int FileViewHelper::windowId() const
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

void FileViewHelper::preHandleCd(const DFMEvent &event)
{
    if (event.eventId() != windowId())
        return;

    if (event.fileUrl().isNetWorkFile()) {
        emit fileSignalManager->requestFetchNetworks(event);
        return;
    } else if (event.fileUrl().isSMBFile()) {
        emit fileSignalManager->requestFetchNetworks(event);
        return;
    }

    lastEventSender = event.sender();
    parent()->cd(event.fileUrl());
    lastEventSender = this;
}

void FileViewHelper::cd(const DFMEvent &event)
{
    if (event.eventId() != windowId())
        return;

    lastEventSender = event.sender();
    parent()->cd(event.fileUrl());
    lastEventSender = this;
}

void FileViewHelper::cdUp(const DFMEvent &event)
{
    if (event.eventId() != windowId())
        return;

    lastEventSender = event.sender();
    parent()->cdUp();
    lastEventSender = this;
}

void FileViewHelper::handleSelectEvent(const DFMEvent &event)
{
    if (event.eventId() != windowId()) {
        return;
    }

    select(event.fileUrlList());
}

void FileViewHelper::selectAll(int windowId)
{
    if(windowId != this->windowId())
        return;

    parent()->selectAll();
}

void FileViewHelper::setFoucsOnFileView(const DFMEvent &event)
{
    if (event.eventId() == windowId()) {
        parent()->setFocus();
    }
}

void FileViewHelper::refreshFileView(const DFMEvent &event)
{
    if (event.eventId() != windowId()) {
        return;
    }

    model()->refresh();
}

void FileViewHelper::onCurrentUrlChanged(const DUrl &url)
{
    DFMEvent e(lastEventSender);
    e.setEventId(windowId());
    e.setData(url);
    emit fileSignalManager->currentUrlChanged(e);
}
