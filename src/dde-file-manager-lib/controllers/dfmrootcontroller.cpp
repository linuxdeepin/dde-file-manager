/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#include "dfmrootcontroller.h"
#include "dfmevent.h"
#include "dfmapplication.h"
#include "dfmglobal.h"
#include "models/dfmrootfileinfo.h"
#include "private/dabstractfilewatcher_p.h"
#include "utils/singleton.h"
#include "app/define.h"
#include "app/filesignalmanager.h"
#include "shutil/fileutils.h"
#include "dfmapplication.h"
#include "dfmsettings.h"
#include "utils.h"
#include "utils/grouppolicy.h"
#include "app/define.h"

#include <dgiofile.h>
#include <dgiofileinfo.h>
#include <dgiomount.h>
#include <dgiovolume.h>
#include <dgiovolumemanager.h>
#include <ddiskmanager.h>
#include <ddiskdevice.h>
#include <dblockpartition.h>
#include "dialogs/dialogmanager.h"
#include "deviceinfo/udisklistener.h"
#include <gvfs/networkmanager.h>
#include <QProcessEnvironment>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

DFM_USE_NAMESPACE

class DFMRootFileWatcherPrivate : public DAbstractFileWatcherPrivate
{
public:
    explicit DFMRootFileWatcherPrivate(DFMRootFileWatcher *qq)
        : DAbstractFileWatcherPrivate(qq) {}

    bool start() override;
    bool stop() override;
    bool handleGhostSignal(const DUrl &target, DAbstractFileWatcher::SignalType1 signal, const DUrl &url) override;

protected:
    void initBlockDevConnections(QSharedPointer<DBlockDevice> blk, const QString &devs);

private:
    QSharedPointer<DGioVolumeManager> vfsmgr;
    QSharedPointer<DDiskManager> udisksmgr;
    QList<QMetaObject::Connection> connections;
    QList<QSharedPointer<DBlockDevice>> blkdevs;
    QStringList connectionsurl;

    Q_DECLARE_PUBLIC(DFMRootFileWatcher)
};

static bool ignoreBlkDevice(const QString& blkPath, QSharedPointer<DBlockDevice> blk, QSharedPointer<DDiskDevice> drv)
{
    bool isOptical = drv->mediaCompatibility().join(",").contains("optical");
    if (!blk->hasFileSystem() && blk->size() < 1024 && !isOptical) {   // a super block is at least 1024 bytes, a full filesystem always have a superblock.
        qWarning() << "block device is ignored cause it's size is less than 1024" << blkPath;
        return true;
    }

    // 过滤snap产生的loop设备
    if(blk->isLoopDevice()){ // loop devices' display status only determind by GA_HideLoopPartitions property.
        if (DFMApplication::genericAttribute(DFMApplication::GA_HideLoopPartitions).toBool()) {
            qDebug()  << "block device is ignored by loop device:"  << blkPath;
            return true;
        } else {
            return false;
        }
    }

    if (blk->hintIgnore()) {
        qWarning()  << "block device is ignored by hintIgnore:"  << blkPath;
        return true;
    }

    if (!blk->hasFileSystem() && !isOptical && !blk->isEncrypted()) {
        if (!drv->removable()){ // 满足外围条件的本地磁盘，直接遵循以前的处理直接 continue
            qWarning()  << "block device is ignored by wrong removeable set for system disk:"  << blkPath;
            return true;
        }
    }

    if (blk->cryptoBackingDevice().length() > 1) {
        qWarning()  << "block device is ignored by crypted back device:"  << blkPath;
        return true;
    }

    // 是否是设备根节点，设备根节点无须记录
    if(blk->hasPartitionTable()){ // 替换 FileUtils::deviceShouldBeIgnore
        qDebug()  << "block device is ignored by parent node:"  << blkPath;
        return true;
    }

    if(blk->hasPartition()){
        QSharedPointer<DBlockPartition> partition(DDiskManager::createBlockPartition(blkPath));
        if(!partition.isNull()){
            DBlockPartition::Type type = partition->eType();
            switch (type) {
            //Extended partition with CHS addressing. It must reside within the first physical 8 GB of disk, else use 0Fh instead (see 0Fh, 85h, C5h, D5h)
            case DBlockPartition::Win95_Extended_LBA:
            case DBlockPartition::Linux_extended:
            case DBlockPartition::DRDOS_sec_extend:
            case DBlockPartition::Multiuser_DOS_extend:
            case DBlockPartition::Extended:{
                    qWarning()  << "block device is ignored by partion type:"  << partition->type() <<","<< blkPath;
                    return true;
                }
            default:
                break;
            }
        }
    }
    return false;
}

