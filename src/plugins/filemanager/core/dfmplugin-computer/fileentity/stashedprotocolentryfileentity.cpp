// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stashedprotocolentryfileentity.h"
#include "utils/stashmountsutils.h"
#include "utils/computerdatastruct.h"

#include "dfm-base/base/standardpaths.h"

using namespace dfmplugin_computer;

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

DFMBASE_NAMESPACE::EntryFileInfo::EntryOrder StashedProtocolEntryFileEntity::order() const
{
    return DFMBASE_NAMESPACE::EntryFileInfo::EntryOrder::kOrderStashedSmb;
}

bool StashedProtocolEntryFileEntity::isAccessable() const
{
    return true;
}
