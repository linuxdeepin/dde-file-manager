// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "protocolvirtualentryentity.h"
#include "displaycontrol/datahelper/virtualentrydbhandler.h"

DPSMBBROWSER_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

ProtocolVirtualEntryEntity::ProtocolVirtualEntryEntity(const QUrl &url)
    : AbstractEntryFileEntity(url)
{
}

QString dfmplugin_smbbrowser::ProtocolVirtualEntryEntity::displayName() const
{
    static constexpr char kVEntryDisplayName[] { "ventry_display_name" };
    if (!datas.value(kVEntryDisplayName).toString().isEmpty())
        return datas.value(kVEntryDisplayName).toString();
    datas.insert(kVEntryDisplayName, VirtualEntryDbHandler::instance()->getDisplayNameOf(entryUrl));
    return datas.value(kVEntryDisplayName).toString();
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
