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
#include "app/filesignalmanager.h"

#include <dgiofile.h>
#include <dgiofileinfo.h>
#include <dgiomount.h>
#include <dgiovolume.h>
#include <dgiovolumemanager.h>
#include <dgiosettings.h>
#include <ddiskmanager.h>
#include <dblockdevice.h>
#include <ddiskdevice.h>
#include <unistd.h>

#include <QStandardPaths>
#include <QStorageInfo>
#include <QTextCodec>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QTimer>


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
    QString idUUID;
    QString currentUUID;
    QString backupUUID;
    bool isod;
    bool encrypted;
    DFMRootFileInfo *q_ptr;
    Q_DECLARE_PUBLIC(DFMRootFileInfo)
};

QMap<QString, DiskInfoStr> DFMRootFileInfo::DiskInfoMap = QMap<QString, DiskInfoStr>();

DFMRootFileInfo::DFMRootFileInfo(const DUrl &url) :
    DAbstractFileInfo(url),
    d_ptr(new DFMRootFileInfoPrivate)
{
//    loadDiskInfo();
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
        if (pathList.size() == 0) {
            qWarning() << url << "not existed";
            //fix 临时解决方案，彻底解决需要DDiskManager::resolveDeviceNode往下追踪
            for (int i = 0; i < 20; ++i) {
                QThread::msleep(50);

                pathList = DDiskManager::resolveDeviceNode("/dev" + url.path().chopped(QString("." SUFFIX_UDISKS).length()), {});
                if (pathList.size() != 0)
                    break;
                qWarning() << url << "not existed" << i;
            }
            if (pathList.size() == 0)
                return;
            //old
            //return;
            //endl
        }

        //note: fix the U disk quickly installed and removed into two U disk, last valid
        QString udiskspath;
        if (pathList.size() > 0) {
            udiskspath = pathList.last();
        }
        QSharedPointer<DBlockDevice> blk(DDiskManager::createBlockDevice(udiskspath));
        if (blk->path().length() != 0) {
            QSharedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(blk->drive()));
            d_ptr->isod = drv->mediaCompatibility().join(" ").contains("optical");
            d_ptr->backer_url = udiskspath;
            d_ptr->blk = blk;
            d_ptr->blk->setWatchChanges(true);
            checkCache();
            QObject::connect(d_ptr->blk.data(), &DBlockDevice::idLabelChanged, [this] {this->checkCache();});
            QObject::connect(d_ptr->blk.data(), &DBlockDevice::mountPointsChanged, [this] {
//                this->loadDiskInfo();
                this->checkCache();
            });
            QObject::connect(d_ptr->blk.data(), &DBlockDevice::sizeChanged, [this] {this->checkCache();});
            QObject::connect(d_ptr->blk.data(), &DBlockDevice::idTypeChanged, [this] {this->checkCache();});
            QObject::connect(d_ptr->blk.data(), &DBlockDevice::cleartextDeviceChanged, [this] {this->checkCache();});
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
        if (d->blk)  {
            return d->blk->path().length() != 0;
        } else {
            return false;
        }
    }
    return true;
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
        if (d->blk) {
            QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(d->blk->drive()));
            if (d->isod) {
                ret = ItemType::UDisksOptical;
            } else {
                ret = ItemType::UDisksFixed;
                if (drv->media() == "thumb" || drv->removable() || drv->mediaRemovable() || drv->ejectable()) {
                    ret = ItemType::UDisksRemovable;
                }
            }
        } else {
            ret = ItemType::UDisksRemovable;
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
        if (!d->gmnt || d->gmnt->themedIconNames().size() == 0) {
            return "";
        }
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
    Q_UNUSED(type)
    bool protectUnmountOrEject = false;
    DGioSettings gsettings("com.deepin.dde.filemanager.general", "/com/deepin/dde/filemanager/general/");
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
        if (gsettings.value("protect-non-media-mounts").toBool()) {
            QList<QByteArray> mountPoints = blk->mountPoints();
            for (auto &mountPoint : mountPoints) {
                if (!mountPoint.startsWith("/media/")) {
                    protectUnmountOrEject = true;
                    break;
                }
            }
        }

        if (gsettings.value("protect-root-device-mounts").toBool() && !protectUnmountOrEject) {
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
        if (d->blk) {
            QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(d->blk->drive()));
            if (drv->optical()) {
                return DUrl::fromBurnFile(QString(d->blk->device()) + "/" + BURN_SEG_ONDISC + "/");
            }
        }
        if (d->mps.size()) {
            DUrl rootUrl = DUrl::fromLocalFile(d->mps.first());

            //点击数据盘直接跳转到主目录
            if (rootUrl == DUrl::fromLocalFile("/data")) {
                QString userPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
                //确认/data目录下是否挂载了home目录
                QDir dir("/data" + userPath);
                if (dir.exists())
                    rootUrl = DUrl::fromLocalFile("/data" + userPath);
            }
            return rootUrl;
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
    ret["fsFreeSize"] = 0;
    if (suffix() == SUFFIX_GVFSMP) {
        if (d->gfsi) {
            ret["fsUsed"] = d->gfsi->fsTotalBytes() - d->gfsi->fsFreeBytes();
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
            ret["fsFreeSize"] = quint64(si.bytesFree());
        }
        ret["fsSize"] = quint64(d->size);
        ret["fsType"] = d->fs;
        ret["encrypted"] = d->encrypted;
        ret["unlocked"] = !d->encrypted || d->ctblk;
        if (d->ctblk) {
            ret["udisksblk"] = d->ctblk->path();
        } else if (d->blk) {
            ret["udisksblk"] = d->blk->path();
        }
        ret["mounted"] = !d->mps.empty();
    }
    return ret;
}

void DFMRootFileInfo::refresh(const bool isForce)
{
    Q_UNUSED(isForce)
    //udisk的refresh函数中的proxy为空指针，没有实际意义
    //由于优化性能后，u盘挂载文件信息会被缓存，因此需要在此增加刷新U盘的挂载数据
    if (suffix() == SUFFIX_UDISKS)
        checkCache();

    DAbstractFileInfo::refresh();
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

    //FAT32的卷标编码为ascii，idLabel中取不到中文，这里需要从symlink中取出label
    //symlink中的label编码有问题，这里做进一步处理
    QString idVersion = blk->idVersion().toUpper();
    if (idVersion == "FAT32") {
        for (QByteArray ba : blk->symlinks()) {
            QString link(ba);
            if (link.contains("/by-label/") && link.contains("\\x")) {
                QByteArray t_destByteArray;
                QByteArray t_tmpByteArray;
                for (int i = 0; i < ba.size(); i++) {
                    if (92 == ba.at(i)) {
                        if (4 == t_tmpByteArray.size()) {
                            t_destByteArray.append(QByteArray::fromHex(t_tmpByteArray));
                        } else {
                            if (t_tmpByteArray.size() > 4) {
                                t_destByteArray.append(QByteArray::fromHex(t_tmpByteArray.left(4)));
                                t_destByteArray.append(t_tmpByteArray.mid(4));
                            } else {
                                t_destByteArray.append(t_tmpByteArray);
                            }
                        }
                        t_tmpByteArray.clear();
                        t_tmpByteArray.append(ba.at(i));
                        continue;
                    } else if (t_tmpByteArray.size() > 0) {
                        t_tmpByteArray.append(ba.at(i));
                        continue;
                    } else {
                        t_destByteArray.append(ba.at(i));
                    }
                }

                if (4 == t_tmpByteArray.size()) {
                    t_destByteArray.append(QByteArray::fromHex(t_tmpByteArray));
                } else {
                    if (t_tmpByteArray.size() > 4) {
                        t_destByteArray.append(QByteArray::fromHex(t_tmpByteArray.left(4)));
                        t_destByteArray.append(t_tmpByteArray.mid(4));
                    } else {
                        t_destByteArray.append(t_tmpByteArray);
                    }
                }

                link = QTextCodec::codecForName("GBK")->toUnicode(t_destByteArray);

                int idx = link.lastIndexOf("/", link.length() - 1);
                d->label = link.mid(idx + 1);
                break;
            }
        }
    }
    loadDiskInfo();
    d->fs = blk->idType();
    d->idUUID = blk->idUUID();
    d->udispname = udisksDisplayName();
}

QString DFMRootFileInfo::udisksDisplayName()
{
    Q_D(DFMRootFileInfo);

    //windows分区需要显示该分区在windows下的盘符或卷标
    if (DFMRootFileInfo::DiskInfoMap.contains(d->idUUID)) {
        const DiskInfoStr info = DFMRootFileInfo::DiskInfoMap.value(d->idUUID);
        if (!info.label.isEmpty()) {
            return info.label;
        } else {
            return info.driver;
        }
    }

    static QMap<QString, const char *> i18nMap {
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
    } else if (!d->idUUID.isEmpty()) {
        if (d->currentUUID.isEmpty() || d->backupUUID.isEmpty()) {
            QFile recoveryFile(QString("/etc/%1/ab-recovery.json").arg(qApp->organizationName()));

            if (recoveryFile.open(QIODevice::ReadOnly)) {

                QByteArray recoveryData = recoveryFile.readAll();
                recoveryFile.close();

                QJsonParseError parseJsonErr;
                QJsonDocument jsonDoc(QJsonDocument::fromJson(recoveryData, &parseJsonErr));
                if (!(parseJsonErr.error == QJsonParseError::NoError)) {
                    qDebug() << "decode json file error, create new json data！";
                } else {
                    QJsonObject rootObj = jsonDoc.object();
                    const QString currentcurrentUUIDKey = "Current";
                    const QString backupUUIDKey = "Backup";
                    if (rootObj.contains(currentcurrentUUIDKey)) {
                        d->currentUUID = rootObj[currentcurrentUUIDKey].toString();
                    }
                    if (rootObj.contains(backupUUIDKey)) {
                        d->backupUUID = rootObj[backupUUIDKey].toString();
                    }
                }
            }
        }
        if (d->currentUUID == d->idUUID || d->backupUUID == d->idUUID) {
            return QCoreApplication::translate("PathManager", "System Disk");
        }
    }

    if (d->label.length() == 0) {
        QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(d->blk->drive()));
        if (drv->opticalBlank()) {
            return QCoreApplication::translate("DeepinStorage", "Blank %1 Disc").arg(opticalmediamap[drv->media()]);
        }
        // TODO xust，暂时先屏蔽判定uuid的语句，该语句会导致无法正常挂载的盘连盘符都不显示，引入该改动是为解决自动光驱弹出托盘后界面不刷新的问题，此处之后再找其他的方式调整。
        if ((!drv->mediaAvailable()/* || d->idUUID.isEmpty()*/) && drv->mediaCompatibility().join(" ").contains("optical")) { // uuid 为空认为光盘未挂载或已卸载（托盘已弹出），此时强制使用无光盘的名称
            d->blk->unmount({});
            d->size = 0;
            QTimer::singleShot(100, [] {
                emit fileSignalManager->requestUpdateComputerView();
            });
            QString maxmediacompat;
            for (auto i = opticalmediakv.rbegin(); i != opticalmediakv.rend(); ++i) {
                if (drv->mediaCompatibility().contains(i->first)) {
                    maxmediacompat = i->second;
                    break;
                }
            }
            return QCoreApplication::translate("DeepinStorage", "%1 Drive").arg(maxmediacompat);
        }
        if (d->blk->isEncrypted() && !d->ctblk) {
            return QCoreApplication::translate("DeepinStorage", "%1 Encrypted").arg(FileUtils::formatSize(d->size));
        }
        return QCoreApplication::translate("DeepinStorage", "%1 Volume").arg(FileUtils::formatSize(d->size));
    }

    return d->label;
}