DFMRootController::DFMRootController(QObject *parent) : DAbstractFileController(parent)
{

}

bool DFMRootController::renameFile(const QSharedPointer<DFMRenameEvent> &event) const
{
    DAbstractFileInfoPointer fi(new DFMRootFileInfo(event->fromUrl()));
    if (!fi->canRename()) {
        return false;
    }

    DFMRootFileInfo *rootFi = dynamic_cast<DFMRootFileInfo*>(fi.data());
    if (rootFi && rootFi->canSetAlias())
        return setLocalDiskAlias(rootFi, event->toUrl().path());

    QString udiskspath = fi->extraProperties()["udisksblk"].toString();
    QScopedPointer<DBlockDevice> blk(DDiskManager::createBlockDevice(udiskspath));
    Q_ASSERT(blk && blk->path().length() > 0);

    const QString &curName = rootFi->udisksDisplayName();
    const QString &destName = event->toUrl().path();
    if (curName == destName)
        return true;

    blk->setLabel(destName, {});
    if (blk->lastError().type() != QDBusError::NoError) {
        qDebug() << blk->lastError() << blk->lastError().name();
    }

    return blk->lastError().type() == QDBusError::NoError;
}

void DFMRootController::reloadBlkName(const QString& blkPath, QSharedPointer<DBlockDevice> blk) const
{
    //检查挂载目录下是否存在diskinfo文件
    QByteArrayList mps = blk->mountPoints();
    if (mps.empty()) {
        qWarning()  << "failed to reload block device name for:"  << blkPath;
        return;
    }
    QString mpPath(mps.front());
    if (mpPath.lastIndexOf("/") != (mpPath.length() - 1))
        mpPath += "/";
    QDir kidDir(mpPath + "UOSICON");
    if (kidDir.exists()) {
        QString jsonPath = kidDir.absolutePath();
        loadDiskInfo(jsonPath);
    }
}

