// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "commonmounthelper.h"

#include <QDebug>

#include <libmount/libmount.h>

SERVICEMOUNTCONTROL_USE_NAMESPACE

QVariantMap CommonMountHelper::mount(const QString &path, const QVariantMap &opts)
{
    return {
        { MountReturnField::kResult, false },
        { MountReturnField::kErrorMessage, "function is not implement" }
    };
}

QVariantMap CommonMountHelper::unmount(const QString &path, const QVariantMap &opts)
{
    if (path.isEmpty()) {
        return { { MountReturnField::kResult, true },
                 { MountReturnField::kErrorMessage, "no need to unmount empty path" } };
    }

    bool unmountAll = opts.value(MountOptionsField::kUnmountAllStacked, false).toBool();
    // if unmountAll then unmount all paths which start with `path`.

    libmnt_table *tab { mnt_new_table() };
    libmnt_iter *iter { mnt_new_iter(MNT_ITER_BACKWARD) };

    int ret = mnt_table_parse_mtab(tab, nullptr);
    if (ret != 0) {
        mnt_free_table(tab);
        mnt_free_iter(iter);
        fmWarning() << "device: cannot parse mtab" << ret;
        return { { MountReturnField::kResult, false },
                 { MountReturnField::kErrorMessage, "cannot parse mtab" } };
    }

    QStringList unmountTargets;
    libmnt_fs *fs = nullptr;
    while (mnt_table_next_fs(tab, iter, &fs) == 0) {
        if (!fs)
            continue;
        const QString &srcDev = mnt_fs_get_source(fs);
        if (srcDev.startsWith("/dev/"))   // do not unmount the block item.
            continue;

        const QString &descPath = mnt_fs_get_target(fs);
        const QString &stdPath = descPath.endsWith("/") ? descPath : descPath + "/";
        if (stdPath.startsWith(path))
            unmountTargets << descPath;
    }

    mnt_free_table(tab);
    mnt_free_iter(iter);

    fmInfo() << "unmounting sub mounts of " << path;
    fmInfo() << "unmount items: " << unmountTargets;

    for (const auto &target : unmountTargets) {
        int ret = ::umount(target.toStdString().c_str());
        if (ret != 0) {
            int code = errno;
            QString errMsg = strerror(code);
            return { { MountReturnField::kResult, false },
                     { MountReturnField::kErrorCode, code },
                     { MountReturnField::kErrorMessage, errMsg + target } };
        }
        if (!unmountAll)
            break;
    }

    return { { MountReturnField::kResult, true },
             { MountReturnField::kErrorCode, 0 } };
}
