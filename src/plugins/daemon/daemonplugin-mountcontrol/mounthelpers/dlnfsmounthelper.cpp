// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dlnfsmounthelper.h"

#include <QDebug>
#include <QProcess>
#include <QStandardPaths>

#include <libmount/libmount.h>

DAEMONPMOUNTCONTROL_USE_NAMESPACE

QVariantMap DlnfsMountHelper::mount(const QString &path, const QVariantMap &opts)
{
    Q_UNUSED(opts)   // not used yet and might be used in the future.
    using namespace MountReturnField;

    static constexpr char kDlnfs[] { "dlnfs" };

    // 1. check if dlnfs at `path` is already exist.
    if (checkDlnfsExist(path))
        return { { kResult, true }, { kErrorCode, -kDlnMountMounted } };

    // 2. check `dlnfs` process exist.
    if (QStandardPaths::findExecutable(kDlnfs).isEmpty())
        return { { kResult, false }, { kErrorCode, -kDlnFsProcessNotExists } };

    // 3. mount dlnfs on `path`
    QStringList args { "-o",
                       "atomic_o_trunc,nonempty,use_ino,attr_timeout=0,entry_timeout=0,negative_timeout=0,allow_other,default_permissions",
                       path,
                       "--base",
                       path };
    QProcess p;
    int re = p.execute(kDlnfs, args);
    auto rets = p.readAllStandardOutput();
    qDebug() << "dlnfs: mount: " << rets;

    return { { kResult, re == 0 }, { kErrorCode, re } };
}

QVariantMap DlnfsMountHelper::unmount(const QString &path, const QVariantMap &opts)
{
    Q_UNUSED(opts)
    using namespace MountReturnField;

    static constexpr char kFusermount[] { "fusermount" };
    // 1. check if dlnfs is already mounted at `path`
    if (!checkDlnfsExist(path)) {
        qDebug() << "dlnfs: is not mounted at" << path;
        return { { kResult, true }, { kErrorCode, -kMountNotExist } };
    }

    // 2. check `fusermount` process exists.
    if (QStandardPaths::findExecutable(kFusermount).isEmpty()) {
        qWarning() << "dlnfs: fusermount do not exist";
        return { { kResult, false }, { kErrorCode, -kFusermountProcessNotExists } };
    }

    // 3. do unmount dlnfs
    QStringList args { "-u", path };
    QProcess p;
    int re = p.execute(kFusermount, args);
    auto rets = p.readAllStandardOutput();
    qDebug() << "dlnfs: unmount: " << rets;
    return { { kResult, re == 0 }, { kErrorCode, re } };
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
    qDebug() << "parse mtab: " << ret;

    std::string aPath = path.toStdString();
    auto fs = mnt_table_find_target(tab, aPath.c_str(), MNT_ITER_BACKWARD);
    if (!fs)
        return false;

    QString fsType = mnt_fs_get_fstype(fs);
    return fsType == "fuse.dlnfs";
}
