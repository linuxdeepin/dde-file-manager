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

DFMBASE_USE_NAMESPACE
DPMYSHARES_USE_NAMESPACE
DSC_USE_NAMESPACE

ShareFileInfo::ShareFileInfo(const QUrl &url)
    : AbstractFileInfo(url, new ShareFileInfoPrivate(url, this))
{
    QString path = url.path();
    setProxy(InfoFactory::create<AbstractFileInfo>(QUrl::fromLocalFile(path)));
}

ShareFileInfo::~ShareFileInfo()
{
}

QUrl ShareFileInfo::redirectedFileUrl() const
{
    const QString &path = dptr->url.path();
    return QUrl::fromLocalFile(path);
}

QString ShareFileInfo::fileDisplayName() const
{
    return fileName();
}

QString ShareFileInfo::fileName() const
{
    auto d = dynamic_cast<ShareFileInfoPrivate *>(dptr.data());
    return d->info.getShareName();
}

bool ShareFileInfo::isDir() const
{
    // a `openFile` function is register, so we have to treat the share folder like a normal file, when double click to access the directory,
    // the `openFile` will be invoked to open the real directory
    return false;
}

bool ShareFileInfo::canRename() const
{
    return false;
}

bool ShareFileInfo::isShared() const
{
    return true;
}

ShareFileInfoPrivate::ShareFileInfoPrivate(const QUrl &url, AbstractFileInfo *qq)
    : AbstractFileInfoPrivate(qq)
{
    this->url = url;
    if (url.path() != "/")
        info = UserShareService::service()->getInfoByPath(url.path());
}

ShareFileInfoPrivate::~ShareFileInfoPrivate()
{
}
