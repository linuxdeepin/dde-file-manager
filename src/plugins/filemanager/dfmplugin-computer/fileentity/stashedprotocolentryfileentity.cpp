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
#include "utils/stashmountsutils.h"
#include "utils/computerdatastruct.h"

#include "dfm-base/base/standardpaths.h"

#include <QMenu>

DPCOMPUTER_USE_NAMESPACE

/*!
 * \class StashedProtocolEntryFileEntity
 * \brief class that present stashed protocol devices
 */
StashedProtocolEntryFileEntity::StashedProtocolEntryFileEntity(const QUrl &url)
    : AbstractEntryFileEntity(url)
{
}

QString StashedProtocolEntryFileEntity::displayName() const
{
    return StashMountsUtils::displayName(entryUrl);
}

QIcon StashedProtocolEntryFileEntity::icon() const
{
    return QIcon::fromTheme("folder-remote");
}

bool StashedProtocolEntryFileEntity::exists() const
{
    return StashMountsUtils::isStashedDevExist(entryUrl);
}

bool StashedProtocolEntryFileEntity::showProgress() const
{
    return false;
}

bool StashedProtocolEntryFileEntity::showTotalSize() const
{
    return false;
}

bool StashedProtocolEntryFileEntity::showUsageSize() const
{
    return false;
}

void StashedProtocolEntryFileEntity::onOpen()
{
}

dfmbase::EntryFileInfo::EntryOrder StashedProtocolEntryFileEntity::order() const
{
    return dfmbase::EntryFileInfo::EntryOrder::kOrderStashedSmb;
}

QMenu *StashedProtocolEntryFileEntity::createMenu()
{
    QMenu *menu = new QMenu();
    menu->addAction(ContextMenuActionTrs::trMount());
    menu->addAction(ContextMenuActionTrs::trRemove());
    menu->addSeparator();
    menu->addAction(ContextMenuActionTrs::trProperties());
    return menu;
}

bool StashedProtocolEntryFileEntity::isAccessable() const
{
    return true;
}
