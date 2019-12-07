/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *               2019 ~ 2019 Chris Xiong
 *
 * Author:     Chris Xiong<chirs241097@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dfmrootfileinfo.h"
#include "shutil/fileutils.h"
#include "app/define.h"
#include "utils/singleton.h"
#include "controllers/pathmanager.h"

#include <dgiofile.h>
#include <dgiofileinfo.h>
#include <dgiomount.h>
#include <dgiovolume.h>
#include <dgiovolumemanager.h>
#include <ddiskmanager.h>
#include <dblockdevice.h>
#include <ddiskdevice.h>

#include <QStandardPaths>
#include <QStorageInfo>

class DFMRootFileInfoPrivate
{
public:
    QStandardPaths::StandardLocation stdloc;
    QSharedPointer<DBlockDevice> blk;
    QSharedPointer<DBlockDevice> ctblk;
    QExplicitlySharedDataPointer<DGioMount> gmnt;
    QExplicitlySharedDataPointer<DGioFileInfo> gfsi;
    QString backer_url;
    QByteArrayList mps;
    qulonglong size;
    QString label;
    QString fs;
    QString udispname;
    bool isod;
    bool encrypted;
    DFMRootFileInfo *q_ptr;
    Q_DECLARE_PUBLIC(DFMRootFileInfo)
};

DFMRootFileInfo::DFMRootFileInfo(const DUrl &url) :
    DAbstractFileInfo(url),
    d_ptr(new DFMRootFileInfoPrivate)
{
    if (suffix() == SUFFIX_USRDIR) {
        QStandardPaths::StandardLocation loc = QStandardPaths::StandardLocation::HomeLocation;
        if (baseName() == "desktop") {
            loc = QStandardPaths::StandardLocation::DesktopLocation;
        } else if (baseName() == "videos") {
            loc = QStandardPaths::StandardLocation::MoviesLocation;
        } else if (baseName() == "music") {
            loc = QStandardPaths::StandardLocation::MusicLocation;
        } else if (baseName() == "pictures") {
            loc = QStandardPaths::StandardLocation::PicturesLocation;
        } else if (baseName() == "documents") {
            loc = QStandardPaths::StandardLocation::DocumentsLocation;
        } else if (baseName() == "downloads") {
            loc = QStandardPaths::StandardLocation::DownloadLocation;
        }

        d_ptr->stdloc = loc;
        if (QStandardPaths::writableLocation(loc) == QStandardPaths::writableLocation(QStandardPaths::StandardLocation::HomeLocation)) {
            d_ptr->backer_url = "";
        } else {
            FileUtils::mkpath(DUrl::fromLocalFile(QStandardPaths::writableLocation(loc)));
            d_ptr->backer_url = QStandardPaths::writableLocation(loc);
        }
    } else if (suffix() == SUFFIX_GVFSMP) {
        QString mpp = QUrl::fromPercentEncoding(fileUrl().path().mid(1).chopped(QString("." SUFFIX_GVFSMP).length()).toUtf8());
        QExplicitlySharedDataPointer<DGioMount> mp(DGioMount::createFromPath(mpp));
        if (mp && mp->getRootFile() && mp->getRootFile()->path() == mpp) {
            QExplicitlySharedDataPointer<DGioFileInfo> fi = mp->getRootFile()->createFileInfo("*", FILE_QUERY_INFO_NONE, 2000);
            if (fi && fi->fileType() == DGioFileType::FILE_TYPE_DIRECTORY) {
                QString mpurl = mp->getRootFile()->path();
                d_ptr->backer_url = mpurl;
                d_ptr->gmnt = mp;
                d_ptr->gfsi = mp->getRootFile()->createFileSystemInfo();
            }
        }
    } else if (suffix() == SUFFIX_UDISKS) {
        QStringList pathList = DDiskManager::resolveDeviceNode("/dev" + url.path().chopped(QString("." SUFFIX_UDISKS).length()), {});
        if (pathList.size()==0) {
            qWarning() << url << "not existed";
            return;
        }

        QString udiskspath = pathList.first();
        QSharedPointer<DBlockDevice> blk(DDiskManager::createBlockDevice(udiskspath));
        if (blk->path().length() != 0) {
            QSharedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(blk->drive()));
            d_ptr->isod = drv->mediaCompatibility().join(" ").contains("optical");
            d_ptr->backer_url = udiskspath;
            d_ptr->blk = blk;
            d_ptr->blk->setWatchChanges(true);
            checkCache();
            QObject::connect(d_ptr->blk.data(), &DBlockDevice::idLabelChanged, [this] {this->checkCache();});
            QObject::connect(d_ptr->blk.data(), &DBlockDevice::mountPointsChanged, [this] {this->checkCache();});
            QObject::connect(d_ptr->blk.data(), &DBlockDevice::sizeChanged, [this] {this->checkCache();});
            QObject::connect(d_ptr->blk.data(), &DBlockDevice::idTypeChanged, [this] {this->checkCache();});
            QObject::connect(d_ptr->blk.data(), &DBlockDevice::cleartextDeviceChanged, [this]{this->checkCache();});
        }
    }
}

