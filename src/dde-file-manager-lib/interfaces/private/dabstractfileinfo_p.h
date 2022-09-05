// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
