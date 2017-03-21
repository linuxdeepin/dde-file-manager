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
    explicit ShareControler(QObject *parent = 0);

    const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvnet> &event) const Q_DECL_OVERRIDE;
    const QList<DAbstractFileInfoPointer> getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const Q_DECL_OVERRIDE;
    DAbstractFileWatcher *createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const Q_DECL_OVERRIDE;

    bool unShareFolder(const QSharedPointer<DFMCancelFileShareEvent> &event) const Q_DECL_OVERRIDE;

private:
    static DUrl realUrl(const DUrl &shareUrl);
};

#endif // SHARECONTROLER_H