bool DFMRootFileInfo::exists() const
{
    Q_D(const DFMRootFileInfo);
    if (suffix() == SUFFIX_USRDIR) {
        return d->backer_url.length() != 0;
    } else if (suffix() == SUFFIX_GVFSMP) {
        if (!d->gmnt) {
            return false;
        }
        QExplicitlySharedDataPointer<DGioFileInfo> fi = d->gmnt->getRootFile()->createFileInfo("*", FILE_QUERY_INFO_NONE, 2000);
        return fi && fi->fileType() == DGioFileType::FILE_TYPE_DIRECTORY;
    } else if (suffix() == SUFFIX_UDISKS) {
        return d->blk->path().length() != 0;
    }
}

QString DFMRootFileInfo::suffix() const
{
    QRegularExpression re(".*\\.(.*)$");
    auto rem = re.match(fileName());
    if (!rem.hasMatch()) {
        return "";
    }
    return rem.captured(1);
}

QString DFMRootFileInfo::fileDisplayName() const
{
    Q_D(const DFMRootFileInfo);

    if (suffix() == SUFFIX_USRDIR) {
        QString namekey = baseName();
        namekey[0] = namekey[0].toUpper();
        return systemPathManager->getSystemPathDisplayName(namekey);
    } else if (suffix() == SUFFIX_GVFSMP) {
        return d->gmnt ? d->gmnt->name() : "";
    } else if (suffix() == SUFFIX_UDISKS) {
        return d->udispname;
    }
    return baseName();
}

bool DFMRootFileInfo::canRename() const
{
    Q_D(const DFMRootFileInfo);
    if (suffix() != SUFFIX_UDISKS || !d->blk) {
        return false;
    }
    if (d->blk->readOnly() || d->mps.size() > 0) {
        return false;
    }
    return true;
}

bool DFMRootFileInfo::canShare() const
{
    return false;
}

bool DFMRootFileInfo::canFetch() const
{
    return false;
}

bool DFMRootFileInfo::isReadable() const
{
    return true;
}

bool DFMRootFileInfo::isWritable() const
{
    return false;
}

bool DFMRootFileInfo::isExecutable() const
{
    return false;
}

bool DFMRootFileInfo::isHidden() const
{
    return false;
}

bool DFMRootFileInfo::isRelative() const
{
    return false;
}

bool DFMRootFileInfo::isAbsolute() const
{
    return true;
}

bool DFMRootFileInfo::isShared() const
{
    return false;
}

bool DFMRootFileInfo::isTaged() const
{
    return false;
}

bool DFMRootFileInfo::isWritableShared() const
{
    return false;
}

bool DFMRootFileInfo::isAllowGuestShared() const
{
    return false;
}

bool DFMRootFileInfo::isFile() const
{
    return true;
}

bool DFMRootFileInfo::isDir() const
{
    return false;
}