QString DFMRootFileInfo::getVolTag()
{
    Q_D(DFMRootFileInfo);
    int nIdx = d->backer_url.lastIndexOf("/");
    return nIdx < 0 ? QString() : d->backer_url.mid(nIdx + 1);
}

bool DFMRootFileInfo::checkMpsStr(const QString &path) const
{
    Q_D(const DFMRootFileInfo);

    for (QByteArray ba : d->mps) {
        QString baStr(ba.data());
        if (baStr == path)
            return true;
    }

    return false;
}

bool DFMRootFileInfo::typeCompare(const DAbstractFileInfoPointer &a, const DAbstractFileInfoPointer &b)
{
    static const QHash<ItemType, int> priomap = {
        {ItemType::UserDirectory, -1},
        {ItemType::UDisksRoot,  0},
        {ItemType::UDisksData,  1},
        {ItemType::UDisksFixed,  2},
        {ItemType::UDisksRemovable,  3},
        {ItemType::UDisksOptical,  4},
        {ItemType::GvfsSMB,  5},
        {ItemType::GvfsFTP,  5},
        {ItemType::GvfsMTP,  6},
        {ItemType::GvfsGPhoto2,  6},
        {ItemType::GvfsGeneric,  7}
    };
    if (!a || !a->exists()) {
        return false;
    }
    if (!b || !b->exists()) {
        return true;
    }
    return priomap[static_cast<DFMRootFileInfo::ItemType>(a->fileType())] < priomap[static_cast<DFMRootFileInfo::ItemType>(b->fileType())];
}

