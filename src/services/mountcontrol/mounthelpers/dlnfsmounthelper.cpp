// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dlnfsmounthelper.h"

#include <QDebug>
#include <QProcess>
#include <QStandardPaths>

#include <libmount/libmount.h>

SERVICEMOUNTCONTROL_USE_NAMESPACE

QVariantMap DlnfsMountHelper::mount(const QString &path, const QVariantMap &opts)
{
    Q_UNUSED(opts)   // not used yet and might be used in the future.
    using namespace MountReturnField;

    static constexpr char kDlnfs[] { "dlnfs" };

    // 1. check if dlnfs at `path` is already exist.
    if (checkDlnfsExist(path))
        return { { kResult, true },
                 { kErrorCode, -kDlnMountMounted },
                 { kErrorMessage, QString("dlnfs is already mounted at %1").arg(path) } };

    // 2. check `dlnfs` process exist.
    if (QStandardPaths::findExecutable(kDlnfs).isEmpty())
        return { { kResult, false },
                 { kErrorCode, -kDlnFsProcessNotExists },
                 { kErrorMessage, "dlnfs do not exist" } };

    // 3. mount dlnfs on `path`
    QStringList args { "-o",
                       "atomic_o_trunc,nonempty,use_ino,attr_timeout=0,entry_timeout=0,negative_timeout=0,allow_other,default_permissions",
                       path,
                       "--base",
                       path };

    QProcess p;
    p.start(kDlnfs, args);
    p.waitForFinished();
    auto rets = p.readAllStandardError();
    fmInfo() << "dlnfs: mount result: " << rets;
    return { { kResult, rets.isEmpty() },
             { kErrorMessage, QString(rets) },
             { kErrorCode, parseErrorCodeByMsg(rets) } };
}

QVariantMap DlnfsMountHelper::unmount(const QString &path, const QVariantMap &opts)
{
    Q_UNUSED(opts)
    using namespace MountReturnField;

    static constexpr char kFusermount[] { "fusermount" };
    // 1. check if dlnfs is already mounted at `path`
    if (!checkDlnfsExist(path)) {
        fmDebug() << "dlnfs: is not mounted at" << path;
        return { { kResult, true },
                 { kErrorCode, -kMountNotExist },
                 { kErrorMessage, QString("dlnfs is not mounted at %1").arg(path) } };
    }

    // 2. check `fusermount` process exists.
    if (QStandardPaths::findExecutable(kFusermount).isEmpty()) {
        fmWarning() << "dlnfs: fusermount do not exist";
        return { { kResult, false },
                 { kErrorCode, -kFusermountProcessNotExists },
                 { kErrorMessage, "fusermount do not exist" } };
    }

    // 3. do unmount dlnfs
    QStringList args { "-u", path };
    QProcess p;
    p.start(kFusermount, args);
    p.waitForFinished();
    auto rets = p.readAllStandardError();
    fmInfo() << "dlnfs: unmount result: " << rets;
    return { { kResult, rets.isEmpty() },
             { kErrorMessage, QString(rets) },
             { kErrorCode, parseErrorCodeByMsg(rets) } };
}

bool DlnfsMountHelper::checkDlnfsExist(const QString &path)
{
    class Helper
    {
    public:
        Helper() { tab = mnt_new_table(); }
        ~Helper() { mnt_free_table(tab); }
        libmnt_table *tab { nullptr };
    };

    Helper d;
    auto tab = d.tab;
    int ret = mnt_table_parse_mtab(tab, nullptr);
    fmDebug() << "parse mtab: " << ret;

    std::string aPath = path.toStdString();
    auto fs = mnt_table_find_target(tab, aPath.c_str(), MNT_ITER_BACKWARD);
    if (!fs)
        return false;

    QString fsType = mnt_fs_get_fstype(fs);
    return fsType == "fuse.dlnfs";
}

int DlnfsMountHelper::parseErrorCodeByMsg(const QString &msg)
{
    if (msg.isEmpty())
        return 0;
    if (msg.contains("Device or resource busy"))
        return EBUSY;
    if (msg.contains("the underlying file system does not support"))
        return -kUnsupportedFsTypeOrProtocol;
    return -kUnhandledError;
}
