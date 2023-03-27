// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEINFO_P_H
#define FILEINFO_P_H

#include "dfm-base/utils/threadcontainer.hpp"
#include "dfm-base/interfaces/fileinfo.h"

#include <dfm-io/dfileinfo.h>

#include <QPointer>

USING_IO_NAMESPACE
namespace dfmbase {

class FileInfoPrivate
{
    friend class FileInfo;

public:
    QUrl url;   // 文件的url
    explicit FileInfoPrivate(const QUrl &url, FileInfo *qq);
    virtual ~FileInfoPrivate();

    FileInfo *const q;   // DAbstractFileInfo实例对象
    FileInfoPointer proxy { nullptr };
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

#endif   // FILEINFO_P_H
