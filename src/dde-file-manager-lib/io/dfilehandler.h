// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFILEHANDLER_H
#define DFILEHANDLER_H

#include <dfmglobal.h>

DFM_BEGIN_NAMESPACE

class DFileHandlerPrivate;
class DFileHandler
{
public:
    virtual ~DFileHandler();

    QString errorString() const;

    virtual bool exists(const DUrl &url) = 0;
    virtual bool touch(const DUrl &url) = 0;
    virtual bool mkdir(const DUrl &url) = 0;
    virtual bool mkpath(const DUrl &url) = 0;
    virtual bool link(const QString &path, const DUrl &linkUrl) = 0;
    virtual bool remove(const DUrl &url) = 0;
    virtual bool rmdir(const DUrl &url) = 0;
    virtual bool rename(const DUrl &url, const DUrl &newUrl) = 0;
    virtual bool setPermissions(const DUrl &url, QFileDevice::Permissions permissions) = 0;
    virtual bool setFileTime(const DUrl &url, const QDateTime &accessDateTime, const QDateTime &lastModifiedTime) = 0;

protected:
    DFileHandler();
    explicit DFileHandler(DFileHandlerPrivate &dd);

    QScopedPointer<DFileHandlerPrivate> d_ptr;

    Q_DECLARE_PRIVATE(DFileHandler)
};

DFM_END_NAMESPACE
Q_DECLARE_METATYPE(DFM_NAMESPACE::DFileHandler*)

#endif // DFILEHANDLER_H
