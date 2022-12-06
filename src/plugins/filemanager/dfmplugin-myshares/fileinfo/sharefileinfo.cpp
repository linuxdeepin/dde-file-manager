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
#include "sharefileinfo.h"
#include "private/sharefileinfo_p.h"

#include "dfm-base/base/schemefactory.h"

#include <dfm-framework/event/event.h>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_myshares;

ShareFileInfo::ShareFileInfo(const QUrl &url)
    : AbstractFileInfo(url)
{
    dptr.reset(new ShareFileInfoPrivate(url, this));
    QString path = url.path();
    setProxy(InfoFactory::create<AbstractFileInfo>(QUrl::fromLocalFile(path)));
}

ShareFileInfo::~ShareFileInfo()
{
}

QString ShareFileInfo::displayInfo(const AbstractFileInfo::DisplayInfoType type) const
{
    if (AbstractFileInfo::DisplayInfoType::kFileDisplayName == type)
        return dptr.staticCast<ShareFileInfoPrivate>()->fileName();
    return AbstractFileInfo::displayInfo(type);
}

QString ShareFileInfo::nameInfo(const AbstractFileInfo::FileNameInfoType type) const
{
    switch (type) {
    case AbstractFileInfo::FileNameInfoType::kFileName:
        [[fallthrough]];
    case AbstractFileInfo::FileNameInfoType::kFileCopyName:
        return dptr.staticCast<ShareFileInfoPrivate>()->fileName();
    default:
        return AbstractFileInfo::nameInfo(type);
    }
}

QUrl ShareFileInfo::urlInfo(const AbstractFileInfo::FileUrlInfoType type) const
{
    switch (type) {
    case FileUrlInfoType::kRedirectedFileUrl:
        return QUrl::fromLocalFile(dptr->url.path());
    default:
        return urlInfo(type);
    }
}

bool ShareFileInfo::isDir() const
{
    return true;
}

bool ShareFileInfo::canRename() const
{
    return false;
}

ShareFileInfoPrivate::ShareFileInfoPrivate(const QUrl &url, AbstractFileInfo *qq)
    : AbstractFileInfoPrivate(url, qq)
{
    refresh();
}

ShareFileInfoPrivate::~ShareFileInfoPrivate()
{
}

void ShareFileInfoPrivate::refresh()
{
    if (url.path() != "/")
        info = dpfSlotChannel->push("dfmplugin_dirshare", "slot_Share_ShareInfoOfFilePath", url.path()).value<QVariantMap>();
}

QString ShareFileInfoPrivate::fileName() const
{
    return info.value(ShareInfoKeys::kName).toString();
}

bool dfmplugin_myshares::ShareFileInfo::canDrag()
{
    return false;
}

bool dfmplugin_myshares::ShareFileInfo::isWritable() const
{
    return true;
}

void dfmplugin_myshares::ShareFileInfo::refresh()
{
    auto d = dynamic_cast<ShareFileInfoPrivate *>(dptr.data());
    d->refresh();
}
