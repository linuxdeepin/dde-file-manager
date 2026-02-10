// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mountcontroldbus.h"
#include "private/mountcontroldbus_p.h"
#include "mounthelpers/cifsmounthelper.h"
#include "mounthelpers/commonmounthelper.h"
#include "service_mountcontrol_global.h"

#include <QFile>

#include <DConfig>

static constexpr char kMountControlObjPath[] { "/org/deepin/Filemanager/MountControl" };

namespace service_mountcontrol {
DFM_LOG_REGISTER_CATEGORY(SERVICEMOUNTCONTROL_NAMESPACE)
}

SERVICEMOUNTCONTROL_USE_NAMESPACE

MountControlDBus::MountControlDBus(const char *name, QObject *parent)
    : QObject(parent), QDBusContext(), d(new MountControlDBusPrivate(this))
{
    // NOTE: A PROBLEM cannot be resolved in short time.
    // if samba is mounted via CIFS and network disconnected between server and client
    // any invokation on the mounted samba file will be blocked forever.
    QFile switcher("/etc/deepin/disable_dfm_daemon_mount");
    if (switcher.exists())   // if the config file exist then DISABLE daemon mount.
        return;

    QDBusConnection::RegisterOptions opts =
            QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties;

    QDBusConnection::connectToBus(QDBusConnection::SystemBus, QString(name)).registerObject(kMountControlObjPath, this, opts);
}

MountControlDBus::~MountControlDBus() { }

QVariantMap MountControlDBus::Mount(const QString &path, const QVariantMap &opts)
{
    using namespace MountOptionsField;
    using namespace MountReturnField;

    auto fs = opts.value(kFsType, "").toString();
    if (fs.isEmpty())
        return { { kResult, false },
                 { kErrorCode, -kNoFsTypeSpecified },
                 { kErrorMessage, "fsType field must be specified." } };

    auto helper = d->mountHelpers.value(fs, nullptr);
    if (helper) {
        if (!helper->checkAuthentication(message().service())) {
            fmWarning() << "Mount: Authentication failed for path:" << path;
            return { { kResult, false },
                    { kErrorCode, -kAuthenticationFailed },
                    { kErrorMessage, "Authentication failed: insufficient privileges to mount" } };
        }

        return helper->mount(path, opts);
    }
    else
        return { { kResult, false },
                 { kErrorCode, -kUnsupportedFsTypeOrProtocol },
                 { kErrorMessage, "current fsType is not supported" } };
}

QVariantMap MountControlDBus::Unmount(const QString &path, const QVariantMap &opts)
{
    using namespace MountOptionsField;
    using namespace MountReturnField;

    auto fs = opts.value(kFsType, "").toString();
    if (fs.isEmpty())
        return { { kResult, false },
                 { kErrorCode, -kNoFsTypeSpecified },
                 { kErrorMessage, "fsType field must be specified." } };

    auto helper = d->mountHelpers.value(fs, nullptr);
    if (helper) {
        if (!helper->checkAuthentication(message().service())) {
            fmWarning() << "Unmount: Authentication failed for path:" << path;
            return { { kResult, false },
                    { kErrorCode, -kAuthenticationFailed },
                    { kErrorMessage, "Authentication failed: insufficient privileges to unmount" } };
        }

        return helper->unmount(path, opts);
    }
    else
        return { { kResult, false },
                 { kErrorCode, -kUnsupportedFsTypeOrProtocol },
                 { kErrorMessage, "current fsType is not supported" } };
}

QStringList MountControlDBus::SupportedFileSystems()
{
    // handle method call com.deepin.filemanager.daemon.MountControl.SupportedFileSystems
    return d->supportedFS;
}

MountControlDBusPrivate::MountControlDBusPrivate(MountControlDBus *qq)
    : q(qq), adapter(new MountControlAdaptor(qq))
{
    CifsMountHelper *cifsHelper = new CifsMountHelper(qq);
    CommonMountHelper *commonHelper = new CommonMountHelper(qq);

    cifsHelper->cleanMountPoint();

    auto config = Dtk::Core::DConfig::create("org.deepin.dde.file-manager",
                                             "org.deepin.dde.file-manager.mount", "", q);
    if (!config || !config->value("enableCifsMount").toBool()) {
        fmInfo() << "cannot create config object or cifs mount disabled." << config;
        return;
    }

    mountHelpers.insert(MountFstypeSupportedField::kCifs, cifsHelper);
    supportedFS.append(MountFstypeSupportedField::kCifs);
    mountHelpers.insert(MountFstypeSupportedField::kCommon, commonHelper);
}

MountControlDBusPrivate::~MountControlDBusPrivate()
{
    CifsMountHelper *helper = dynamic_cast<CifsMountHelper *>(mountHelpers.value(MountFstypeSupportedField::kCifs, nullptr));
    if (helper)
        helper->cleanMountPoint();

    const auto &keys = mountHelpers.keys();
    for (const auto &key : keys)
        delete mountHelpers[key];
    mountHelpers.clear();
}