const QList<DAbstractFileInfoPointer> DFMRootController::getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const
{
    QList<DAbstractFileInfoPointer> ret;

    if (event->url().scheme() != DFMROOT_SCHEME || event->url().path() != "/") {
        return ret;
    }

    static const QList<QString> udir = {"desktop", "videos", "music", "pictures", "documents", "downloads"};
    for (auto d : udir) {
        DAbstractFileInfoPointer fp(new DFMRootFileInfo(DUrl(DFMROOT_ROOT + d + "." SUFFIX_USRDIR)));
        if (fp->exists()) {
            ret.push_back(fp);
        }
    }

    bool hasSetDiskPolicy;
    QStringList diskPolicyList;
    QStringList hintSystemDisks;
    if (DTK_POLICY_SUPPORT) {
        hasSetDiskPolicy = GroupPolicy::instance()->containKey(DISK_HIDDEN);
        if (hasSetDiskPolicy)
            diskPolicyList = GroupPolicy::instance()->getValue(DISK_HIDDEN).toStringList();
    }

    QStringList blkds = DDiskManager::blockDevices({});
    for (auto blks : blkds) {
        QSharedPointer<DBlockDevice> blk(DDiskManager::createBlockDevice(blks));
        QSharedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(blk->drive()));
        if(ignoreBlkDevice(blks, blk, drv)){
            continue;
        }

        reloadBlkName(blks, blk);

        if (DTK_POLICY_SUPPORT) {
            if (hasSetDiskPolicy && blk->hintSystem() && diskPolicyList.contains(blk->idUUID()))
                continue;

            if (blk->hintSystem())
                hintSystemDisks << blk->idUUID();

            if (!hasSetDiskPolicy && DFMApplication::genericAttribute(DFMApplication::GA_HiddenSystemPartition).toBool() && blk->hintSystem()) {
                qDebug()  << "block device is ignored by hintSystem&HiddenSystemPartition:"  << blks;
                continue;
            }
        } else {
            if (DFMApplication::genericAttribute(DFMApplication::GA_HiddenSystemPartition).toBool() && blk->hintSystem()) {
                qDebug()  << "block device is ignored by hintSystem&HiddenSystemPartition:"  << blks;
                continue;
            }
        }

        DAbstractFileInfoPointer fp(new DFMRootFileInfo(DUrl(DFMROOT_ROOT + QString(blk->device()).mid(QString("/dev/").length()) + "." SUFFIX_UDISKS)));
        ret.push_back(fp);
    }

    for (auto gvfsvol : DGioVolumeManager::getVolumes()) {
        if (gvfsvol->volumeMonitorName().contains(QRegularExpression("(MTP|GPhoto2|Afc)$")) && !gvfsvol->getMount()) {
            gvfsvol->mount();
        }
    }
    if (event->canconst()) {
        return ret;
    }
    //寻找所有的移动设备（移动硬盘，手机，U盘等）
    QStringList urllist;
    for (auto gvfsmp : DGioVolumeManager::getMounts()) {
        auto volume = gvfsmp->getVolume();
        if (volume && volume->volumeMonitorName().endsWith("UDisks2"))
            continue;
        if (gvfsmp->mountClass() == "GUnixMount")
            continue;

        auto rootFile = gvfsmp->getRootFile();
        if (!rootFile)
            continue;

        if (DUrl(rootFile->uri()).scheme() == BURN_SCHEME)
            continue;
        // fix bug 60719. 分区编辑器打开后，原本该被过滤的由 udisks2 控制的磁盘，现无法过滤了，导致缓存中的 udisks2 磁盘显示一次
        // 在这里的 gvfs 里又显示一次，且数据内容不一致。
        // 因此，在这里通过过滤 /meida/ 目录挂载点的方式，来过滤 udisks2 设备（udisks2 在默认情况下都是挂载磁盘到 /media/$USER/ 目录下）
        // 手动 sudo mount 挂载的设备不会被 Gio 捕获，因此不用考虑手动使用 mount 挂载到 /media 目录下的磁盘被过滤
        if (rootFile->uri().startsWith("file:///media/")) {
            // fix bug 97468
            // 仅在特殊场景下会触发此代码，因此不影响启动性能
            // 原 bug 60719 修复会将块设备和协议设备全部跳过，因此造成了协议设备
            // 挂载到 /media 下后无法在计算机页面显示的问题
            if (deviceListener->isFromNativeBlockDev(rootFile->path()))
                continue;
        }

        DUrl url;
        url.setScheme(DFMROOT_SCHEME);
        url.setPath("/" + QUrl::toPercentEncoding(rootFile->path()) + "." SUFFIX_GVFSMP);

        if (urllist.contains(QString("/" + QUrl::toPercentEncoding(rootFile->path()) + "." SUFFIX_GVFSMP)))
            continue;
        DAbstractFileInfoPointer fp(new DFMRootFileInfo(url));
        urllist << QString("/" + QUrl::toPercentEncoding(rootFile->path()) + "." SUFFIX_GVFSMP);
        if (fp->exists()) {
            ret.push_back(fp);
        }
    }

    // get the not connected, storaged remote connection
    if (DFMApplication::genericAttribute(DFMApplication::GA_AlwaysShowOfflineRemoteConnections).toBool()) {
        const QList<QVariantMap> &&stashedMounts = RemoteMountsStashManager::remoteMounts();
        qDebug() << stashedMounts;
        for (const auto &mount: stashedMounts) {
            if (!mount.contains(REMOTE_KEY))
                continue;
            auto key = mount.value(REMOTE_KEY).toString() + "." + SUFFIX_GVFSMP;
            auto encodedKey = QUrl::toPercentEncoding(key);
            encodedKey.prepend("/");
            qDebug() << encodedKey;
            if (urllist.contains(encodedKey)) {
                qDebug() << "stashed mount is mounted" << mount;
                continue;
            }

            // ftp sometimes do not have share field
            if (!(mount.contains(REMOTE_PROTOCOL) && mount.contains(REMOTE_HOST))) {
                qWarning() << "invalid stashed remote connection: " << mount;
                continue;
            }
            auto protocol = mount.value(REMOTE_PROTOCOL).toString();
            auto host = mount.value(REMOTE_HOST).toString();
            auto share = mount.value(REMOTE_SHARE).toString();
            if (protocol.isEmpty() || host.isEmpty()) {
                qWarning() << "protocol or host is empty: " << mount;
                continue;
            }
            //考虑到用户升级到有smb挂载项聚合功能的版本后的兼容性，把配置中缓存的.remote挂载信息移到StashedSmbDevices字段中
            //以smb://x.x.x.x格式保存
            if(protocol == SMB_SCHEME){
                RemoteMountsStashManager::insertStashedSmbDevice(QString("%1://%2").arg(protocol).arg(host));
                QString path = QString("%1://%2/%3").arg(protocol).arg(host).arg(share);
            }
            /*smb挂载项聚合功能中，这里不再对缓存的挂载目录创建DFMRootFileInfo对象（请保留此部分注释）
            QString path = QString("%1://%2/%3").arg(protocol).arg(host).arg(share);
            path.append(QString(".%1").arg(SUFFIX_STASHED_REMOTE));
            path.prepend(DFMROOT_ROOT);
            qDebug() << "get stashed remote connection: " << path;

            auto stashedMountInfo = new DFMRootFileInfo(DUrl::fromPercentEncoding(path.toUtf8()));
            DAbstractFileInfoPointer fp(stashedMountInfo);
            ret.push_back(fp);
            */
        }
    }



    qInfo() << "get rootfileinfo over !" << ret;
    for(auto item: ret) {
        qInfo() << item->fileUrl();
    }

    if (DTK_POLICY_SUPPORT && hasSetDiskPolicy) {
        if (diskPolicyList.isEmpty()) {
            // 同步组策略到老配置(不选中隐藏)
            DFMApplication::instance()->setGenericAttribute(DFMApplication::GA_HiddenSystemPartition, false);
            return ret;
        }

        bool allSet = true;
        for (auto blk : hintSystemDisks) {
            if (!diskPolicyList.contains(blk))
                allSet = false;
        }

        // 同步组策略到老配置(选中隐藏)

        if (allSet) {
            DFMApplication::instance()->setGenericAttribute(DFMApplication::GA_HiddenSystemPartition, true);
        } else {
            DFMApplication::instance()->setGenericAttribute(DFMApplication::GA_HiddenSystemPartition, false);
        }
    }

    return ret;
}

