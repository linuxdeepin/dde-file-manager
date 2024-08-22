// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mountcontroldbus.h"
#include "private/mountcontroldbus_p.h"
#include "private/mountcontrol_adapter.h"
#include "mounthelpers/cifsmounthelper.h"
#include "mounthelpers/dlnfsmounthelper.h"
#include "mounthelpers/commonmounthelper.h"

#include <QFile>
#include <DConfig>

static constexpr char kMountControlObjPath[] { "/com/deepin/filemanager/daemon/MountControl" };

DAEMONPMOUNTCONTROL_USE_NAMESPACE

MountControlDBus::MountControlDBus(QObject *parent)
    : QObject(parent), QDBusContext(), d(new MountControlDBusPrivate(this))
{
    // NOTE: A PROBLEM cannot be resolved in short time.
    // if samba is mounted via CIFS and network disconnected between server and client
    // any invokation on the mounted samba file will be blocked forever.
    QFile switcher("/etc/deepin/disable_dfm_daemon_mount");
    if (switcher.exists())   // if the config file exist then DISABLE daemon mount.
        return;

    QDBusConnection::systemBus().registerObject(kMountControlObjPath, this);
}

MountControlDBus::~MountControlDBus() {}

QVariantMap MountControlDBus::Mount(const QString &path, const QVariantMap &opts)
{
    using namespace MountOptionsField;
    using namespace MountReturnField;
    auto fs = opts.value(kFsType, "").toString();
    if (fs.isEmpty())
        return { { kResult, false },
                 { kErrorCode, -kNoFsTypeSpecified },
                 { kErrorMessage, "fsType filed must be specified." } };

    auto helper = d->mountHelpers.value(fs, nullptr);
    if (helper)
        return helper->mount(path, opts);
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
                 { kErrorMessage, "fsType filed must be specified." } };

    auto helper = d->mountHelpers.value(fs, nullptr);
    if (helper)
        return helper->unmount(path, opts);
    else
        return { { kResult, false },
                 { kErrorCode, -kUnsupportedFsTypeOrProtocol },
                 { kErrorMessage, "current fsType is not supported" } };
}

QStringList MountControlDBus::SupportedFileSystems()
{
    return d->supportedFS;
}

MountControlDBusPrivate::MountControlDBusPrivate(MountControlDBus *qq)
    : q(qq), adapter(new MountControlAdapter(qq))
{
    CifsMountHelper *cifsHelper = new CifsMountHelper(qq);
    DlnfsMountHelper *dlnfsHelper = new DlnfsMountHelper(qq);
    CommonMountHelper *commonHelper = new CommonMountHelper(qq);

    cifsHelper->cleanMountPoint();
    mountHelpers.insert(MountFstypeSupportedField::kDlnFs, dlnfsHelper);
    supportedFS.append(MountFstypeSupportedField::kDlnFs);
    mountHelpers.insert(MountFstypeSupportedField::kCommon, commonHelper);

    auto config = Dtk::Core::DConfig::create("org.deepin.dde.file-manager",
                                             "org.deepin.dde.file-manager.mount");
    if (!config || !config->value("enableCifsMount").toBool()) {
        fmInfo() << "cannot create config object or cifs mount disabled." << config;
        return;
    }
    mountHelpers.insert(MountFstypeSupportedField::kCifs, cifsHelper);
    supportedFS.append(MountFstypeSupportedField::kCifs);
    config->deleteLater();
}

MountControlDBusPrivate::~MountControlDBusPrivate()
{
    if (adapter)
        delete adapter;

    CifsMountHelper *helper = dynamic_cast<CifsMountHelper *>(mountHelpers.value(MountFstypeSupportedField::kCifs, nullptr));
    if (helper)
        helper->cleanMountPoint();

    const auto &keys = mountHelpers.keys();
    for (const auto &key : keys)
        delete mountHelpers[key];
    mountHelpers.clear();
}
