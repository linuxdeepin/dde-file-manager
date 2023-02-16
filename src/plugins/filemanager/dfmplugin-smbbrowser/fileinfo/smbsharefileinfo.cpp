// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "smbsharefileinfo.h"
#include "private/smbsharefileinfo_p.h"
#include "utils/smbbrowserutils.h"

#include "dfm-base/dfm_global_defines.h"

using namespace dfmplugin_smbbrowser;
DFMBASE_USE_NAMESPACE

SmbShareFileInfo::SmbShareFileInfo(const QUrl &url)
    : AbstractFileInfo(url), d(new SmbShareFileInfoPrivate(url, this))
{
    dptr.reset(d);
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
        return AbstractFileInfo::nameOf(type);
    }
}

QString SmbShareFileInfo::displayOf(const DisPlayInfoType type) const
{
    if (d->url.toString() == QString("%1:/").arg(Global::Scheme::kNetwork))
        return QObject::tr("Computers in LAN");
    if (DisPlayInfoType::kFileDisplayName == type)
        return d->fileName();
    return AbstractFileInfo::displayOf(type);
}

QIcon SmbShareFileInfo::fileIcon()
{
    auto dp = dynamic_cast<SmbShareFileInfoPrivate *>(dptr.data());
    return QIcon::fromTheme(dp->node.iconType);
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
        return AbstractFileInfo::isAttributes(type);
    }
}

bool SmbShareFileInfo::canAttributes(const CanableInfoType type) const
{
    switch (type) {
    case FileCanType::kCanDrag:
        return false;
    default:
        return AbstractFileInfo::canAttributes(type);
    }
}

SmbShareFileInfoPrivate::SmbShareFileInfoPrivate(const QUrl &url, AbstractFileInfo *qq)
    : AbstractFileInfoPrivate(url, qq)
{
    {
        QMutexLocker locker(&smb_browser_utils::nodesMutex());
        node = smb_browser_utils::shareNodes().value(url);
#if 0
        qDebug() << node;
#endif
    }
}

SmbShareFileInfoPrivate::~SmbShareFileInfoPrivate()
{
}

QString SmbShareFileInfoPrivate::fileName() const
{
    return node.displayName;
}
