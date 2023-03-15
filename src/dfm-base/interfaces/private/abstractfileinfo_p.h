// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTFILEINFO_P_H
#define ABSTRACTFILEINFO_P_H

#include "dfm-base/utils/threadcontainer.hpp"
#include "dfm-base/interfaces/abstractfileinfo.h"

#include <dfm-io/dfileinfo.h>

#include <QPointer>

USING_IO_NAMESPACE
namespace dfmbase {

class AbstractFileInfoPrivate
{
    friend class AbstractFileInfo;

public:
    QUrl url;   // 文件的url
    explicit AbstractFileInfoPrivate(const QUrl &url, AbstractFileInfo *qq);
    virtual ~AbstractFileInfoPrivate();

    AbstractFileInfo *const q;   // DAbstractFileInfo实例对象
    AbstractFileInfoPointer proxy { nullptr };
    QMap<ExtInfoType, QVariant> extendOtherCache;
    QString pinyinName;
    QMap<DFMIO::DFileInfo::AttributeID, QVariant> cacheAttributes;

private:
    QUrl getUrlByChildFileName(const QString &fileName) const;
    QUrl getUrlByNewFileName(const QString &fileName) const;
    QString fileName() const;
    QString baseName() const;
    QString suffix() const;
    bool canDrop();
};

}

#endif   // ABSTRACTFILEINFO_P_H