DAbstractFileInfo::FileType DFMRootFileInfo::fileType() const
{
    Q_D(const DFMRootFileInfo);

    ItemType ret;

    if (suffix() == SUFFIX_USRDIR) {
        ret = ItemType::UserDirectory;
    } else if (suffix() == SUFFIX_GVFSMP) {
        ret = ItemType::GvfsGeneric;
        if (d->gmnt && d->gmnt->getRootFile()) {
            DUrl url(d->gmnt->getRootFile()->uri());
            if (url.scheme() == FTP_SCHEME) {
                ret = ItemType::GvfsFTP;
            } else if (url.scheme() == SMB_SCHEME) {
                ret = ItemType::GvfsSMB;
            } else if (url.scheme() == MTP_SCHEME) {
                ret = ItemType::GvfsMTP;
            } else if (url.scheme() == GPHOTO2_SCHEME) {
                ret = ItemType::GvfsGPhoto2;
            }
        }
    } else if (d->mps.contains(QByteArray("/\0", 2))) {
        ret = ItemType::UDisksRoot;
    } else if (d->label == "_dde_data") {
        ret = ItemType::UDisksData;
    } else {
        QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(d->blk->drive()));
        if (d->isod) {
            ret = ItemType::UDisksOptical;
        } else {
            ret = ItemType::UDisksFixed;
            if (drv->media() == "thumb" || drv->removable() || drv->mediaRemovable() || drv->ejectable()) {
                ret = ItemType::UDisksRemovable;
            }
        }
    }
    return static_cast<FileType>(ret);
}

int DFMRootFileInfo::filesCount() const
{
    return 0;
}

QString DFMRootFileInfo::iconName() const
{
    Q_D(const DFMRootFileInfo);
    if (suffix() == SUFFIX_USRDIR) {
        return systemPathManager->getSystemPathIconNameByPath(redirectedFileUrl().path());
    } else if (suffix() == SUFFIX_GVFSMP) {
        return d->gmnt ? d->gmnt->themedIconNames().front() : "";
    } else if (suffix() == SUFFIX_UDISKS) {
        if (d->blk) {
            QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(d->blk->drive()));
            if (drv->mediaCompatibility().join(" ").contains("optical")) {
                return "media-optical";
            }
            if (static_cast<ItemType>(fileType()) == ItemType::UDisksRemovable) {
                return QString("drive-removable-media") + (d->encrypted ? "-encrypted" : "");
            }
            if (d->mps.contains(QByteArray("/\0", 2))) {
                return "drive-harddisk-root";
            }
        }
        return QString("drive-harddisk") + (d->encrypted ? "-encrypted" : "");
    }
    return "";
}

QVector<MenuAction> DFMRootFileInfo::menuActionList(DAbstractFileInfo::MenuType type) const
{
    Q_D(const DFMRootFileInfo);
    bool protectUnmountOrEject = false;
    QVector<MenuAction> ret;
    if (suffix() == SUFFIX_USRDIR) {
        ret.push_back(MenuAction::OpenInNewWindow);
        ret.push_back(MenuAction::OpenInNewTab);
    } else {
        ret.push_back(MenuAction::OpenDiskInNewWindow);
        ret.push_back(MenuAction::OpenDiskInNewTab);
    }

    ret.push_back(MenuAction::Separator);

    QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(d->blk ? d->blk->drive() : ""));
    DBlockDevice *blk = d->ctblk ? d->ctblk.data() : d->blk.data();

    if (suffix() == SUFFIX_UDISKS && blk && !blk->mountPoints().empty()) {
        QList<QByteArray> mountPoints = blk->mountPoints();
        for (auto & mountPoint : mountPoints) {
            if (!mountPoint.startsWith("/media/")) {
                protectUnmountOrEject = true;
                break;
            }
        }

        if (!protectUnmountOrEject) {
            QStorageInfo qsi("/");
            QStringList rootDevNodes = DDiskManager::resolveDeviceNode(qsi.device(), {});
            if (!rootDevNodes.isEmpty()) {
                if (DDiskManager::createBlockDevice(rootDevNodes.first())->drive() == blk->drive()) {
                    protectUnmountOrEject = true;
                }
            }
        }
    }

    //cleartextDevice has hintSystem = true regardless of its backer's value ...
    if (suffix() == SUFFIX_GVFSMP || (suffix() == SUFFIX_UDISKS && blk && ((!blk->mountPoints().empty() && !d->blk->hintSystem())))) {
        if (!protectUnmountOrEject) {
            ret.push_back(MenuAction::Unmount);
        }
    }

    if (suffix() == SUFFIX_UDISKS && blk && blk->mountPoints().empty()) {
        ret.push_back(MenuAction::Mount);
        if (!blk->readOnly()) {
            ret.push_back(MenuAction::Rename);
            ret.push_back(MenuAction::FormatDevice);
        }
        if (drv->optical() && drv->media().contains(QRegularExpression("_r(w|e)"))) {
            ret.push_back(MenuAction::OpticalBlank);
        }
    }

    if (drv && drv->ejectable() && (!drv->canPowerOff() || drv->mediaCompatibility().join(" ").contains("optical"))) {
        if (!protectUnmountOrEject) {
            ret.push_back(MenuAction::Eject);
        }
    }
    if (drv && drv->canPowerOff()) {
        ret.push_back(MenuAction::SafelyRemoveDrive);
    }

    if (suffix() == SUFFIX_GVFSMP && d->gmnt && d->gmnt->getRootFile()) {
        QString scheme = QUrl(d->gmnt->getRootFile()->uri()).scheme();
        if (QSet<QString>({SMB_SCHEME, FTP_SCHEME, SFTP_SCHEME, DAV_SCHEME}).contains(scheme)) {
            ret.push_back(MenuAction::ForgetPassword);
        }
    }

    ret.push_back(MenuAction::Separator);

    ret.push_back(MenuAction::Property);
    return ret;
}

