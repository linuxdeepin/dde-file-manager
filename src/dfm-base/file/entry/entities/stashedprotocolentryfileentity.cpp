/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#include "stashedprotocolentryfileentity.h"

DFMBASE_USE_NAMESPACE

/*!
 * \class StashedProtocolEntryFileEntity
 * \brief class that present stashed protocol devices
 */
StashedProtocolEntryFileEntity::StashedProtocolEntryFileEntity(const QUrl &url)
    : AbstractEntryFileEntity(url)
{
    // TODO(xust) FINISH THIS CLASS
}

QString StashedProtocolEntryFileEntity::displayName() const
{
    return {};
}

QIcon StashedProtocolEntryFileEntity::icon() const
{
    return {};
}

bool StashedProtocolEntryFileEntity::exists() const
{
    return {};
}

bool StashedProtocolEntryFileEntity::showProgress() const
{
    return {};
}

bool StashedProtocolEntryFileEntity::showTotalSize() const
{
    return {};
}

bool StashedProtocolEntryFileEntity::showUsageSize() const
{
    return {};
}

void StashedProtocolEntryFileEntity::onOpen()
{
}

EntryFileInfo::EntryOrder StashedProtocolEntryFileEntity::order() const
{
    return EntryFileInfo::EntryOrder::kOrderStashedSmb;
}
