// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DLOCALFILEHANDLER_H
#define DLOCALFILEHANDLER_H

#include <dfilehandler.h>

DFM_BEGIN_NAMESPACE

class DLocalFileHandler : public DFileHandler
{
public:
    DLocalFileHandler();

    bool exists(const DUrl &url) override;
    bool touch(const DUrl &url) override;
    bool mkdir(const DUrl &url) override;
    bool mkpath(const DUrl &url) override;
    bool link(const QString &path, const DUrl &linkUrl) override;
    bool remove(const DUrl &url) override;
    bool rmdir(const DUrl &url) override;
    bool rename(const DUrl &url, const DUrl &newUrl) override;
    bool setPermissions(const DUrl &url, QFileDevice::Permissions permissions) override;
    bool setFileTime(const DUrl &url, const QDateTime &accessDateTime, const QDateTime &lastModifiedTime) override;
};

DFM_END_NAMESPACE

#endif // DLOCALFILEHANDLER_H
