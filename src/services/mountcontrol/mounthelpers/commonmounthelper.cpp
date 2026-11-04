// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "commonmounthelper.h"

#include <QDebug>
#include <QFileInfo>
#include <QRegularExpression>

#include <libmount/libmount.h>
#include <pwd.h>
#include <unistd.h>

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

// Check whether the caller is authorized to mount/unmount at `path`.
//
// Unmount access control:
//   The user-requested `path` must be canonical — no ".." segments or symlinks —
//   otherwise `path` cannot match any existing mount entry in the system.
//
// 1. path canonical check (pre-filter):
//      canonicalFilePath() must equal the input `path`. If they differ (or the
//      path does not exist) the bypass is rejected immediately (return false),
//      preventing a caller from spoofing /media/<other-user>/... via symlinks.
//
// 2. path form check:
//      `path` must be of the form /media/<user>/<dir>. The captured <user> is
//      resolved via getpwnam_r and its uid compared against callerID; only the
//      mount-point owner gets the polkit bypass.
bool CommonMountHelper::checkAuthentication(const QString &appName, const uint callerID, const bool doMount, const QString &path)
{
    // For unmount operations only, bypass polkit if the caller owns the mount point.
    if (!doMount && !path.isEmpty()) {
        // (1) Canonical pre-filter: reject paths that contain ".." or symlinks so
        //     that `path` can reliably match existing mount entries downstream.
        const QString &canonical = QFileInfo(path).canonicalFilePath();
        if (canonical.isEmpty() || canonical != path) {
            fmWarning() << "checkAuthentication: path is not canonical, reject:" << path << "->" << canonical;
            return false;
        }

        // (2) Form check: /media/<user>/<dir>, and the caller must be <user>.
        static const QRegularExpression re("^/media/([^/]+)/.+");
        QRegularExpressionMatch match = re.match(path);
        if (match.hasMatch()) {
            const QString &userName = match.captured(1);
            // getpwnam_r is reentrant/thread-safe; getpwnam is not.
            struct passwd pwd;
            struct passwd *result = nullptr;
            char buf[8192];
            int rc = getpwnam_r(userName.toLocal8Bit().constData(), &pwd, buf, sizeof(buf), &result);
            if (rc == 0 && result && pwd.pw_uid == callerID)
                return true;
        }
    }

    return AbstractMountHelper::checkAuthentication(appName, callerID, doMount, path);
}
