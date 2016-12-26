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

#include <QPointer>

class DAbstractFileInfoPrivate
{
public:
    DAbstractFileInfoPrivate(const DUrl &url, DAbstractFileInfo *qq);
    ~DAbstractFileInfoPrivate();

    void setUrl(const DUrl &url);
    static DAbstractFileInfo *getFileInfo(const DUrl &fileUrl);

    DAbstractFileInfo *q_ptr = Q_NULLPTR;

    mutable QString pinyinName;
    bool active = false;

    DAbstractFileInfoPointer proxy;

private:
    DUrl fileUrl;
    static QMap<DUrl, DAbstractFileInfo*> urlToFileInfoMap;

    Q_DECLARE_PUBLIC(DAbstractFileInfo)
};

#endif // DABSTRACTFILEINFO_P_H
