// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEEXTENDATTRIBTHREADPRIVATE_H
#define FILEEXTENDATTRIBTHREADPRIVATE_H

#include <dfm-base/dfm_base_global.h>

#include <QUrl>
#include <QList>
#include <QQueue>

namespace dfmbase {
class FileExtendAttribThread;
class FileExtendAttribThreadPrivate
{
    friend class FileExtendAttribThread;
    Q_DISABLE_COPY(FileExtendAttribThreadPrivate)
private:
    explicit FileExtendAttribThreadPrivate(FileExtendAttribThread *extendAttribThread = nullptr);

    virtual ~FileExtendAttribThreadPrivate();

private:
    FileExtendAttribThread *fileExtendAttibThread { nullptr };
    QList<QUrl> urlList {};
    qint64 totalSize { 0 };
    QQueue<QUrl> dirList {};
    int state { 0 };
};
}
#endif   //FILEEXTENDATTRIBTHREADPRIVATE_H
