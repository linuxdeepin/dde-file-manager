/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#include "smbsharefileinfo.h"
#include "private/smbsharefileinfo_p.h"
#include "utils/smbbrowserutils.h"

DPSMBBROWSER_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

SmbShareFileInfo::SmbShareFileInfo(const QUrl &url)
    : AbstractFileInfo(url, new SmbShareFileInfoPrivate(url, this))
{
}

SmbShareFileInfo::~SmbShareFileInfo()
{
}

QString SmbShareFileInfo::fileName() const
{
    auto dp = dynamic_cast<SmbShareFileInfoPrivate *>(dptr.data());
    return dp->node.displayName;
}

QString SmbShareFileInfo::fileDisplayName() const
{
    return fileName();
}

QIcon SmbShareFileInfo::fileIcon()
{
    auto dp = dynamic_cast<SmbShareFileInfoPrivate *>(dptr.data());
    return QIcon::fromTheme(dp->node.iconType);
}

bool SmbShareFileInfo::isDir() const
{
    return false;
}

bool SmbShareFileInfo::isReadable() const
{
    return true;
}

bool SmbShareFileInfo::isWritable() const
{
    return true;
}

SmbShareFileInfoPrivate::SmbShareFileInfoPrivate(const QUrl &url, AbstractFileInfo *qq)
    : AbstractFileInfoPrivate(qq)
{
    {
        QMutexLocker locker(&SmbBrowserUtils::mutex);
        node = SmbBrowserUtils::shareNodes.value(url);
        qDebug() << "smb share url: " << url
                 << "\nnode info: " << node;
    }
    this->url = url;
}

SmbShareFileInfoPrivate::~SmbShareFileInfoPrivate()
{
}