const DAbstractFileInfoPointer DFMRootController::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const
{
    return DAbstractFileInfoPointer(new DFMRootFileInfo(event->url()));
}

DAbstractFileWatcher *DFMRootController::createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const
{
    return new DFMRootFileWatcher(event->url());
}

QStringList DFMRootController::systemDiskList()
{
    QStringList tempList;
    QStringList blkds = DDiskManager::blockDevices({});

    for (auto blks : blkds) {
        QSharedPointer<DBlockDevice> blk(DDiskManager::createBlockDevice(blks));
        QSharedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(blk->drive()));
        if(ignoreBlkDevice(blks, blk, drv))
            continue;

        if (blk->hintSystem())
            tempList << blk->idUUID();
    }
    return tempList;
}

void DFMRootController::loadDiskInfo(const QString &jsonPath) const
{
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

bool DFMRootController::setLocalDiskAlias(DFMRootFileInfo *fi, const QString &alias) const
{
    if (!fi || !fi->canRename() || fi->getUUID().isEmpty()) {
        qWarning() << "params 'fi' exception";
        return false;
    }

    QString uuid(fi->getUUID());
    QString dispalyAlias(alias.trimmed());
    QString displayName(fi->udisksDisplayName());
    QVariantList list = DFMApplication::genericSetting()->value(DISKALIAS_GROUP, DISKALIAS_ITEMS).toList();

    // [a] empty alias  -> remove from list
    // [b] exists alias -> cover it
    // [c] not exists   -> append
    bool exists = false;
    for (int i = 0; i < list.count(); i++) {
        QVariantMap map = list.at(i).toMap();
        if (map.value(DISKALIAS_ITEM_UUID).toString() == uuid) {
            if (dispalyAlias.isEmpty()) {      // [a]
                list.removeAt(i);
            } else {                           // [b]
                map[DISKALIAS_ITEM_NAME] = displayName;
                map[DISKALIAS_ITEM_ALIAS] = dispalyAlias;
                list[i] = map;
            }
            exists = true;
            break;
        }
    }

    // [c]
    if (!exists && !dispalyAlias.isEmpty() && !uuid.isEmpty()) {
        QVariantMap map;
        map[DISKALIAS_ITEM_UUID] = uuid;
        map[DISKALIAS_ITEM_NAME] = displayName;
        map[DISKALIAS_ITEM_ALIAS] = dispalyAlias;
        list.append(map);
        qInfo() << "append setting item: " << map;
    }

    DFMApplication::genericSetting()->setValue(DISKALIAS_GROUP, DISKALIAS_ITEMS, list);
    DAbstractFileWatcher::ghostSignal(DUrl(DFMROOT_ROOT), &DAbstractFileWatcher::fileAttributeChanged, fi->fileUrl());
    return true;
}

DFMRootFileWatcher::DFMRootFileWatcher(const DUrl &url, QObject *parent):
    DAbstractFileWatcher(*new DFMRootFileWatcherPrivate(this), url, parent)
{

}

void DFMRootFileWatcherPrivate::initBlockDevConnections(QSharedPointer<DBlockDevice> blk, const QString &devs)
{
    Q_Q(DFMRootFileWatcher);
    DFMRootFileWatcher *wpar = qobject_cast<DFMRootFileWatcher *>(q);
    blkdevs.push_back(blk);
    blk->setWatchChanges(true);
    QString urlstr = DFMROOT_ROOT + devs.mid(QString("/org/freedesktop/UDisks2/block_devices/").length()) + "." SUFFIX_UDISKS;
    DUrl url(urlstr);

    if (blk->isEncrypted()) {
        QSharedPointer<DBlockDevice> ctblk(DDiskManager::createBlockDevice(blk->cleartextDevice()));
        blkdevs.push_back(ctblk);
        ctblk->setWatchChanges(true);
        foreach (const QString &str, connectionsurl) {
            if (str == urlstr + QString("_en")) {
                return;
            }
        }
        connections.push_back(QObject::connect(blk.data(), &DBlockDevice::cleartextDeviceChanged, [wpar, url](const QString &) {
            Q_EMIT wpar->fileAttributeChanged(url);
        }));
        connections.push_back(QObject::connect(ctblk.data(), &DBlockDevice::idLabelChanged, [wpar, url](const QString &) {
            Q_EMIT wpar->fileAttributeChanged(url);
        }));
        connections.push_back(QObject::connect(ctblk.data(), &DBlockDevice::mountPointsChanged, [wpar, url](const QByteArrayList &) {
            Q_EMIT wpar->fileAttributeChanged(url);
        }));
        connectionsurl << urlstr + QString("_en");
    } else {
        foreach (const QString &str, connectionsurl) {
            if (str == urlstr) {
                return;
            }
        }
        connections.push_back(QObject::connect(blk.data(), &DBlockDevice::idLabelChanged, [wpar, url](const QString &) {
            Q_EMIT wpar->fileAttributeChanged(url);
        }));
        connections.push_back(QObject::connect(blk.data(), &DBlockDevice::mountPointsChanged, [wpar, url](const QByteArrayList &) {
            Q_EMIT wpar->fileAttributeChanged(url);
        }));

        connections.push_back(QObject::connect(blk.data(), &DBlockDevice::sizeChanged, [wpar, url](qulonglong) {
            Q_EMIT wpar->fileAttributeChanged(url);
        }));
        connections.push_back(QObject::connect(blk.data(), &DBlockDevice::idTypeChanged, [wpar, url](QString) {
            Q_EMIT wpar->fileAttributeChanged(url);
        }));
        connections.push_back(QObject::connect(blk.data(), &DBlockDevice::cleartextDeviceChanged, [wpar, url](const QString &) {
            Q_EMIT wpar->fileAttributeChanged(url);
        }));
        connectionsurl << urlstr;
    }
}

bool DFMRootFileWatcherPrivate::start()
{
    Q_Q(DFMRootFileWatcher);

    if (q->fileUrl().path() != "/" || started) {
        return false;
    }

    if (!vfsmgr) {
        vfsmgr.reset(new DGioVolumeManager);
    }
    if (!udisksmgr) {
        udisksmgr.reset(new DDiskManager);
    }

    udisksmgr->setWatchChanges(true);

    DFMRootFileWatcher *wpar = qobject_cast<DFMRootFileWatcher *>(q);

    connections.push_back(QObject::connect(vfsmgr.data(), &DGioVolumeManager::mountAdded, [wpar](QExplicitlySharedDataPointer<DGioMount> mnt) {
        if (mnt->getVolume() && mnt->getVolume()->volumeMonitorName().endsWith("UDisks2")) {
            return;
        }
        if (DUrl(mnt->getRootFile()->uri()).scheme() == BURN_SCHEME) {
            return;
        }
        if (mnt->mountClass() == "GUnixMount") {
            return;
        }
        if (mnt->getRootFile()->path().length() == 0) {
            return;
        }

        // fix bug 60719/65994. 分区编辑器打开后，原本该被过滤的由 udisks2 控制的磁盘，现无法过滤了，导致缓存中的 udisks2 磁盘显示一次
        // 在这里的 gvfs 里又显示一次，且数据内容不一致。
        // 因此，在这里通过过滤 /meida/ 目录挂载点的方式，来过滤 udisks2 设备（udisks2 在默认情况下都是挂载磁盘到 /media/$USER/ 目录下）
        // 手动 sudo mount 挂载的设备不会被 Gio 捕获，因此不用考虑手动使用 mount 挂载到 /media 目录下的磁盘被过滤
        if (mnt->getRootFile()->uri().startsWith("file:///media/")) {
            // fix bug 97468
            // 仅在特殊场景下会触发此代码，因此不影响启动性能
            // 原 bug 60719 修复会将块设备和协议设备全部跳过，因此造成了协议设备
            // 挂载到 /media 下后无法在计算机页面显示的问题
            if (deviceListener->isFromNativeBlockDev(mnt->getRootFile()->path()))
                return;
        }

        DUrl url;
        url.setScheme(DFMROOT_SCHEME);
        QString mountPointPath = mnt->getRootFile()->path();
        url.setPath("/" + QUrl::toPercentEncoding(mnt->getRootFile()->path()) + "." SUFFIX_GVFSMP);
        Q_EMIT wpar->subfileCreated(url);
        if (FileUtils::isSmbPath(mountPointPath)) {
            //refresh all dde-file-manager window
            emit fileSignalManager->requestFreshAllFileView();
            //refresh dde-desktop
            emit fileSignalManager->requestFreshAllDesktop();

            emit fileSignalManager->requestShowNewWindows();
        }
    }));
    connections.push_back(QObject::connect(vfsmgr.data(), &DGioVolumeManager::mountRemoved, [wpar](QExplicitlySharedDataPointer<DGioMount> mnt) {
        if (mnt->getVolume() && mnt->getVolume()->volumeMonitorName().endsWith("UDisks2")) {
            return;
        }
        DUrl url;
        url.setScheme(DFMROOT_SCHEME);
        QString path = mnt->getRootFile()->path();
        // 此处 Gio Wrapper 或许有 bug， 有时可以获取 uri，但无法获取 path, 因此有了以下略丑的代码
        // 目的是将已知的 uri 拼装成 path，相关 bug：46021
        if (path.isNull() || path.isEmpty()) {
            QStringList qq = mnt->getRootFile()->uri().replace("/", "").split(":");
            if (qq.size() >= 3) {
                path = QString("/run/user/") + QString::number(getuid()) +
                       QString("/gvfs/") + qq.at(0) + QString(":host=" + qq.at(1) + QString(",port=") + qq.at(2));
            } else if (qq.size() == 2) {
                //修复bug55778,在mips合arm上这里切分出来就是2个
                if (qq.at(0).startsWith("smb")) {
                    QStringList smblist = mnt->getRootFile()->uri().replace(":/", "").split("/");
                    path = smblist.count() >= 3 ? QString("/run/user/")+ QString::number(getuid()) + QString("/gvfs/") +
                                        smblist.at(0) + QString("-share:server=" + smblist.at(1) + QString(",share=") + smblist.at(2)) : QString();
                }
                else {
                    path = QString("/run/user/")+ QString::number(getuid()) +
                                                                QString("/gvfs/") + qq.at(0) + QString(":host=" + qq.at(1));
                }
            }
        }
        qDebug() << path;
        url.setPath("/" + QUrl::toPercentEncoding(path) + "." SUFFIX_GVFSMP);
        QString uri = mnt->getRootFile()->uri();
        wpar->setProperty("isBathUnmuntSmb",deviceListener->isBatchedRemovingSmbMount());
        wpar->setProperty("remainUnmuntSmb",deviceListener->getCountOfMountedSmb(DUrl(uri).host()));
        Q_EMIT wpar->fileDeleted(url);
        emit fileSignalManager->requestRemoveRecentFile(path);
        qDebug() << uri << "mount removed";
        if (FileUtils::isSmbPath(path)) {
          int remainCountOfSmbMount = deviceListener->getCountOfMountedSmb(DUrl(uri).host());
          if(!(deviceListener->isBatchedRemovingSmbMount() && remainCountOfSmbMount >0)){//批量卸载SMB时，需卸载完毕再刷新界面以避免卡顿
              deviceListener->setBatchedRemovingSmbMount(false);
              emit fileSignalManager->requestFreshAllFileView();
              emit fileSignalManager->requestFreshAllDesktop();
           }

        }
        if (uri.contains("smb-share://") || uri.contains("smb://") || uri.contains("ftp://") || uri.contains("sftp://")) {
            // remove NetworkNodes cache, so next time cd uri will fetchNetworks
            std::string stdStr = uri.toStdString();
            QString smbUri = QUrl::fromPercentEncoding(QByteArray(stdStr.data()));
            qDebug() << smbUri << "mount removed";
            if (smbUri.endsWith("/")) {
                smbUri = smbUri.left(smbUri.length() - 1);
            }
            DUrl smbUrl(smbUri);
            DUrlList networkNodesKeys = NetworkManager::NetworkNodes.keys();
            for (auto networkNodesKey : networkNodesKeys) {
               if (networkNodesKey.toString().toLower().startsWith(smbUrl.toString().toLower()))
                    NetworkManager::NetworkNodes.remove(networkNodesKey);
            }
            NetworkManager::NetworkNodes.remove(smbUrl);
            smbUrl.setPath("");
            NetworkManager::NetworkNodes.remove(smbUrl);
            mnt->unmount(); // yes, we need do it again...otherwise we will goto an removed path like /run/user/1000/gvfs/smb-sharexxxx
        }
    }));
    connections.push_back(QObject::connect(vfsmgr.data(), &DGioVolumeManager::volumeAdded, [](QExplicitlySharedDataPointer<DGioVolume> vol) {
        if (vol->volumeMonitorName().contains(QRegularExpression("(MTP|GPhoto2|Afc)$"))) {
            vol->mount();
        }
    }));
    connections.push_back(QObject::connect(udisksmgr.data(), &DDiskManager::blockDeviceAdded, [wpar, this](const QString & blks) {
        QSharedPointer<DBlockDevice> blk(DDiskManager::createBlockDevice(blks));
        QSharedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(blk->drive()));

        if(ignoreBlkDevice(blks, blk, drv)) {
            return ;
        }

        initBlockDevConnections(blk, blks);

        Q_EMIT wpar->subfileCreated(DUrl(DFMROOT_ROOT + blks.mid(QString("/org/freedesktop/UDisks2/block_devices/").length()) + "." SUFFIX_UDISKS));
    }));
    connections.push_back(QObject::connect(udisksmgr.data(), &DDiskManager::blockDeviceRemoved, [wpar](const QString & blks) {
        Q_EMIT wpar->fileDeleted(DUrl(DFMROOT_ROOT + blks.mid(QString("/org/freedesktop/UDisks2/block_devices/").length()) + "." SUFFIX_UDISKS));
    }));

    for (auto devs : udisksmgr->blockDevices({})) {
        QSharedPointer<DBlockDevice> blk(DDiskManager::createBlockDevice(devs));
        QSharedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(blk->drive()));

        auto mountPoints = blk->mountPoints();
        if (!drv->removable() && !mountPoints.isEmpty()) { // feature: hide the specified dir of unremovable devices
            QString mountPoint = mountPoints[0];
            if (!mountPoint.endsWith("/"))
                mountPoint += "/";
            // no permission to create files under '/', cannot create .hidden file, so just hardcode here.
            deviceListener->appendHiddenDirs(mountPoint + "root");
            deviceListener->appendHiddenDirs(mountPoint + "lost+found");
        }

        if(ignoreBlkDevice(devs, blk, drv)){
            continue;
        }

        initBlockDevConnections(blk, devs);
    }

    started = true;
    return true;
}

