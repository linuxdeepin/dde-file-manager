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
#include <QtConcurrent>

Q_DECLARE_METATYPE(const char *)
using namespace GlobalDConfDefines::ConfigPath;

namespace dfmplugin_smbbrowser {
namespace smb_browser_utils {

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
    if (service.isEmpty() || (service != "smb" && service != "nmb")) {
        fmWarning() << "Invalid service name for status check:" << service;
        return false;
    }

    // 使用 SystemServiceManager 检查服务状态
    QString serviceName = QString("%1d.service").arg(service);
    return dfmbase::SystemServiceManager::instance().isServiceRunning(serviceName);
}

bool enableServiceNow(const QString &service)
{
    if (service.isEmpty() || (service != "smb" && service != "nmb")) {
        fmWarning() << "Invalid service name for enable operation:" << service;
        return false;
    }
    fmDebug() << "Enable service:" << service;
    QString serviceName = QString("%1d.service").arg(service);
    bool result = dfmbase::SystemServiceManager::instance().enableServiceNow(serviceName);
    fmDebug() << "Service enable result for" << service << ":" << result;
    return result;
}

bool checkAndEnableService(const QString &service)
{
    if (isServiceRuning(service)) {
        fmDebug() << "Service already running:" << service;
        return true;
    }

    fmDebug() << "Service not running, attempting to start:" << service;
    if (enableServiceNow(service)) {
        fmDebug() << "Successfully started and enabled service:" << service;
        return true;
    }

    fmCritical() << "Failed to start service:" << service;
    return false;
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
