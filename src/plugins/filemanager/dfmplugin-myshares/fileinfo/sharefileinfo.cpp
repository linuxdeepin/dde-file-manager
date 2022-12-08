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
    : AbstractFileInfo(url), d(new ShareFileInfoPrivate(url, this))
{
    dptr.reset(d);
    QString path = url.path();
    setProxy(InfoFactory::create<AbstractFileInfo>(QUrl::fromLocalFile(path)));
}

ShareFileInfo::~ShareFileInfo()
{
}

QString ShareFileInfo::displayInfo(const AbstractFileInfo::DisplayInfoType type) const
{
    if (AbstractFileInfo::DisplayInfoType::kFileDisplayName == type)
        return d->fileName();
    return AbstractFileInfo::displayInfo(type);
}

QString ShareFileInfo::nameInfo(const NameInfo type) const
{
    switch (type) {
    case NameInfo::kFileName:
        [[fallthrough]];
    case NameInfo::kFileCopyName:
        return d->fileName();
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
        return AbstractFileInfo::urlInfo(type);
    }
}

bool ShareFileInfo::isAttributes(const AbstractFileInfo::FileIsType type) const
{
    switch (type) {
    case FileIsType::kIsDir:
        return true;
    case FileIsType::kIsWritable:
        return false;
    default:
        return AbstractFileInfo::isAttributes(type);
    }
}

bool ShareFileInfo::canAttributes(const AbstractFileInfo::FileCanType type) const
{
    switch (type) {
    case FileCanType::kCanRename:
        [[fallthrough]];
    case FileCanType::kCanDrag:
        return false;
    default:
        return AbstractFileInfo::canAttributes(type);
    }
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

void dfmplugin_myshares::ShareFileInfo::refresh()
{
    d->refresh();
}
