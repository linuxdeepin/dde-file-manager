// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "smbsharefileinfo.h"
#include "private/smbsharefileinfo_p.h"
#include "utils/smbbrowserutils.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/mimetype/mimetypedisplaymanager.h>

using namespace dfmplugin_smbbrowser;
DFMBASE_USE_NAMESPACE

SmbShareFileInfo::SmbShareFileInfo(const QUrl &url)
    : FileInfo(url), d(new SmbShareFileInfoPrivate(this))
{
}

SmbShareFileInfo::~SmbShareFileInfo()
{
}

QString SmbShareFileInfo::nameOf(const NameInfoType type) const
{
    switch (type) {
    case NameInfoType::kFileName:
        [[fallthrough]];
    case NameInfoType::kFileCopyName:
        return d->fileName();
    default:
        return FileInfo::nameOf(type);
    }
}

QString SmbShareFileInfo::displayOf(const DisPlayInfoType type) const
{
    bool isNetworkRoot = url.scheme() == Global::Scheme::kNetwork
            && url.path() == "/";
    bool isSmbRoot = url.scheme() == Global::Scheme::kSmb
            && url.path().isEmpty();

    if (DisPlayInfoType::kFileDisplayName == type) {
        if (isNetworkRoot)
            return QObject::tr("Computers in LAN");

        if (isSmbRoot)
            return url.host();

        return d->fileName();
    }

    if (DisPlayInfoType::kMimeTypeDisplayName == type)
        return MimeTypeDisplayManager::instance()->displayName("inode/directory");

    return FileInfo::displayOf(type);
}

QIcon SmbShareFileInfo::fileIcon()
{
    bool isNetworkRoot = url.scheme() == Global::Scheme::kNetwork
            && url.path() == "/";
    bool isSmbRoot = url.scheme() == Global::Scheme::kSmb
            && (url.path().isEmpty() || url.path() == "/");

    if (isNetworkRoot)
        return QIcon::fromTheme("network-workgroup");
    if (isSmbRoot)
        return QIcon::fromTheme("network-server");

    QIcon icon;
    d->checkAndUpdateNode();
    icon = QIcon::fromTheme(d->node.iconType);

    if (!icon.isNull())
        return icon;

    return QIcon::fromTheme("unknown");
}

bool SmbShareFileInfo::exists() const
{
    return true;
}

bool SmbShareFileInfo::isAttributes(const OptInfoType type) const
{
    switch (type) {
    case FileIsType::kIsDir:
        [[fallthrough]];
    case FileIsType::kIsReadable:
        [[fallthrough]];
    case FileIsType::kIsWritable:
        return true;
    default:
        return FileInfo::isAttributes(type);
    }
}

bool SmbShareFileInfo::canAttributes(const CanableInfoType type) const
{
    switch (type) {
    case FileCanType::kCanDrag:
        return false;
    case FileCanType::kCanDrop:
        return d->canDrop() ? FileInfo::canAttributes(FileCanType::kCanDrop) : false;
    default:
        return FileInfo::canAttributes(type);
    }
}

SmbShareFileInfoPrivate::SmbShareFileInfoPrivate(SmbShareFileInfo *qq)
    : q(qq)
{
}

SmbShareFileInfoPrivate::~SmbShareFileInfoPrivate()
{
}

bool SmbShareFileInfoPrivate::canDrop() const
{
    if (UniversalUtils::urlEquals(q->url, smb_browser_utils::netNeighborRootUrl()))
        return false;
    if (!smb_browser_utils::isSmbMounted(q->url.toString()))
        return false;
    return true;
}

void SmbShareFileInfoPrivate::checkAndUpdateNode()
{
    if (node.url.isEmpty()) {
        QMutexLocker locker(&smb_browser_utils::nodesMutex());
        node = smb_browser_utils::shareNodes().value(q->fileUrl());
    }
}

QString SmbShareFileInfoPrivate::fileName()
{
    checkAndUpdateNode();
    return node.displayName;
}