bool DFMRootFileWatcherPrivate::stop()
{
    if (!started) {
        return false;
    }

    udisksmgr->setWatchChanges(false);

    for (auto &conn : connections) {
        QObject::disconnect(conn);
    }
    connections.clear();
    connectionsurl.clear();
    blkdevs.clear();

    vfsmgr.clear();
    udisksmgr.clear();

    started = false;

    return true;
}

bool DFMRootFileWatcherPrivate::handleGhostSignal(const DUrl &target, DAbstractFileWatcher::SignalType1 signal, const DUrl &url)
{
    Q_UNUSED(target)
    Q_Q(DFMRootFileWatcher);

    bool ok = true;
    QString &&path = url.path();
    if (path.startsWith("/dev/loop")) {
        if (signal == &DAbstractFileWatcher::fileDeleted) {
            // 命令 umount loop 设备不会触发 udisks2 和 gio 的信号，因此此处使用 gostSignal 的方式在计算机面移除 loop 设备 (bug-63131)
            qInfo() << "remove loop device: " << path;
            DFMRootFileWatcher *wpar = qobject_cast<DFMRootFileWatcher *>(q);
            Q_EMIT wpar->fileDeleted(DUrl(DFMROOT_ROOT + path.mid(QString("/dev/").length()) + "." SUFFIX_UDISKS));
        } else if (signal == &DAbstractFileWatcher::subfileCreated) {
            // TODO(zhangs): loop 设备手动挂载当前计算机页面并不会及时响应，
            // 但 loop 设备在未来的需求中, 将不会被显示在计算机页面，因此暂时不实现
        } else {
            ok = false;
        }
    } else {
        ok = false;
    }

    return ok;
}
