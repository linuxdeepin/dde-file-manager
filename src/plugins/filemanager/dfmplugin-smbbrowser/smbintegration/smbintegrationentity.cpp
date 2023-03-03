// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
