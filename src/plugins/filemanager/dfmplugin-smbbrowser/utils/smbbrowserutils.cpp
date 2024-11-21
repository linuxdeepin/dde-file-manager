// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
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
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/dfm_global_defines.h>

#include <dfm-framework/dpf.h>

#include <QUrl>
#include <QDBusInterface>
#include <QDBusPendingCall>

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

    fmDebug() << "cannot find matched device id of" << stdSmb;
    return stdSmb;
}

/*!
 * \brief isServiceRuning
 * \param service: options { "smb" | "nmb" }
 * \return
 */
bool isServiceRuning(const QString &service)
{
    if (service.isEmpty() || (service != "smb" && service != "nmb"))
        return false;

    QDBusInterface iface("org.freedesktop.systemd1",
                         QString("/org/freedesktop/systemd1/unit/%1d_2eservice").arg(service),
                         "org.freedesktop.systemd1.Unit",
                         QDBusConnection::systemBus());

    if (iface.isValid()) {
        const QVariant &variantStatus = iface.property("SubState");   // 获取属性 SubState，等同于 systemctl status smbd 结果 Active 值
        if (variantStatus.isValid())
            return "running" == variantStatus.toString();
    }
    return false;
}

/*!
 * \brief startService
 * \param service: options { "smb" | "nmb" }
 * \return
 */
bool startService(const QString &service)
{
    if (service.isEmpty() || (service != "smb" && service != "nmb"))
        return false;

    fmDebug() << QString("activate smbd: construct %1d interface").arg(service);
    QDBusInterface iface("org.freedesktop.systemd1",
                         QString("/org/freedesktop/systemd1/unit/%1d_2eservice").arg(service),
                         "org.freedesktop.systemd1.Unit",
                         QDBusConnection::systemBus());
    fmDebug() << QString("activate smbd: constructed %1d interface").arg(service);

    QDBusPendingCall call = iface.asyncCall("Start", "replace");
    call.waitForFinished();
    fmDebug() << QString("activate smbd: calling the %1d::Start method: ").arg(service) << call.isValid();
    return call.isValid();
}

void enableServiceAsync()
{
    QDBusInterface iface("org.deepin.Filemanager.UserShareManager",
                         "/org/deepin/Filemanager/UserShareManager",
                         "org.deepin.Filemanager.UserShareManager",
                         QDBusConnection::systemBus());
    iface.asyncCall("EnableSmbServices");
}

bool checkAndEnableService(const QString &service)
{
    if (isServiceRuning(service))
        return true;
    if (startService(service)) {
        enableServiceAsync();
        return true;
    }
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
