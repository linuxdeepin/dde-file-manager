/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef SHARECONTROLER_H
#define SHARECONTROLER_H

#include "dabstractfilecontroller.h"

class ShareControler : public DAbstractFileController
{
    Q_OBJECT
public:
    explicit ShareControler(QObject *parent = nullptr);

    const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const override;
    const QList<DAbstractFileInfoPointer> getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const override;
    DAbstractFileWatcher *createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const override;

    // 文件打开接口
    bool openFile(const QSharedPointer<DFMOpenFileEvent> &event) const override;
    // 权限设置接口
    bool setPermissions(const QSharedPointer<DFMSetPermissionEvent> &event) const override;
    bool shareFolder(const QSharedPointer<DFMFileShareEvent> &event) const override;
    bool unShareFolder(const QSharedPointer<DFMCancelFileShareEvent> &event) const override;

    bool addToBookmark(const QSharedPointer<DFMAddToBookmarkEvent> &event) const override;
    bool removeBookmark(const QSharedPointer<DFMRemoveBookmarkEvent> &event) const override;

private:
    static DUrl realUrl(const DUrl &shareUrl);
};

#endif // SHARECONTROLER_H