bool DFMRootFileInfo::canRedirectionFileUrl() const
{
    return true;
}

DUrl DFMRootFileInfo::redirectedFileUrl() const
{
    Q_D(const DFMRootFileInfo);
    if (suffix() == SUFFIX_USRDIR) {
        return DUrl::fromLocalFile(d->backer_url);
    } else if (suffix() == SUFFIX_GVFSMP) {
        return DUrl::fromLocalFile(d->backer_url);
    } else if (suffix() == SUFFIX_UDISKS) {
        QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(d->blk->drive()));
        if (drv->optical()) {
            return DUrl::fromBurnFile(QString(d->blk->device()) + "/" + BURN_SEG_ONDISC + "/");
        }
        if (d->mps.size()) {
            return DUrl::fromLocalFile(d->mps.first());
        }
    }
    return DUrl();
}

bool DFMRootFileInfo::canDrop() const
{
    return false;
}

Qt::DropActions DFMRootFileInfo::supportedDragActions() const
{
    return Qt::DropAction::IgnoreAction;
}

Qt::DropActions DFMRootFileInfo::supportedDropActions() const
{
    return Qt::DropAction::IgnoreAction;
}

QVariantHash DFMRootFileInfo::extraProperties() const
{
    Q_D(const DFMRootFileInfo);
    QVariantHash ret;
    if (suffix() == SUFFIX_GVFSMP) {
        if (d->gfsi) {
            ret["fsUsed"] = d->gfsi->fsUsedBytes();
            ret["fsSize"] = d->gfsi->fsTotalBytes();
            ret["fsType"] = d->gfsi->fsType();
        }
        ret["rooturi"] = d->gmnt && d->gmnt->getRootFile() ? d->gmnt->getRootFile()->uri() : "";
        ret["mounted"] = true;
    } else if (suffix() == SUFFIX_UDISKS) {
        if (d->mps.empty()) {
            ret["fsUsed"] = ~0ULL;
        } else {
            QStorageInfo si(d->mps.front());
            ret["fsUsed"] = quint64(si.bytesTotal() - si.bytesFree());
        }
        ret["fsSize"] = quint64(d->size);
        ret["fsType"] = d->fs;
        ret["encrypted"] = d->encrypted;
        ret["unlocked"] = !d->encrypted || d->ctblk;
        ret["udisksblk"] = d->ctblk ? d->ctblk->path() : d->blk->path();
        ret["mounted"] = !d->mps.empty();
    }
    return ret;
}

void DFMRootFileInfo::checkCache()
{
    Q_D(DFMRootFileInfo);
    if (!d->blk) {
        return;
    }

    d->encrypted = d->blk->isEncrypted();
    if (d->encrypted) {
        if (d->blk->cleartextDevice().length() > 1) {
            d->ctblk.reset(DDiskManager::createBlockDevice(d->blk->cleartextDevice()));
        } else {
            d->ctblk.clear();
        }
    }

    DBlockDevice *blk = d->ctblk ? d->ctblk.data() : d->blk.data();
    d->mps = blk->mountPoints();
    d->size = blk->size();
    d->label = blk->idLabel();
    d->fs = blk->idType();
    d->udispname = udisksDisplayName();
}

