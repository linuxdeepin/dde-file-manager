/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
#ifndef ABSTRACTFILEINFO_P_H
#define ABSTRACTFILEINFO_P_H

#include "base/abstractfileinfo.h"
#include "utils/threadcontainer.hpp"

#include <QPointer>

#include <dfm-io/core/dfileinfo.h>

USING_IO_NAMESPACE
DFMBASE_BEGIN_NAMESPACE
class AbstractFileInfoPrivate
{
    Q_DECLARE_PUBLIC(AbstractFileInfo)

public:
    AbstractFileInfoPrivate(AbstractFileInfo *qq);
    virtual ~AbstractFileInfoPrivate();

    QSharedPointer<DFileInfo> dfmFileInfo = nullptr;                     // dfm文件的信息
    QUrl url;                                                            // 文件的url
    AbstractFileInfo * const q_ptr;                                      // DAbstractFileInfo实例对象
private:
    DThreadMap<AbstractFileInfo::FileInfoCacheType, QVariant> caches;    // 文件缓存
};

DFMBASE_END_NAMESPACE

#endif // ABSTRACTFILEINFO_P_H
