/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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

#include "infodatafuture.h"
#include "utils/fileinfohelper.h"

USING_IO_NAMESPACE
namespace dfmbase {
InfoDataFuture::InfoDataFuture(DFileFuture *future, QObject *parent)
    : QObject(parent), future(future)
{
    if (future) {
        qRegisterMetaType<QMap<DFileInfo::AttributeExtendID, QVariant>>("QMap<DFileInfo::AttributeExtendID, QVariant>");
        connect(future, &DFileFuture::infoMedia, this, &InfoDataFuture::infoMedia);
        connect(this, &InfoDataFuture::infoMediaAttributes, &FileInfoHelper::instance(), &FileInfoHelper::mediaDataFinished);
    }
}

QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> InfoDataFuture::mediaInfo() const
{
    return attribute;
}

bool InfoDataFuture::isFinished() const
{
    return finshed;
}

InfoDataFuture::~InfoDataFuture()
{
}

void InfoDataFuture::infoMedia(const QUrl &url, const QMap<DFileInfo::AttributeExtendID, QVariant> &map)
{
    // 先处理数据缓存下来，再转发信号给infohelper，析构future
    attribute = std::move(map);
    finshed = true;
    emit infoMediaAttributes(url, map);
    future.reset(nullptr);
}

}
