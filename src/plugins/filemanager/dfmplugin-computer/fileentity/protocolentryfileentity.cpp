// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-base/dfm_global_defines.h>
#include "protocolentryfileentity.h"
#include "utils/computerdatastruct.h"
#include "utils/computerutils.h"

#include <dfm-base/file/entry/entryfileinfo.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/utils/protocolutils.h>
#include <dfm-base/base/device/devicealiasmanager.h>

#include <QRegularExpression>

using namespace dfmplugin_computer;
using namespace GlobalServerDefines;
DFMBASE_USE_NAMESPACE

/*!
 * \class ProtocolEntryFileEntity
 * \brief class that present protocol devices
 */
ProtocolEntryFileEntity::ProtocolEntryFileEntity(const QUrl &url)
    : AbstractEntryFileEntity(url)
{
    if (!url.path().endsWith(SuffixInfo::kProtocol)) {
        fmCritical() << "Invalid protocol device URL suffix:" << url;
        abort();
    }

    refresh();
}

QString ProtocolEntryFileEntity::displayName() const
{
    QString displayName = datas.value(DeviceProperty::kDisplayName).toString();
    const auto &alias = NPDeviceAliasManager::instance()->getAlias(targetUrl());

    QString host, share;
    bool isSmb = dfmbase::DeviceUtils::parseSmbInfo(displayName, host, share);
    if (isSmb) {
        if (alias.isEmpty())
            displayName = tr("%1 on %2").arg(share, host);
        else
            displayName = tr("%1 on %2").arg(share, alias);
    } else if (!alias.isEmpty()) {
        static QRegularExpression ipRegex(R"([0-9]{1,3}(?:\.[0-9]{1,3}){3})");
        displayName.replace(ipRegex, alias);
    }

    return displayName;
}

QString ProtocolEntryFileEntity::editDisplayText() const
{
    const auto &url = targetUrl();
    const auto &alias = NPDeviceAliasManager::instance()->getAlias(url);
    if (!alias.isEmpty())
        return alias;

    if (!url.host().isEmpty())
        return url.host();

    QString devId = datas.value(DeviceProperty::kId).toString();
    QUrl idUrl(devId);
    if (!idUrl.host().isEmpty())
        return idUrl.host();

    return displayName();
}

QIcon ProtocolEntryFileEntity::icon() const
{
    auto icons = datas.value(DeviceProperty::kDeviceIcon).toStringList();
    for (auto iconName : icons) {
        QString devId = datas.value(DeviceProperty::kId).toString();
        if (iconName == "phone" && (devId.startsWith("gphoto") || devId.startsWith("mtp")))
            iconName = "android-device";
        if (devId.contains("Apple_Inc") || devId.startsWith("afc"))
            iconName = "ios-device";
        auto iconObj = QIcon::fromTheme(iconName);
        if (iconObj.isNull())
            continue;
        return iconObj;
    }

    fmWarning() << "No valid icon found for protocol device:" << entryUrl << "available icons:" << icons;
    return {};
}

bool ProtocolEntryFileEntity::exists() const
{
    return !datas.value(DeviceProperty::kMountPoint).toString().isEmpty();
}

bool ProtocolEntryFileEntity::showProgress() const
{
    return true;
}

bool ProtocolEntryFileEntity::showTotalSize() const
{
    return true;
}

bool ProtocolEntryFileEntity::showUsageSize() const
{
    return true;
}

DFMBASE_NAMESPACE::AbstractEntryFileEntity::EntryOrder ProtocolEntryFileEntity::order() const
{
    const QString &id = datas.value(DeviceProperty::kId).toString();

    if (id.startsWith(DFMBASE_NAMESPACE::Global::Scheme::kFtp)
        || id.startsWith(DFMBASE_NAMESPACE::Global::Scheme::kSFtp))
        return DFMBASE_NAMESPACE::AbstractEntryFileEntity::EntryOrder::kOrderFtp;

    if (id.startsWith(DFMBASE_NAMESPACE::Global::Scheme::kSmb)
        || DFMBASE_NAMESPACE::ProtocolUtils::isSMBFile(QUrl(id)))
        return DFMBASE_NAMESPACE::AbstractEntryFileEntity::EntryOrder::kOrderSmb;

    if (id.startsWith(DFMBASE_NAMESPACE::Global::Scheme::kMtp))
        return DFMBASE_NAMESPACE::AbstractEntryFileEntity::EntryOrder::kOrderMTP;

    if (id.startsWith(DFMBASE_NAMESPACE::Global::Scheme::kGPhoto2))
        return DFMBASE_NAMESPACE::AbstractEntryFileEntity::EntryOrder::kOrderGPhoto2;

    return DFMBASE_NAMESPACE::AbstractEntryFileEntity::EntryOrder::kOrderFiles;
}

quint64 ProtocolEntryFileEntity::sizeTotal() const
{
    return datas.value(DeviceProperty::kSizeTotal).toULongLong();
}

quint64 ProtocolEntryFileEntity::sizeUsage() const
{
    return datas.value(DeviceProperty::kSizeUsed).toULongLong();
}

void ProtocolEntryFileEntity::refresh()
{
    auto id = entryUrl.path().remove("." + QString(SuffixInfo::kProtocol));
    datas = DFMBASE_NAMESPACE::UniversalUtils::convertFromQMap(DevProxyMng->queryProtocolInfo(id));
}

QUrl ProtocolEntryFileEntity::targetUrl() const
{
    auto mpt = datas.value(DeviceProperty::kMountPoint).toString();
    if (mpt.isEmpty()) {
        fmDebug() << "No mount point found for protocol device:" << entryUrl;
        return QUrl();
    }

    QUrl target = QUrl::fromLocalFile(mpt);
    if (DFMBASE_NAMESPACE::ProtocolUtils::isSMBFile(target))
        return DFMBASE_NAMESPACE::DeviceUtils::getSambaFileUriFromNative(target);

    return target;
}

bool ProtocolEntryFileEntity::renamable() const
{
    return DFMBASE_NAMESPACE::NPDeviceAliasManager::instance()->canSetAlias(targetUrl());
}
