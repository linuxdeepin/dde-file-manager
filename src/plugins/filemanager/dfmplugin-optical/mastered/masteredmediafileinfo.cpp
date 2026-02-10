// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "masteredmediafileinfo.h"
#include "masteredmediafileinfo_p.h"

#include "utils/opticalhelper.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/interfaces/private/fileinfo_p.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/utils/universalutils.h>

DFMBASE_USE_NAMESPACE

namespace dfmplugin_optical {

using namespace GlobalServerDefines;

MasteredMediaFileInfo::MasteredMediaFileInfo(const QUrl &url)
    : ProxyFileInfo(url), d(new MasteredMediaFileInfoPrivate(this))
{
    d->backupInfo(url);
    setProxy(InfoFactory::create<FileInfo>(d->backerUrl));
}

MasteredMediaFileInfo::MasteredMediaFileInfo(const QUrl &url, const FileInfoPointer proxy)
    : ProxyFileInfo(url), d(new MasteredMediaFileInfoPrivate(this))
{
    assert(!proxy.isNull());
    d->backupInfo(url);
    setProxy(proxy);
}

bool MasteredMediaFileInfo::exists() const
{
    if (url.isEmpty())
        return false;

    if (!d->backerUrl.isValid() || d->backerUrl.isEmpty()) {
        const QString &dev { OpticalHelper::burnDestDevice(url) };
        const QUrl &discRoot { OpticalHelper::discRoot(dev) };
        const QUrl &stagingRoot { OpticalHelper::localStagingRoot() };
        if (UniversalUtils::urlEquals(url, discRoot) || UniversalUtils::urlEquals(url, stagingRoot))
            return true;
        return false;
    }

    return proxy && proxy->exists();
}

QString MasteredMediaFileInfo::displayOf(const DisPlayInfoType type) const
{
    if (DisPlayInfoType::kFileDisplayName == type) {
        if (OpticalHelper::burnFilePath(urlOf(UrlInfoType::kUrl)).contains(QRegularExpression("^(/*)$"))) {
            const auto &map { DevProxyMng->queryBlockInfo(d->curDevId) };
            QString idLabel { qvariant_cast<QString>(map[DeviceProperty::kIdLabel]) };
            if (idLabel.isEmpty())
                idLabel = DeviceUtils::convertSuitableDisplayName(map);
            return idLabel;
        }

        if (!proxy)
            return "";
        return proxy->displayOf(DisPlayInfoType::kFileDisplayName);
    }
    return ProxyFileInfo::displayOf(type);
}

QString MasteredMediaFileInfo::nameOf(const NameInfoType type) const
{
    switch (type) {
    case NameInfoType::kFileCopyName:
        return MasteredMediaFileInfo::displayOf(DisPlayInfoType::kFileDisplayName);
    default:
        return ProxyFileInfo::nameOf(type);
    }
}

QUrl MasteredMediaFileInfo::urlOf(const UrlInfoType type) const
{
    switch (type) {
    case FileUrlInfoType::kRedirectedFileUrl:
        if (proxy) {
            return proxy->urlOf(UrlInfoType::kUrl);
        }
        return ProxyFileInfo::urlOf(UrlInfoType::kUrl);
    case FileUrlInfoType::kParentUrl:
        return d->parentUrl();
    case FileUrlInfoType::kUrl:
        return url;
    default:
        return ProxyFileInfo::urlOf(type);
    }
}

bool MasteredMediaFileInfo::isAttributes(const OptInfoType type) const
{
    switch (type) {
    case FileIsType::kIsDir:
        return !proxy || proxy->isAttributes(type);
    case FileIsType::kIsReadable:
        if (!proxy)
            return true;

        return proxy->isAttributes(type);
    case FileIsType::kIsWritable:
        if (!proxy)
            return false;
        return proxy->isAttributes(type);
    default:
        return ProxyFileInfo::isAttributes(type);
    }
}

QVariantHash MasteredMediaFileInfo::extraProperties() const
{
    QVariantHash ret;
    if (proxy) {
        ret = proxy->extraProperties();
    }
    ret["mm_backer"] = d->backerUrl.path();
    return ret;
}

void MasteredMediaFileInfo::refresh()
{
    ProxyFileInfo::refresh();
    if (proxy) {
        return;
    }

    d->backupInfo(urlOf(UrlInfoType::kUrl));
    setProxy(InfoFactory::create<FileInfo>(d->backerUrl));
}

void MasteredMediaFileInfo::updateAttributes(const QList<FileInfo::FileInfoAttributeID> &types)
{
    ProxyFileInfo::updateAttributes(types);
    if (proxy) {
        return;
    }

    d->backupInfo(urlOf(UrlInfoType::kUrl));
    setProxy(InfoFactory::create<FileInfo>(d->backerUrl));
}

bool MasteredMediaFileInfo::canAttributes(const CanableInfoType type) const
{
    switch (type) {
    case FileCanType::kCanRename:
        if (proxy)
            return proxy->canAttributes(type);

        return false;
    case FileCanType::kCanRedirectionFileUrl:
        return !proxy.isNull();
    case FileCanType::kCanDrop:
        return d->canDrop();
    case FileCanType::kCanDragCompress:
        [[fallthrough]];
    case FileCanType::kCanHidden:
        return false;
    default:
        return ProxyFileInfo::canAttributes(type);
    }
}

Qt::DropActions MasteredMediaFileInfo::supportedOfAttributes(const SupportedType type) const
{
    if (type == SupportType::kDrop) {
        if (!OpticalHelper::isBurnEnabled())
            return Qt::IgnoreAction;

        if (isAttributes(OptInfoType::kIsWritable)) {
            return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
        }

        if (d->canDrop()) {
            return Qt::CopyAction | Qt::MoveAction;
        }
        return Qt::IgnoreAction;
    }
    return ProxyFileInfo::supportedOfAttributes(type);
}

QString MasteredMediaFileInfo::viewOfTip(const ViewInfoType type) const
{
    if (type == ViewType::kEmptyDir)
        return QObject::tr("Folder is empty");
    return ProxyFileInfo::viewOfTip(type);
}

MasteredMediaFileInfoPrivate::MasteredMediaFileInfoPrivate(MasteredMediaFileInfo *qq)
    : q(qq)
{
}

MasteredMediaFileInfoPrivate::~MasteredMediaFileInfoPrivate()
{
}

void MasteredMediaFileInfoPrivate::backupInfo(const QUrl &url)
{
    if (OpticalHelper::burnDestDevice(url).length() == 0)
        return;

    if (OpticalHelper::burnIsOnDisc(url)) {
        QString &&devFile { OpticalHelper::burnDestDevice(url) };
        QString &&mnt { DeviceUtils::getMountInfo(devFile) };
        curDevId = { DeviceUtils::getBlockDeviceId(OpticalHelper::burnDestDevice(url)) };
        if (mnt.isEmpty())
            return;
        backerUrl = QUrl::fromLocalFile(mnt + OpticalHelper::burnFilePath(url));
    } else {
        backerUrl = OpticalHelper::localStagingFile(url);
    }
}

QUrl MasteredMediaFileInfoPrivate::parentUrl() const
{
    QString burnPath { OpticalHelper::burnFilePath(q->fileUrl()) };
    if (burnPath.contains(QRegularExpression("^(/*)$"))) {
        return QUrl::fromLocalFile(QDir::homePath());
    }
    return UrlRoute::urlParent(q->fileUrl());
}

bool MasteredMediaFileInfoPrivate::canDrop()
{
    if (!OpticalHelper::burnIsOnDisc(backerUrl))
        return true;
    const auto &map { DevProxyMng->queryBlockInfo(curDevId) };
    quint64 avil { qvariant_cast<quint64>(map[DeviceProperty::kSizeFree]) };
    return avil > 0;
}

}
