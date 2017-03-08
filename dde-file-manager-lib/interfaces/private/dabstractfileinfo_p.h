/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef DABSTRACTFILEINFO_P_H
#define DABSTRACTFILEINFO_P_H

#include "dabstractfileinfo.h"
#include "dmimedatabase.h"

#include <QPointer>

QT_BEGIN_NAMESPACE
class QReadWriteLock;
QT_END_NAMESPACE

DFM_USE_NAMESPACE

class DAbstractFileInfoPrivate
{
public:
    DAbstractFileInfoPrivate(const DUrl &url, DAbstractFileInfo *qq, bool hasCache);
    virtual ~DAbstractFileInfoPrivate();

    void setUrl(const DUrl &url, bool hasCache);
    static DAbstractFileInfo *getFileInfo(const DUrl &fileUrl);

    DAbstractFileInfo *q_ptr = Q_NULLPTR;

    mutable QString pinyinName;
    bool active = false;

    DAbstractFileInfoPointer proxy;
    static DMimeDatabase mimeDatabase;

private:
    DUrl fileUrl;
    static QReadWriteLock *urlToFileInfoMapLock;
    static QMap<DUrl, DAbstractFileInfo*> urlToFileInfoMap;

    Q_DECLARE_PUBLIC(DAbstractFileInfo)
};

#endif // DABSTRACTFILEINFO_P_H
