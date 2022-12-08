/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhuangshu<zhuangshu@uniontech.com>
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
#include "smbintegrationentity.h"

#include "dfm-base/base/standardpaths.h"

using namespace dfmplugin_smbbrowser;

/*!
 * \class SmbIntegrationEntity
 * \brief class that present stashed protocol devices
 */
SmbIntegrationEntity::SmbIntegrationEntity(const QUrl &url)
    : AbstractEntryFileEntity(url)
{
}

QString SmbIntegrationEntity::displayName() const
{
    return entryUrl.host();
}

QIcon SmbIntegrationEntity::icon() const
{
    return QIcon::fromTheme("folder-remote");
}

bool SmbIntegrationEntity::exists() const
{
    return true;
}

bool SmbIntegrationEntity::showProgress() const
{
    return false;
}

bool SmbIntegrationEntity::showTotalSize() const
{
    return false;
}

bool SmbIntegrationEntity::showUsageSize() const
{
    return false;
}

DFMBASE_NAMESPACE::EntryFileInfo::EntryOrder SmbIntegrationEntity::order() const
{
    return DFMBASE_NAMESPACE::EntryFileInfo::EntryOrder::kOrderStashedSmb;
}

bool SmbIntegrationEntity::isAccessable() const
{
    return true;
}
