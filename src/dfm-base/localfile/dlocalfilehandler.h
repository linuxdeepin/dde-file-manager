/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
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

#include "localfile/dfilehandler.h"

class DLocalFileHandler : public DFileHandler
{
public:
    DLocalFileHandler();

    bool exists(const QUrl &url) override;
    bool touch(const QUrl &url) override;
    bool mkdir(const QUrl &url) override;
    bool mkpath(const QUrl &url) override;
    bool link(const QString &path, const QUrl &linkUrl) override;
    bool remove(const QUrl &url) override;
    bool rmdir(const QUrl &url) override;
    bool rename(const QUrl &url, const QUrl &newUrl) override;
    bool setPermissions(const QUrl &url, QFileDevice::Permissions permissions) override;
    bool setFileTime(const QUrl &url, const QDateTime &accessDateTime, const QDateTime &lastModifiedTime) override;
};

#endif // DLOCALFILEHANDLER_H
