// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "smbbrowserutils.h"
#include "displaycontrol/utilities/protocoldisplayutilities.h"

#include <plugins/common/dfmplugin-utils/reportlog/datas/smbreportdata.h>
#include <dfm-base/settingdialog/settingjsongenerator.h>
#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/configs/settingbackend.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/utils/systemservicemanager.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/dfm_global_defines.h>

#include <dfm-framework/dpf.h>

#include <QUrl>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QMutexLocker>
#include <QtConcurrent>

Q_DECLARE_METATYPE(const char *)
using namespace GlobalDConfDefines::ConfigPath;

namespace dfmplugin_smbbrowser {
namespace smb_browser_utils {

namespace {
static constexpr char kAvahiDaemonService[] { "avahi-daemon.service" };

QString serviceUnitName(const QString &service)
{
    if (service == "smb")
        return "smbd.service";
    if (service == "nmb")
        return "nmbd.service";
    if (service == "smbd" || service == "nmbd")
        return service + ".service";
    if (service == "smbd.service" || service == "nmbd.service")
        return service;
    if (service == kAvahiDaemonService)
        return kAvahiDaemonService;
    return {};
}
}

QString networkScheme()
{
    return "network";
}

QUrl netNeighborRootUrl()
{
    QUrl u;
    u.setScheme(networkScheme());
    u.setPath("/");
    u.setHost("");
    return u;
}

QIcon icon()
{
    return QIcon::fromTheme("network-server-symbolic");
}

bool isSmbMounted(const QString &stdSmb)
{
    using namespace protocol_display_utilities;
    const QStringList &mountedSmbs = getStandardSmbPaths(getMountedSmb());
    QString currSmbPath = stdSmb.toLower();
    if (!currSmbPath.endsWith("/"))
        currSmbPath.append("/");
    return mountedSmbs.contains(currSmbPath);
}

QString getDeviceIdByStdSmb(const QString &stdSmb)
{
    using namespace protocol_display_utilities;
    QString smb = stdSmb.toLower();
    if (!smb.endsWith("/"))
        smb.append("/");

    const QStringList &smbIds = getMountedSmb();
    for (const QString &id : smbIds) {
        const QString &standard = getStandardSmbPath(id);
        if (standard == smb)
            return id;
    }

    fmWarning() << "No matching device ID found for SMB path:" << stdSmb << "using path as fallback";
    return stdSmb;
}

/*!
 * \brief isServiceRuning
 * \param service: options { "smb" | "nmb" }
 * \return
 */
bool isServiceRuning(const QString &service)
{
    const QString &serviceName = serviceUnitName(service);
    if (serviceName.isEmpty()) {
        fmWarning() << "Invalid service name for status check:" << service;
        return false;
    }

    return dfmbase::SystemServiceManager::instance().isServiceRunning(serviceName);
}

bool enableServiceNow(const QString &service)
{
    const QString &serviceName = serviceUnitName(service);
    if (serviceName.isEmpty()) {
        fmWarning() << "Invalid service name for enable operation:" << service;
        return false;
    }

    fmDebug() << "Enable service:" << serviceName;
    bool result = dfmbase::SystemServiceManager::instance().enableServiceNow(serviceName);
    fmDebug() << "Service enable result for" << serviceName << ":" << result;
    return result;
}

bool checkAndEnableService(const QString &service)
{
    const QString &serviceName = serviceUnitName(service);
    if (serviceName.isEmpty()) {
        fmWarning() << "Invalid service name for check operation:" << service;
        return false;
    }

    if (!dfmbase::SystemServiceManager::instance().serviceExists(serviceName)) {
        fmWarning() << "Service unit does not exist:" << serviceName;
        return false;
    }

    if (isServiceRuning(service)) {
        fmDebug() << "Service already running:" << serviceName;
        return true;
    }

    fmDebug() << "Service not running, attempting to start:" << serviceName;
    if (enableServiceNow(service)) {
        fmDebug() << "Successfully started and enabled service:" << serviceName;
        return true;
    }

    fmCritical() << "Failed to start service:" << serviceName;
    return false;
}

bool ensureNetworkDiscoveryService(bool *serviceStarted)
{
    if (serviceStarted)
        *serviceStarted = false;

    static QMutex mutex;
    QMutexLocker locker(&mutex);

    const QString serviceName { kAvahiDaemonService };
    if (!dfmbase::SystemServiceManager::instance().serviceExists(serviceName)) {
        fmWarning() << "Network discovery service unit does not exist:" << serviceName;
        return false;
    }

    if (isServiceRuning(serviceName)) {
        fmDebug() << "Network discovery service already running:" << serviceName;
        return true;
    }

    if (!enableServiceNow(serviceName)) {
        fmCritical() << "Failed to start network discovery service:" << serviceName;
        return false;
    }

    if (serviceStarted)
        *serviceStarted = true;
    return true;
}

void bindSetting()
{
    static constexpr char kShowOfflineKey[] { "dfm.samba.permanent" };
    DFMBASE_USE_NAMESPACE
    SettingBackend::instance()->addSettingAccessor(
            "10_advance.01_mount.03_always_show_offline_remote_connection",
            [] { return DConfigManager::instance()->value(kDefaultCfgPath, kShowOfflineKey); },
            [](const QVariant &var) { DConfigManager::instance()->setValue(kDefaultCfgPath, kShowOfflineKey, var); });
}

QMutex &nodesMutex()
{
    static QMutex mutex;
    return mutex;
}

QMap<QUrl, SmbShareNode> &shareNodes()
{
    static QMap<QUrl, SmbShareNode> nodes;
    return nodes;
}

void initSettingPane()
{
    dfmbase::SettingJsonGenerator::instance()->addCheckBoxConfig("10_advance.01_mount.03_always_show_offline_remote_connection",
                                                                 QObject::tr("Keep showing the mounted Samba shares"));
}

}
}
