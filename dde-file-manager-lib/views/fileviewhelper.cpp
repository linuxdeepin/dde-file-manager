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
#include "app/filemanagerapp.h"
#include "controllers/appcontroller.h"
#include "dfilesystemmodel.h"
#include "widgets/singleton.h"
#include "usershare/usersharemanager.h"

FileViewHelper::FileViewHelper(DFileView *parent)
    : DFileViewHelper(parent)
    , lastEventSource(DFMEvent::FileView)
{
    connect(parent, &DFileView::triggerEdit, this, &DFileViewHelper::triggerEdit);
    connect(parent, &DFileView::currentUrlChanged, this, &FileViewHelper::onCurrentUrlChanged);

    connect(fileSignalManager, &FileSignalManager::requestRename,
            this, &FileViewHelper::edit);
    connect(fileSignalManager, &FileSignalManager::requestViewSelectAll,
            this, &FileViewHelper::selectAll);
    connect(fileSignalManager, &FileSignalManager::requestSelectFile,
            this, &FileViewHelper::select);
    connect(fileSignalManager, &FileSignalManager::requestSelectRenameFile,
            this, &FileViewHelper::selectAndRename);
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

const QSharedPointer<DAbstractFileInfo> FileViewHelper::fileInfo(const QModelIndex &index) const
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

void FileViewHelper::preHandleCd(const DFMEvent &event)
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

    lastEventSource = event.source();
    parent()->cd(event.fileUrl());
    lastEventSource = DFMEvent::FileView;
}

void FileViewHelper::cd(const DFMEvent &event)
{
    if (event.windowId() != windowId())
        return;

    lastEventSource = event.source();
    parent()->cd(event.fileUrl());
    lastEventSource = DFMEvent::FileView;
}

void FileViewHelper::cdUp(const DFMEvent &event)
{
    if (event.windowId() != windowId())
        return;

    lastEventSource = event.source();
    parent()->cdUp();
    lastEventSource = DFMEvent::FileView;
}

void FileViewHelper::edit(const DFMEvent &event)
{
    if (event.windowId() != windowId() || event.fileUrlList().isEmpty())
        return;

    DUrl fileUrl = event.fileUrlList().first();

    if (fileUrl.isEmpty())
        return;

    const QModelIndex &index = model()->index(fileUrl);

    parent()->edit(index, QAbstractItemView::EditKeyPressed, 0);
}

void FileViewHelper::select(const DFMEvent &event)
{
    if (event.windowId() != windowId()) {
        return;
    }

    parent()->select(event.fileUrlList());
}

void FileViewHelper::selectAll(int windowId)
{
    if(windowId != this->windowId())
        return;

    parent()->selectAll();
}

void FileViewHelper::selectAndRename(const DFMEvent &event)
{
    if (event.windowId() != windowId()) {
        return;
    }

    parent()->select(event.fileUrlList());
    edit(event);
}

void FileViewHelper::setFoucsOnFileView(const DFMEvent &event)
{
    if (event.windowId() == windowId()) {
        parent()->setFocus();
    }
}

void FileViewHelper::refreshFileView(const DFMEvent &event)
{
    if (event.windowId() != windowId()) {
        return;
    }

    model()->refresh();
}

void FileViewHelper::onCurrentUrlChanged(const DUrl &url)
{
    DFMEvent e;
    e << (DFMEvent::EventSource)lastEventSource;
    e << windowId();
    e << currentUrl();
    emit fileSignalManager->currentUrlChanged(e);

    if (url.isUserShareFile()) {
        connect(userShareManager, &UserShareManager::userShareChanged,
                this, &FileViewHelper::onShareFolderCountChanged);
    } else {
        disconnect(userShareManager, &UserShareManager::userShareChanged,
                   this, &FileViewHelper::onShareFolderCountChanged);
    }
}

void FileViewHelper::onShareFolderCountChanged(int count)
{
    if (count == 0) {
        parent()->cd(DUrl::fromLocalFile(QDir::homePath()));
    }
}
