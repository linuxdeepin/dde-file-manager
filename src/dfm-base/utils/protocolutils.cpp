// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
#include "protocolutils.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/deviceproxymanager.h>

DFMBASE_BEGIN_NAMESPACE

namespace ProtocolUtils {

static QString matchPath(const QUrl &url)
{
    return url.isLocalFile() ? url.toLocalFile() : url.path();
}

static int gvfsPathStart(const QString &path)
{
    static const QString kUserPrefix = QStringLiteral("/run/user/");
    static const QString kUserGvfsPrefix = QStringLiteral("/gvfs/");
    static const QString kRootGvfsPrefix = QStringLiteral("/root/.gvfs/");

    if (path.startsWith(kRootGvfsPrefix))
        return kRootGvfsPrefix.size();

    if (!path.startsWith(kUserPrefix))
        return -1;

    int pos = kUserPrefix.size();
    const int size = path.size();
    while (pos < size && path.at(pos).isDigit())
        ++pos;

    if (pos == kUserPrefix.size() || !path.mid(pos).startsWith(kUserGvfsPrefix))
        return -1;

    return pos + kUserGvfsPrefix.size();
}

static bool isGvfsPath(const QString &path)
{
    return gvfsPathStart(path) >= 0;
}

static bool isGvfsProtocolPath(const QString &path, const QString &protocolPrefix)
{
    const int start = gvfsPathStart(path);
    return start >= 0 && path.mid(start).startsWith(protocolPrefix);
}

static bool isMediaSmbMountPath(const QString &path)
{
    if (!path.startsWith(QStringLiteral("/media/")) && !path.startsWith(QStringLiteral("/run/media/")))
        return false;

    return path.contains(QStringLiteral("/smbmounts"));
}

bool isRemoteFile(const QUrl &url)
{
    if (!url.isValid())
        return false;

    // Fast path: known remote schemes
    const auto &scheme = url.scheme();
    static const QSet<QString> kRemoteSchemes {
        Global::Scheme::kSmb, Global::Scheme::kFtp, Global::Scheme::kSFtp,
        Global::Scheme::kGPhoto, Global::Scheme::kGPhoto2, Global::Scheme::kMtp,
        Global::Scheme::kAfc, Global::Scheme::kDav, Global::Scheme::kDavs,
        Global::Scheme::kNfs
    };
    if (kRemoteSchemes.contains(scheme))
        return true;

    const QString path = matchPath(url);
    return isGvfsPath(path) || isMediaSmbMountPath(path);
}

bool isMTPFile(const QUrl &url)
{
    if (!url.isValid())
        return false;

    if (url.scheme() == Global::Scheme::kMtp)
        return true;

    return isGvfsProtocolPath(matchPath(url), QStringLiteral("mtp:host"));
}

bool isGphotoFile(const QUrl &url)
{
    if (!url.isValid())
        return false;

    const auto &scheme = url.scheme();
    if (scheme == Global::Scheme::kGPhoto || scheme == Global::Scheme::kGPhoto2)
        return true;

    return isGvfsProtocolPath(matchPath(url), QStringLiteral("gphoto2:host"));
}

bool isFTPFile(const QUrl &url)
{
    if (!url.isValid())
        return false;
    if (url.scheme() == Global::Scheme::kFtp)
        return true;

    const QString path = matchPath(url);
    return isGvfsProtocolPath(path, QStringLiteral("ftp"))
            || isGvfsProtocolPath(path, QStringLiteral("sftp"));
}

bool isSFTPFile(const QUrl &url)
{
    if (!url.isValid())
        return false;
    if (url.scheme() == Global::Scheme::kSFtp)
        return true;

    return isGvfsProtocolPath(matchPath(url), QStringLiteral("sftp"));
}

bool isSMBFile(const QUrl &url)
{
    if (!url.isValid())
        return false;
    if (url.scheme() == Global::Scheme::kSmb)
        return true;
    // TODO(xust) smbmounts path might be changed in the future.
    const QString path = matchPath(url);
    return isGvfsProtocolPath(path, QStringLiteral("smb")) || isMediaSmbMountPath(path);
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

    if (url.scheme() == Global::Scheme::kNfs)
        return true;

    return isGvfsProtocolPath(matchPath(url), QStringLiteral("nfs"));
}

bool isDavFile(const QUrl &url)
{
    if (!url.isValid())
        return false;

    if (url.scheme() == Global::Scheme::kDav)
        return true;

    const QString path = matchPath(url);
    return isGvfsProtocolPath(path, QStringLiteral("dav:")) && path.contains(QStringLiteral("ssl=false"));
}

bool isDavsFile(const QUrl &url)
{
    if (!url.isValid())
        return false;

    if (url.scheme() == Global::Scheme::kDavs)
        return true;

    const QString path = matchPath(url);
    return isGvfsProtocolPath(path, QStringLiteral("dav:")) && path.contains(QStringLiteral("ssl=true"));
}

}   // namespace ProtocolUtils

DFMBASE_END_NAMESPACE
