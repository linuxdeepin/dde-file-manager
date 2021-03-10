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
