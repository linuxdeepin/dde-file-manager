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
#ifndef DFILEHANDLER_H
#define DFILEHANDLER_H

#include "base/dfmglobal.h"

class DFileHandlerPrivate;
class DFileHandler
{
public:
    virtual ~DFileHandler();

    QString errorString() const;

    virtual bool exists(const QUrl &url) = 0;
    virtual bool touch(const QUrl &url) = 0;
    virtual bool mkdir(const QUrl &url) = 0;
    virtual bool mkpath(const QUrl &url) = 0;
    virtual bool link(const QString &path, const QUrl &linkUrl) = 0;
    virtual bool remove(const QUrl &url) = 0;
    virtual bool rmdir(const QUrl &url) = 0;
    virtual bool rename(const QUrl &url, const QUrl &newUrl) = 0;
    virtual bool setPermissions(const QUrl &url, QFileDevice::Permissions permissions) = 0;
    virtual bool setFileTime(const QUrl &url, const QDateTime &accessDateTime, const QDateTime &lastModifiedTime) = 0;

protected:
    DFileHandler();
    explicit DFileHandler(DFileHandlerPrivate &dd);

    QScopedPointer<DFileHandlerPrivate> d_ptr;

    Q_DECLARE_PRIVATE(DFileHandler)
};


Q_DECLARE_METATYPE(DFileHandler*)

#endif // DFILEHANDLER_H
