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
#include "filemonitor/filemonitor.h"

class ShareControler : public DAbstractFileController
{
    Q_OBJECT
public:
    explicit ShareControler(QObject *parent = 0);

    const DAbstractFileInfoPointer createFileInfo(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
    const QList<DAbstractFileInfoPointer> getChildren(const DUrl &fileUrl, const QStringList &nameFilters,
                                                     QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                                     bool &accepted) const Q_DECL_OVERRIDE;
    DAbstractFileWatcher *createFileWatcher(const DUrl &fileUrl, QObject *parent, bool &accepted) const Q_DECL_OVERRIDE;
};

#endif // SHARECONTROLER_H