QString DFMRootFileInfo::udisksDisplayName()
{
    Q_D(DFMRootFileInfo);

    static QMap<QString, const char*> i18nMap {
        {"data", "Data Disk"}
    };
    const QString ddeI18nSym = QStringLiteral("_dde_");

    static std::initializer_list<std::pair<QString, QString>> opticalmediakeys {
        {"optical",                "Optical"},
        {"optical_cd",             "CD-ROM"},
        {"optical_cd_r",           "CD-R"},
        {"optical_cd_rw",          "CD-RW"},
        {"optical_dvd",            "DVD-ROM"},
        {"optical_dvd_r",          "DVD-R"},
        {"optical_dvd_rw",         "DVD-RW"},
        {"optical_dvd_ram",        "DVD-RAM"},
        {"optical_dvd_plus_r",     "DVD+R"},
        {"optical_dvd_plus_rw",    "DVD+RW"},
        {"optical_dvd_plus_r_dl",  "DVD+R/DL"},
        {"optical_dvd_plus_rw_dl", "DVD+RW/DL"},
        {"optical_bd",             "BD-ROM"},
        {"optical_bd_r",           "BD-R"},
        {"optical_bd_re",          "BD-RE"},
        {"optical_hddvd",          "HD DVD-ROM"},
        {"optical_hddvd_r",        "HD DVD-R"},
        {"optical_hddvd_rw",       "HD DVD-RW"},
        {"optical_mo",             "MO"}
    };
    static QVector<std::pair<QString, QString>> opticalmediakv(opticalmediakeys);
    static QMap<QString, QString> opticalmediamap(opticalmediakeys);

    if (d->label.startsWith(ddeI18nSym)) {
        QString i18nKey = d->label.mid(ddeI18nSym.size(), d->label.size() - ddeI18nSym.size());
        return qApp->translate("DeepinStorage", i18nMap.value(i18nKey, i18nKey.toUtf8().constData()));
    }

    if (d->mps.contains(QByteArray("/\0", 2))) {
        return QCoreApplication::translate("PathManager", "System Disk");
    }
    if (d->label.length() == 0) {
        QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(d->blk->drive()));
        if (!drv->mediaAvailable() && drv->mediaCompatibility().join(" ").contains("optical")) {
            QString maxmediacompat;
            for (auto i = opticalmediakv.rbegin(); i != opticalmediakv.rend(); ++i) {
                if (drv->mediaCompatibility().contains(i->first)) {
                maxmediacompat = i->second;
                break;
                }
            }
            return QCoreApplication::translate("DeepinStorage", "%1 Drive").arg(maxmediacompat);
        }
        if (drv->opticalBlank()) {
            return QCoreApplication::translate("DeepinStorage", "Blank %1 Disc").arg(opticalmediamap[drv->media()]);
        }
        if (d->blk->isEncrypted() && !d->ctblk) {
            return QCoreApplication::translate("DeepinStorage", "%1 Encrypted").arg(FileUtils::formatSize(d->size));
        }
        return QCoreApplication::translate("DeepinStorage", "%1 Volume").arg(FileUtils::formatSize(d->size));
    }
    return d->label;
}

bool DFMRootFileInfo::typeCompare(const DAbstractFileInfoPointer &a, const DAbstractFileInfoPointer &b)
{
    static const QHash<ItemType, int> priomap = {
        {ItemType::UserDirectory  , -1},
        {ItemType::UDisksRoot     ,  0},
        {ItemType::UDisksData     ,  1},
        {ItemType::UDisksFixed    ,  2},
        {ItemType::UDisksRemovable,  3},
        {ItemType::UDisksOptical  ,  4},
        {ItemType::GvfsSMB        ,  5},
        {ItemType::GvfsFTP        ,  5},
        {ItemType::GvfsMTP        ,  6},
        {ItemType::GvfsGPhoto2    ,  6},
        {ItemType::GvfsGeneric    ,  7}
    };
    if (!a->exists()) {
        return false;
    }
    if (!b->exists()) {
        return true;
    }
    return priomap[static_cast<DFMRootFileInfo::ItemType>(a->fileType())] < priomap[static_cast<DFMRootFileInfo::ItemType>(b->fileType())];
}
