// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
#include "protocolutils.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/deviceproxymanager.h>

DFMBASE_BEGIN_NAMESPACE

namespace ProtocolUtils {

static bool hasMatch(const QString &txt, const QString &rex)
{
    QRegularExpression re(rex);
    QRegularExpressionMatch match = re.match(txt);
    return match.hasMatch();
}

bool isRemoteFile(const QUrl &url)
{
    if (!url.isValid())
        return false;

    // TODO(xust) /media/$USER/smbmounts might be changed in the future.
    static const QString gvfsMatch { "(^/run/user/\\d+/gvfs/|^/root/.gvfs/|^/media/[\\s\\S]*/smbmounts)" };
    return hasMatch(url.toLocalFile(), gvfsMatch);
}

bool isMTPFile(const QUrl &url)
{
    if (!url.isValid())
        return false;

    static const QString gvfsMatch { R"(^/run/user/\d+/gvfs/mtp:host|^/root/.gvfs/mtp:host)" };
    return hasMatch(url.toLocalFile(), gvfsMatch);
}

bool isGphotoFile(const QUrl &url)
{
    if (!url.isValid())
        return false;

    static const QString gvfsMatch { R"(^/run/user/\d+/gvfs/gphoto2:host|^/root/.gvfs/gphoto2:host)" };
    return hasMatch(url.toLocalFile(), gvfsMatch);
}

bool isFTPFile(const QUrl &url)
{
    if (!url.isValid())
        return false;

    static const QString smbMatch { "(^/run/user/\\d+/gvfs/s?ftp|^/root/\\.gvfs/s?ftp)" };
    return hasMatch(url.path(), smbMatch);
}

bool isSFTPFile(const QUrl &url)
{
    if (!url.isValid())
        return false;

    static const QString smbMatch { "(^/run/user/\\d+/gvfs/sftp|^/root/\\.gvfs/sftp)" };
    return hasMatch(url.path(), smbMatch);
}

bool isSMBFile(const QUrl &url)
{
    if (!url.isValid())
        return false;
    if (url.scheme() == Global::Scheme::kSmb)
        return true;
    // TODO(xust) /media/$USER/smbmounts might be changed in the future.}
    static const QString smbMatch { "(^/run/user/\\d+/gvfs/smb|^/root/\\.gvfs/smb|^/media/[\\s\\S]*/smbmounts)" };
    return hasMatch(url.path(), smbMatch);
}

bool isLocalFile(const QUrl &url)
{
    if (!url.isLocalFile())
        return false;
    if (isRemoteFile(url))
        return false;
    if (DevProxyMng->isFileOfExternalBlockMounts(url.path()))
        return false;
    if (DevProxyMng->isFileOfProtocolMounts(url.path()))
        return false;

    return true;
}

}   // namespace ProtocolUtils

DFMBASE_END_NAMESPACE
