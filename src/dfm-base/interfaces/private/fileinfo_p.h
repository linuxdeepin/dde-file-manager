// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEINFO_P_H
#define FILEINFO_P_H

#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/utils/threadcontainer.h>
#include <dfm-base/interfaces/fileinfo.h>

#include <QPointer>

USING_IO_NAMESPACE
namespace dfmbase {

class FileInfoPrivate
{
public:
    explicit FileInfoPrivate(FileInfo *qq);
    virtual ~FileInfoPrivate();

    FileInfo *const q;   // DAbstractFileInfo实例对象

public:
    QUrl getUrlByChildFileName(const QString &fileName) const;
    QUrl getUrlByNewFileName(const QString &fileName) const;
    QString fileName() const;
    QString baseName() const;
    QString suffix() const;
    bool canDrop();
};

}

#endif   // FILEINFO_P_H
