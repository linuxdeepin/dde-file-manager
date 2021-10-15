/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     dengkeyun<dengkeyun@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#include "masteredmediafileinfo.h"
#include "private/dabstractfileinfo_p.h"
#include "app/define.h"
#include "deviceinfo/udisklistener.h"
#include "dfileservices.h"
#include "disomaster.h"
#include "dblockdevice.h"
#include "ddiskdevice.h"
#include "ddiskmanager.h"
#include <QRegularExpression>
#include "controllers/masteredmediacontroller.h"
#include "views/dfmopticalmediawidget.h"

MasteredMediaFileInfo::MasteredMediaFileInfo(const DUrl &url)
    : DAbstractFileInfo(url)
{
    backupInfo(url);
}

bool MasteredMediaFileInfo::exists() const
{
    Q_D(const DAbstractFileInfo);

    if (fileUrl().isEmpty() || !m_backerUrl.isValid() || m_backerUrl.isEmpty()) {
        return false;
    }
    if (fileUrl().fragment() == "dup") {
        return false;
    }

    return d->proxy && d->proxy->exists();
}

bool MasteredMediaFileInfo::isReadable() const
{
    Q_D(const DAbstractFileInfo);

    if (!d->proxy)
        return true;

    return d->proxy->isReadable();
}

bool MasteredMediaFileInfo::isWritable() const
{
    if (!m_backerUrl.burnIsOnDisc())
        return true;
    return ISOMaster->getDevicePropertyCached(fileUrl().burnDestDevice()).avail > 0;
}

bool MasteredMediaFileInfo::isDir() const
{
    Q_D(const DAbstractFileInfo);

    return !d->proxy || d->proxy->isDir();
}

int MasteredMediaFileInfo::filesCount() const
{
    Q_D(const DAbstractFileInfo);

    if (!d->proxy)
        return 0;

    return d->proxy->filesCount();
}

QString MasteredMediaFileInfo::fileDisplayName() const
{
    Q_D(const DAbstractFileInfo);

    if (fileUrl().burnFilePath().contains(QRegularExpression("^(/*)$"))) {
        const QStringList &nodes = DDiskManager::resolveDeviceNode(fileUrl().burnDestDevice(), {});
        if (nodes.isEmpty()) {
            qWarning() << "can't get the node list from: " << fileUrl().path();
            return "";
        }

        QString udiskspath = nodes.first();
        QSharedPointer<DBlockDevice> blkdev(DDiskManager::createBlockDevice(udiskspath));
        return blkdev->idLabel();
    }

    if (!d->proxy)
        return "";
    return d->proxy->fileDisplayName();
}

QVariantHash MasteredMediaFileInfo::extraProperties() const
{
    Q_D(const DAbstractFileInfo);

    QVariantHash ret;
    if (d->proxy) {
        ret = d->proxy->extraProperties();
    }
    ret["mm_backer"] = m_backerUrl.path();
    return ret;
}

QVector<MenuAction> MasteredMediaFileInfo::menuActionList(MenuType type) const
{
    QVector<MenuAction> ret = DAbstractFileInfo::menuActionList(type);
    ret.removeAll(MenuAction::TagInfo);
    ret.removeAll(MenuAction::TagFilesUseColor);
    ret.removeAll(MenuAction::Cut);
    ret.removeAll(MenuAction::Rename);
    ret.removeAll(MenuAction::NewFolder);
    ret.removeAll(MenuAction::NewText);
    ret.removeAll(MenuAction::NewWord);
    ret.removeAll(MenuAction::NewExcel);
    ret.removeAll(MenuAction::NewDocument);
    ret.removeAll(MenuAction::NewPowerpoint);
    ret.removeAll(MenuAction::DecompressHere);
    if (!fileUrl().burnIsOnDisc() || !m_backerUrl.isValid() || m_backerUrl.isEmpty()) {
        ret.removeAll(MenuAction::OpenInTerminal);
        ret.removeAll(MenuAction::OpenAsAdmin);
        ret.removeAll(MenuAction::Copy);
        ret.removeAll(MenuAction::SendToRemovableDisk);
        ret.removeAll(MenuAction::StageFileForBurning);
        ret.removeAll(MenuAction::Compress);
        ret.removeAll(MenuAction::CreateSymlink);
        ret.removeAll(MenuAction::SendToDesktop);
        ret.removeAll(MenuAction::AddToBookMark);
        ret.removeAll(MenuAction::Share);
    }
    if (fileUrl().burnIsOnDisc()) {
        ret.removeAll(MenuAction::Delete);
        ret.removeAll(MenuAction::CompleteDeletion);
    }
    return ret;
}

bool MasteredMediaFileInfo::canRedirectionFileUrl() const
{
    if (isDir())
        return isSymLink(); // fix bug 202007010021 当光驱刻录的文件夹中存在文件夹的链接时，要跳转到链接对应的目标文件夹
    return !isDir();
}

DUrl MasteredMediaFileInfo::redirectedFileUrl() const
{
    Q_D(const DAbstractFileInfo);
    if (d->proxy) {
        return d->proxy->fileUrl();
    }

    return DAbstractFileInfo::fileUrl();
}

