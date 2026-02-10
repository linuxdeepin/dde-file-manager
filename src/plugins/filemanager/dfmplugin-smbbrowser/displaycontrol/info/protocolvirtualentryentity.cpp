// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "protocolvirtualentryentity.h"
#include "displaycontrol/datahelper/virtualentrydbhandler.h"
#include <dfm-base/base/device/devicealiasmanager.h>

DPSMBBROWSER_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

ProtocolVirtualEntryEntity::ProtocolVirtualEntryEntity(const QUrl &url)
    : AbstractEntryFileEntity(url)
{
}

QString dfmplugin_smbbrowser::ProtocolVirtualEntryEntity::displayName() const
{
    static constexpr char kVEntryDisplayName[] { "ventry_display_name" };
    if (datas.value(kVEntryDisplayName).toString().isEmpty()) {
        datas.insert(kVEntryDisplayName, VirtualEntryDbHandler::instance()->getDisplayNameOf(entryUrl));
    }

    auto &&displayName =  datas.value(kVEntryDisplayName).toString();
    const auto &alias = NPDeviceAliasManager::instance()->getAlias(targetUrl());
    if (!alias.isEmpty())
        displayName.replace(targetUrl().host(), alias);

    return displayName;
}

QString dfmplugin_smbbrowser::ProtocolVirtualEntryEntity::editDisplayText() const
{
    const auto &alias = NPDeviceAliasManager::instance()->getAlias(targetUrl());
    return alias.isEmpty() ? targetUrl().host() : alias;
}

QIcon dfmplugin_smbbrowser::ProtocolVirtualEntryEntity::icon() const
{
    return QIcon::fromTheme("folder-remote");
}

bool dfmplugin_smbbrowser::ProtocolVirtualEntryEntity::exists() const
{
    return true;
}

bool dfmplugin_smbbrowser::ProtocolVirtualEntryEntity::showProgress() const
{
    return false;
}

bool dfmplugin_smbbrowser::ProtocolVirtualEntryEntity::showTotalSize() const
{
    return false;
}

bool dfmplugin_smbbrowser::ProtocolVirtualEntryEntity::showUsageSize() const
{
    return false;
}

AbstractEntryFileEntity::EntryOrder dfmplugin_smbbrowser::ProtocolVirtualEntryEntity::order() const
{
    return AbstractEntryFileEntity::EntryOrder::kOrderSmb;
}

QUrl ProtocolVirtualEntryEntity::targetUrl() const
{
    QString path = entryUrl.path();
    path.remove("." + QString(kVEntrySuffix));
    auto ret = QUrl(path);
    if (ret.path() == "/" || ret.path().isEmpty())
        return ret;
    return VirtualEntryDbHandler::instance()->getFullSmbPath(path);
}

bool ProtocolVirtualEntryEntity::renamable() const
{
    return NPDeviceAliasManager::instance()->canSetAlias(targetUrl());
}
