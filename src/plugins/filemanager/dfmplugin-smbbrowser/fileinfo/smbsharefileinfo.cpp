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

using namespace dfmplugin_smbbrowser;
DFMBASE_USE_NAMESPACE

SmbShareFileInfo::SmbShareFileInfo(const QUrl &url)
    : AbstractFileInfo(url)
{
    dptr.reset(new SmbShareFileInfoPrivate(url, this));
}

SmbShareFileInfo::~SmbShareFileInfo()
{
}

QString SmbShareFileInfo::nameInfo(const AbstractFileInfo::FileNameInfoType type) const
{
    switch (type) {
    case AbstractFileInfo::FileNameInfoType::kFileName:
        [[fallthrough]];
    case AbstractFileInfo::FileNameInfoType::kFileCopyName:
        return dptr.staticCast<SmbShareFileInfoPrivate>()->fileName();
    default:
        return AbstractFileInfo::nameInfo(type);
    }
}

QString SmbShareFileInfo::displayInfo(const AbstractFileInfo::DisplayInfoType type) const
{
    if (AbstractFileInfo::DisplayInfoType::kFileDisplayName == type)
        return dptr.staticCast<SmbShareFileInfoPrivate>()->fileName();
    return AbstractFileInfo::displayInfo(type);
}

QIcon SmbShareFileInfo::fileIcon()
{
    auto dp = dynamic_cast<SmbShareFileInfoPrivate *>(dptr.data());
    return QIcon::fromTheme(dp->node.iconType);
}

bool SmbShareFileInfo::isDir() const
{
    return true;
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
    : AbstractFileInfoPrivate(url, qq)
{
    {
        QMutexLocker locker(&SmbBrowserUtils::mutex);
        node = SmbBrowserUtils::shareNodes.value(url);
        qDebug() << "smb share url: " << url
                 << "\nnode info: " << node;
    }
}

SmbShareFileInfoPrivate::~SmbShareFileInfoPrivate()
{
}

bool dfmplugin_smbbrowser::SmbShareFileInfo::canDrag()
{
    return false;
}

QString SmbShareFileInfoPrivate::fileName() const
{
    return node.displayName;
}