DUrl MasteredMediaFileInfo::mimeDataUrl() const
{
    Q_D(const DAbstractFileInfo);
    if (d->proxy) {
        return d->proxy->mimeDataUrl();
    }

    return DAbstractFileInfo::mimeDataUrl();
}

bool MasteredMediaFileInfo::canIteratorDir() const
{
    return true; // fix 27116 光驱中使用搜索无效。通过这种方式，点击搜索结果中的文件夹将会跳转到文件的实际挂载目录，如果要回到刻录页面，需要点击返回或者侧边栏，点击顶部的面包屑导航也只能到挂载根目录
//    return !d->proxy;
}

DUrl MasteredMediaFileInfo::parentUrl() const
{
    if (fileUrl().burnFilePath().contains(QRegularExpression("^(/*)$"))) {
        return DUrl::fromLocalFile(QDir::homePath());
    }
    return DAbstractFileInfo::parentUrl();
}

DUrl MasteredMediaFileInfo::goToUrlWhenDeleted() const
{
    const auto &fUrl = fileUrl();

    QStringList rootDeviceNode = DDiskManager::resolveDeviceNode(fUrl.burnDestDevice(), {});
    if (rootDeviceNode.isEmpty() || this->m_backerUrl.isEmpty()) {
        return DUrl(COMPUTER_ROOT);
    }

    QString volTotag = getVolTag(rootDeviceNode.first());
    CdStatusInfo *pStatusInfo = DFMOpticalMediaWidget::getCdStatusInfo(volTotag);

    if (!fUrl.burnFilePath().isEmpty() && pStatusInfo != nullptr && !pStatusInfo->bReadyToBurn) { // 光盘路径是不能被删除的，有删除动作一般是弹出了，所以直接到homepath
        return DUrl(COMPUTER_ROOT);
    }
    return DAbstractFileInfo::goToUrlWhenDeleted();
}

QString MasteredMediaFileInfo::toLocalFile() const
{
    Q_D(const DAbstractFileInfo);

    if (d->proxy)
        return d->proxy->toLocalFile();

    return "";
}

bool MasteredMediaFileInfo::canDrop() const
{
    return isWritable();
}

bool MasteredMediaFileInfo::canTag() const
{
    return false;
}

bool MasteredMediaFileInfo::canRename() const
{
    return false;
}

QSet<MenuAction> MasteredMediaFileInfo::disableMenuActionList() const
{
    QSet<MenuAction> list;

    if (!isWritable()) {
        list << MenuAction::NewFolder
             << MenuAction::NewDocument
             << MenuAction::Paste;
    }

    list << MenuAction::Cut << MenuAction::Rename;// << MenuAction::Delete;

    if (isVirtualEntry()) {
        list << MenuAction::Copy;
    }

    return list;
}

void MasteredMediaFileInfo::refresh(const bool isForce)
{
    Q_UNUSED(isForce)
    Q_D(const DAbstractFileInfo);

    DAbstractFileInfo::refresh();
    if (d->proxy) {
        return;
    }

    backupInfo(fileUrl());
}

bool MasteredMediaFileInfo::canDragCompress() const
{
    // 光盘不支持拖拽压缩
    return false;
}

QString MasteredMediaFileInfo::subtitleForEmptyFloder() const
{
    return QObject::tr("Folder is empty");
}


QString MasteredMediaFileInfo::getVolTag(const QString &burnPath) const
{
    QString strKey;
    QStringList lst = burnPath.split("/", QString::SkipEmptyParts); // /dev/sr0 -> { dev, sr0 }
    if (lst.count() >= 1)
        strKey = lst[lst.count() - 1]; // strKey =
    return strKey;
}

void MasteredMediaFileInfo::backupInfo(const DUrl &url)
{
    if (url.burnDestDevice().length() == 0) {
        return;
    }

    QStringList rootDeviceNode = DDiskManager::resolveDeviceNode(url.burnDestDevice(), {});
    if (rootDeviceNode.isEmpty()) {
        return;
    }
    QString udiskspath = rootDeviceNode.first();
    QSharedPointer<DBlockDevice> blkdev(DDiskManager::createBlockDevice(udiskspath));
    QSharedPointer<DDiskDevice> diskdev(DDiskManager::createDiskDevice(blkdev->drive()));

    if (url.burnIsOnDisc()) {
        if (diskdev->opticalBlank()) {
            //blank media contains no files on disc, duh!
            return;
        }

        auto points = blkdev->mountPoints();
        if (!points.isEmpty()) {
            QString mntpoint = QString(points.front());
            while (*mntpoint.rbegin() == '/') {
                mntpoint.chop(1);
            }

            m_backerUrl = DUrl::fromLocalFile(mntpoint + url.burnFilePath());
        } else m_backerUrl = DUrl();
    } else {
        m_backerUrl = MasteredMediaController::getStagingFile(url);
    }
    setProxy(DFileService::instance()->createFileInfo(Q_NULLPTR, m_backerUrl));
}
