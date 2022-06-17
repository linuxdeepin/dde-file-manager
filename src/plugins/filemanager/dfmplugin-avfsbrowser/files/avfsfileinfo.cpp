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

#include "avfsfileinfo.h"
#include "private/avfsfileinfo_p.h"
#include "utils/avfsutils.h"

#include "dfm-base/base/schemefactory.h"

DPAVFSBROWSER_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

AvfsFileInfo::AvfsFileInfo(const QUrl &url)
    : AbstractFileInfo(url, new AvfsFileInfoPrivate(this))
{
    setProxy(InfoFactory::create<AbstractFileInfo>(AvfsUtils::avfsUrlToLocal(url)));
}

AvfsFileInfo::~AvfsFileInfo()
{
}

AvfsFileInfoPrivate::AvfsFileInfoPrivate(AvfsFileInfo *qq)
    : AbstractFileInfoPrivate(qq)
{
}

QUrl AvfsFileInfo::url() const
{
    return dptr->url;
}
