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
