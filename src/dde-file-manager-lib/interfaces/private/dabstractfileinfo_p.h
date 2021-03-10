/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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
    qint8 gvfsMountFile = -1;

    DAbstractFileInfoPointer proxy;
    static DMimeDatabase mimeDatabase;

    bool columnCompact = false;

    Q_DECLARE_PUBLIC(DAbstractFileInfo)

private:
    DUrl fileUrl;
    static QReadWriteLock *urlToFileInfoMapLock;
    static QMap<DUrl, DAbstractFileInfo*> urlToFileInfoMap;
};

#endif // DABSTRACTFILEINFO_P_H
