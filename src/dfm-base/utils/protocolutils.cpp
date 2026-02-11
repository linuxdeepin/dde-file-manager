// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
#include "protocolutils.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/deviceproxymanager.h>

DFMBASE_BEGIN_NAMESPACE

namespace ProtocolUtils {

// Helper function: match with cached compiled regex
static bool hasMatch(const QString &txt, const QRegularExpression &re)
{
    return re.match(txt).hasMatch();
}

bool isRemoteFile(const QUrl &url)
{
    if (!url.isValid())
        return false;

    // TODO(xust) smbmounts path might be changed in the future.
    static const QRegularExpression gvfsMatch { R"((^/run/user/\d+/gvfs/|^/root/.gvfs/|^/(?:run/)?media/[\s\S]*/smbmounts))" };
    return hasMatch(url.toLocalFile(), gvfsMatch);
}

bool isMTPFile(const QUrl &url)
{
    if (!url.isValid())
        return false;

    static const QRegularExpression gvfsMatch { R"(^/run/user/\d+/gvfs/mtp:host|^/root/.gvfs/mtp:host)" };
    return hasMatch(url.toLocalFile(), gvfsMatch);
}

bool isGphotoFile(const QUrl &url)
{
    if (!url.isValid())
        return false;

    static const QRegularExpression gvfsMatch { R"(^/run/user/\d+/gvfs/gphoto2:host|^/root/.gvfs/gphoto2:host)" };
    return hasMatch(url.toLocalFile(), gvfsMatch);
}

bool isFTPFile(const QUrl &url)
{
    if (!url.isValid())
        return false;

    static const QRegularExpression smbMatch { R"((^/run/user/\d+/gvfs/s?ftp|^/root/.gvfs/s?ftp))" };
    return hasMatch(url.path(), smbMatch);
}

bool isSFTPFile(const QUrl &url)
{
    if (!url.isValid())
        return false;

    static const QRegularExpression smbMatch { R"((^/run/user/\d+/gvfs/sftp|^/root/.gvfs/sftp))" };
    return hasMatch(url.path(), smbMatch);
}

bool isSMBFile(const QUrl &url)
{
    if (!url.isValid())
        return false;
    if (url.scheme() == Global::Scheme::kSmb)
        return true;
    // TODO(xust) smbmounts path might be changed in the future.
    static const QRegularExpression smbMatch { R"((^/run/user/\d+/gvfs/smb|^/root/.gvfs/smb|^/(?:run/)?media/[\s\S]*/smbmounts))" };
    return hasMatch(url.path(), smbMatch);
}

bool isLocalFile(const QUrl &url)
{
    Q_ASSERT(!url.scheme().isEmpty());

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

bool isNFSFile(const QUrl &url)
{
    if (!url.isValid())
        return false;

    static const QRegularExpression nfsMatch { R"((^/run/user/\d+/gvfs/nfs|^/root/.gvfs/nfs))" };
    return hasMatch(url.path(), nfsMatch);
}

bool isDavFile(const QUrl &url)
{
    if (!url.isValid())
        return false;

    static const QRegularExpression davMatch { R"((^/run/user/\d+/gvfs/dav.*ssl=false|^/root/.gvfs/dav.*ssl=false))" };
    return hasMatch(url.path(), davMatch);
}

bool isDavsFile(const QUrl &url)
{
    if (!url.isValid())
        return false;

    static const QRegularExpression davsMatch { R"((^/run/user/\d+/gvfs/dav.*ssl=true|^/root/.gvfs/dav.*ssl=true))" };
    return hasMatch(url.path(), davsMatch);
}

}   // namespace ProtocolUtils

DFMBASE_END_NAMESPACE
