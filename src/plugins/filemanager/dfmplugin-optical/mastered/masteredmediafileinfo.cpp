// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "masteredmediafileinfo.h"
#include "masteredmediafileinfo_p.h"

#include "utils/opticalhelper.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/device/deviceproxymanager.h"
#include "dfm-base/base/device/deviceutils.h"
#include "dfm-base/interfaces/private/fileinfo_p.h"
#include "dfm-base/dbusservice/global_server_defines.h"

DFMBASE_USE_NAMESPACE

namespace dfmplugin_optical {

using namespace GlobalServerDefines;

MasteredMediaFileInfo::MasteredMediaFileInfo(const QUrl &url)
    : FileInfo(url), d(new MasteredMediaFileInfoPrivate(url, this))
{
    dptr.reset(d);
    d->backupInfo(url);
}

bool MasteredMediaFileInfo::exists() const
{
    if (d->url.isEmpty()
        || !d->backerUrl.isValid()
        || d->backerUrl.isEmpty()) {
        return false;
    }
    if (d->url.fragment() == "dup") {
        return false;
    }

    return d->proxy && d->proxy->exists();
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

        if (!d->proxy)
            return "";
        return d->proxy->displayOf(DisPlayInfoType::kFileDisplayName);
    }
    return FileInfo::displayOf(type);
}

QString MasteredMediaFileInfo::nameOf(const NameInfoType type) const
{
    switch (type) {
    case NameInfoType::kFileCopyName:
        return MasteredMediaFileInfo::displayOf(DisPlayInfoType::kFileDisplayName);
    default:
        return FileInfo::nameOf(type);
    }
}

QUrl MasteredMediaFileInfo::urlOf(const UrlInfoType type) const
{
    switch (type) {
    case FileUrlInfoType::kRedirectedFileUrl:
        if (d->proxy) {
            return d->proxy->urlOf(UrlInfoType::kUrl);
        }
        return FileInfo::urlOf(UrlInfoType::kUrl);
    case FileUrlInfoType::kParentUrl:
        return d->parentUrl();
    default:
        return FileInfo::urlOf(type);
    }
}

bool MasteredMediaFileInfo::isAttributes(const OptInfoType type) const
{
    switch (type) {
    case FileIsType::kIsDir:
        return !d->proxy || d->proxy->isAttributes(type);
    case FileIsType::kIsReadable:
        if (!d->proxy)
            return true;

        return d->proxy->isAttributes(type);
    case FileIsType::kIsWritable:
        if (!d->proxy)
            return false;
        return d->proxy->isAttributes(type);
    default:
        return FileInfo::isAttributes(type);
    }
}

QVariantHash MasteredMediaFileInfo::extraProperties() const
{
    QVariantHash ret;
    if (d->proxy) {
        ret = d->proxy->extraProperties();
    }
    ret["mm_backer"] = d->backerUrl.path();
    return ret;
}

void MasteredMediaFileInfo::refresh()
{
    FileInfo::refresh();
    if (d->proxy) {
        return;
    }

    d->backupInfo(urlOf(UrlInfoType::kUrl));
}

bool MasteredMediaFileInfo::canAttributes(const CanableInfoType type) const
{
    switch (type) {
    case FileCanType::kCanRename:
        if (d->proxy)
            return d->proxy->canAttributes(type);

        return false;
    case FileCanType::kCanRedirectionFileUrl:
        return dptr->proxy;
    case FileCanType::kCanDrop:
        return d->canDrop();
    case FileCanType::kCanDragCompress:
        [[fallthrough]];
    case FileCanType::kCanHidden:
        return false;
    default:
        return FileInfo::canAttributes(type);
    }
}

Qt::DropActions MasteredMediaFileInfo::supportedOfAttributes(const SupportedType type) const
{
    if (type == SupportType::kDrop)
        if (!OpticalHelper::isBurnEnabled())
            return Qt::IgnoreAction;
    return FileInfo::supportedOfAttributes(type);
}

QString MasteredMediaFileInfo::viewOfTip(const ViewInfoType type) const
{
    if (type == ViewType::kEmptyDir)
        return QObject::tr("Folder is empty");
    return FileInfo::viewOfTip(type);
}

MasteredMediaFileInfoPrivate::MasteredMediaFileInfoPrivate(const QUrl &url, MasteredMediaFileInfo *qq)
    : FileInfoPrivate(url, qq)
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
    proxy = InfoFactory::create<FileInfo>(backerUrl);
}

QUrl MasteredMediaFileInfoPrivate::parentUrl() const
{
    QString burnPath { OpticalHelper::burnFilePath(url) };
    if (burnPath.contains(QRegularExpression("^(/*)$"))) {
        return QUrl::fromLocalFile(QDir::homePath());
    }
    return UrlRoute::urlParent(url);
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