bool DFMRootFileInfo::typeCompareByUrl(const DAbstractFileInfoPointer &a, const DAbstractFileInfoPointer &b)
{
    static const QHash<ItemType, int> priomap = {
        {ItemType::UserDirectory, -1},
        {ItemType::UDisksRoot,  0},
        {ItemType::UDisksData,  1},
        {ItemType::UDisksFixed,  2},
        {ItemType::UDisksRemovable,  3},
        {ItemType::UDisksOptical,  4},
        {ItemType::GvfsMTP,  5},
        {ItemType::GvfsGPhoto2,  5},
        {ItemType::GvfsGeneric,  6},
        {ItemType::GvfsSMB,  7},
        {ItemType::GvfsFTP,  7}
    };
    return priomap[static_cast<DFMRootFileInfo::ItemType>(a->fileType())] < priomap[static_cast<DFMRootFileInfo::ItemType>(b->fileType())];
}

void DFMRootFileInfo::loadDiskInfo()
{
    Q_D(const DFMRootFileInfo);

    if (d->mps.empty())
        return;

    QDir dir(QString(d->mps.front()) + "/UOSICON");
    if (!dir.exists()) {
        return;
    }

    QString jsonPath = dir.absolutePath();
    //不存在该目录
    if (jsonPath.isEmpty()) {
        return;
    }

    //读取本地json文件
    QFile file(jsonPath + "/diskinfo.json");
    if (!file.open(QIODevice::ReadWrite)) {
        return;
    }

    //解析json文件
    QJsonParseError jsonParserError;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(file.readAll(), &jsonParserError);
    if (jsonDocument.isNull() || jsonParserError.error != QJsonParseError::NoError) {
        return;
    }

    if (jsonDocument.isObject()) {
        QJsonObject jsonObject = jsonDocument.object();
        if (jsonObject.contains("DISKINFO") && jsonObject.value("DISKINFO").isArray()) {
            QJsonArray jsonArray = jsonObject.value("DISKINFO").toArray();
            for (int i = 0; i < jsonArray.size(); i++) {
                if (jsonArray[i].isObject()) {
                    QJsonObject jsonObjectInfo = jsonArray[i].toObject();
                    DiskInfoStr str;
                    if (jsonObjectInfo.contains("uuid"))
                        str.uuid = jsonObjectInfo.value("uuid").toString();
                    if (jsonObjectInfo.contains("drive"))
                        str.driver = jsonObjectInfo.value("drive").toString();
                    if (jsonObjectInfo.contains("label"))
                        str.label = jsonObjectInfo.value("label").toString();

                    DFMRootFileInfo::DiskInfoMap[str.uuid] = str;
                }
            }
        }
    }
}
