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
    : AbstractFileInfo(url), d(new SmbShareFileInfoPrivate(url, this))
{
    dptr.reset(d);
}

SmbShareFileInfo::~SmbShareFileInfo()
{
}

QString SmbShareFileInfo::nameOf(const NameInfoType type) const
{
    switch (type) {
    case NameInfoType::kFileName:
        [[fallthrough]];
    case NameInfoType::kFileCopyName:
        return d->fileName();
    default:
        return AbstractFileInfo::nameOf(type);
    }
}

QString SmbShareFileInfo::displayOf(const DisPlayInfoType type) const
{
    if (DisPlayInfoType::kFileDisplayName == type)
        return d->fileName();
    return AbstractFileInfo::displayOf(type);
}

QIcon SmbShareFileInfo::fileIcon()
{
    auto dp = dynamic_cast<SmbShareFileInfoPrivate *>(dptr.data());
    return QIcon::fromTheme(dp->node.iconType);
}

bool SmbShareFileInfo::isAttributes(const OptInfoType type) const
{
    switch (type) {
    case FileIsType::kIsDir:
        [[fallthrough]];
    case FileIsType::kIsReadable:
        [[fallthrough]];
    case FileIsType::kIsWritable:
        return true;
    default:
        return AbstractFileInfo::isAttributes(type);
    }
}

bool SmbShareFileInfo::canAttributes(const CanableInfoType type) const
{
    switch (type) {
    case FileCanType::kCanDrag:
        return false;
    default:
        return AbstractFileInfo::canAttributes(type);
    }
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

QString SmbShareFileInfoPrivate::fileName() const
{
    return node.displayName;
}
