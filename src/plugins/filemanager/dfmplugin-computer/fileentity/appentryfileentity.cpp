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
#include "appentryfileentity.h"

DFMBASE_USE_NAMESPACE
DPCOMPUTER_BEGIN_NAMESPACE

/*!
 * \class AppEntryFileEntity
 * \brief class that present application entries.
 */
AppEntryFileEntity::AppEntryFileEntity(const QUrl &url)
    : AbstractEntryFileEntity(url)
{
    // TODO(xust) FINISH THIS CLASS
}

QString AppEntryFileEntity::displayName() const
{
    return {};
}

QIcon AppEntryFileEntity::icon() const
{
    return {};
}

bool AppEntryFileEntity::exists() const
{
    return {};
}

bool AppEntryFileEntity::showProgress() const
{
    return {};
}

bool AppEntryFileEntity::showTotalSize() const
{
    return {};
}

bool AppEntryFileEntity::showUsageSize() const
{
    return {};
}

void AppEntryFileEntity::onOpen()
{
}

QString AppEntryFileEntity::description() const
{
    return {};
}

EntryFileInfo::EntryOrder AppEntryFileEntity::order() const
{
    return EntryFileInfo::EntryOrder::kOrderApps;
}

DPCOMPUTER_END_NAMESPACE
