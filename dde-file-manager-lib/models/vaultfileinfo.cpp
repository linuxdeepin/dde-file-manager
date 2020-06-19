/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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
#include <sys/stat.h>
#include "vaultfileinfo.h"
#include "private/dabstractfileinfo_p.h"
#include "dfileservices.h"
#include "controllers/vaultcontroller.h"
#include "dfilesystemmodel.h"

#include <QStandardPaths>
#include <QIcon>
#include <qplatformdefs.h>

class VaultFileInfoPrivate : public DAbstractFileInfoPrivate
{
public:
    VaultFileInfoPrivate(const DUrl &url, VaultFileInfo *qq) : DAbstractFileInfoPrivate(url, qq, true) {}
};

VaultFileInfo::VaultFileInfo(const DUrl &url)
    : DAbstractFileInfo(url, false)
{
    // normal file map to..
    DUrl actualUrl = DUrl::fromLocalFile(VaultController::vaultToLocal(url));
    setProxy(DAbstractFileInfoPointer(DFileService::instance()->createFileInfo(nullptr, actualUrl)));
}

VaultFileInfo::~VaultFileInfo()
{
}

bool VaultFileInfo::exists() const
{
    Q_D(const VaultFileInfo);
    if (d->proxy) {
        return d->proxy->exists();
    }

    return DAbstractFileInfo::exists();
}

DUrl VaultFileInfo::parentUrl() const
{
    Q_D(const VaultFileInfo);

    if (fileUrl().path() == "/") return DAbstractFileInfo::parentUrl();

    if (d->proxy) {
        return VaultController::localUrlToVault(d->proxy->parentUrl());
    }

    return DAbstractFileInfo::parentUrl();
}

QString VaultFileInfo::iconName() const
{
    Q_D(const VaultFileInfo);

    QString iconName = "dfm_safebox"; // 如果是根目录，用保险柜图标

    if (!isRootDirectory()) {
        if (d->proxy) {
            iconName = d->proxy->iconName();
        }
    }
    else if(!iconName.isEmpty())
    {
        return iconName;
    }

    return DAbstractFileInfo::iconName();
}

QString VaultFileInfo::genericIconName() const
{
    Q_D(const VaultFileInfo);
    if (d->proxy) {
        return d->proxy->genericIconName();
    }

    return "drive-harddisk-encrypted";
}

DUrl VaultFileInfo::mimeDataUrl() const
{
    Q_D(const VaultFileInfo);
    if (d->proxy) {
        return d->proxy->mimeDataUrl();
    }

    return DAbstractFileInfo::mimeDataUrl();
}

bool VaultFileInfo::canRedirectionFileUrl() const
{
    Q_D(const VaultFileInfo);
    if (d->proxy) {
        return !d->proxy->isDir();
    }

    return DAbstractFileInfo::canRedirectionFileUrl();
}

DUrl VaultFileInfo::redirectedFileUrl() const
{
    Q_D(const VaultFileInfo);
    if (d->proxy) {
        return d->proxy->fileUrl();
    }

    return DAbstractFileInfo::redirectedFileUrl();
}

// 解决保险箱无法搜索的问题
bool VaultFileInfo::canIteratorDir() const
{
    return true;
}

QString VaultFileInfo::subtitleForEmptyFloder() const
{
    return QObject::tr("Folder is empty");
}

DUrl VaultFileInfo::getUrlByNewFileName(const QString &fileName) const
{
    DUrl url(parentUrl());
    url.setPath(url.path() + QDir::separator() + fileName);
    return url;
}

QList<QIcon> VaultFileInfo::additionalIcon() const
{
    QList<QIcon> icons;
    if (isSymLink()) {
        icons << QIcon::fromTheme("emblem-symbolic-link", DFMGlobal::instance()->standardIcon(DFMGlobal::LinkIcon));
    }

    if (!isWritable()) {
        icons << QIcon::fromTheme("emblem-readonly", DFMGlobal::instance()->standardIcon(DFMGlobal::LockIcon));
    }

    return icons;
}

bool VaultFileInfo::isWritable() const
{
    VaultController::FileBaseInfo fbi = VaultController::getVaultController()->getFileInfo(fileUrl());
    if (fbi.isExist){
        return fbi.isWritable;
    }

    return DAbstractFileInfo::isWritable();
}

bool VaultFileInfo::isSymLink() const
{
    VaultController::FileBaseInfo fbi = VaultController::getVaultController()->getFileInfo(fileUrl());
    if (fbi.isExist){
        return fbi.isSymLink;
    }

    bool flg = DAbstractFileInfo::isSymLink();
    return flg;
}

QFileDevice::Permissions VaultFileInfo::permissions() const
{
    QFileInfo fileInfo(fileUrl().toLocalFile());
    return fileInfo.permissions();
}

QSet<MenuAction> VaultFileInfo::disableMenuActionList() const
{
    return DAbstractFileInfo::disableMenuActionList();
}

DUrl VaultFileInfo::goToUrlWhenDeleted() const
{
    if (isRootDirectory()) {
        return DUrl(COMPUTER_ROOT);
    }

    return DAbstractFileInfo::goToUrlWhenDeleted();
}

QVector<MenuAction> VaultFileInfo::menuActionList(DAbstractFileInfo::MenuType type) const
{
    if(type != SpaceArea) {
        if (isRootDirectory()) {

            VaultController::VaultState vaultState = VaultController::getVaultController()->state();

            QVector<MenuAction> actions;
            if (vaultState == VaultController::Unlocked) {

                actions << MenuAction::Open
                        << MenuAction::OpenInNewWindow
                        << MenuAction::Separator
                        << MenuAction::LockNow
                        << MenuAction::AutoLock
                        << MenuAction::Separator
                        << MenuAction::DeleteVault
                        << MenuAction::Separator
                        << MenuAction::Property;
            } else if (vaultState == VaultController::Encrypted) {
                actions << MenuAction::UnLock
                        << MenuAction::UnLockByKey;
            }

            return actions;
        }
    }

    return DAbstractFileInfo::menuActionList(type);
}

QMap<MenuAction, QVector<MenuAction> > VaultFileInfo::subMenuActionList(MenuType type) const
{
    if(type != SpaceArea) {
        if (isRootDirectory()) {

            QMap<MenuAction, QVector<MenuAction> > actions;
            QVector<MenuAction> vecActions;

            vecActions << MenuAction::Never
                       << MenuAction::Separator
                       << MenuAction::FiveMinutes
                       << MenuAction::TenMinutes
                       << MenuAction::TwentyMinutes;

            actions.insert(MenuAction::AutoLock, vecActions);

            return actions;
        }
    }

    return DAbstractFileInfo::subMenuActionList();
}

QString VaultFileInfo::fileDisplayName() const
{
    if (isRootDirectory()) {
        return QObject::tr("My Vault");
    }

    return DAbstractFileInfo::fileDisplayName();
}

bool VaultFileInfo::canRename() const
{
    if (isRootDirectory()) {
        return false;
    }

    // 如果父目录为只读权限，则不能重命名
    VaultController::FileBaseInfo fbi = VaultController::getVaultController()->getFileInfo(parentUrl());
    if (fbi.isExist && !fbi.isWritable){
        return false;
    }    

    return true;
}

bool VaultFileInfo::canShare() const
{
    return false;
}

bool VaultFileInfo::canTag() const
{
    if (isRootDirectory()) {
        return false;
    }
    return DAbstractFileInfo::canTag();
}

QIcon VaultFileInfo::fileIcon() const
{
    QIcon icon;
    if (isRootDirectory()) {
        icon = QIcon::fromTheme(iconName());
    } else {
        icon = DAbstractFileInfo::fileIcon();
    }

    return icon;
}

qint64 VaultFileInfo::size() const
{
    if (isRootDirectory()) {

        qint64 totoalSize = VaultController::getVaultController()->totalsize();
        return totoalSize;
    }

    return DAbstractFileInfo::size();
}

bool VaultFileInfo::isDir() const
{
    // resolved the issue when directory not exist
    QString path = this->filePath();
    QString local = VaultController::vaultToLocal(VaultController::makeVaultUrl());
    if (local.endsWith("/")) {
        local.chop(1);
    }

    if (path.endsWith("/") || path == local) {
        return true;
    }
    return DAbstractFileInfo::isDir();
}

bool VaultFileInfo::canDrop() const
{
    // 保险箱处于开锁状态下，可以拖拽文件到保险箱，否则，不支持拖拽
    if(VaultController::VaultState::Unlocked == VaultController::getVaultController()->getVaultState()){
        return true;
    }else {
        return false;
    }
}

bool VaultFileInfo::isAncestorsUrl(const DUrl &url, QList<DUrl> *ancestors) const
{
    Q_UNUSED(url)
    DUrl parentUrl = this->parentUrl();

    forever {
        if (ancestors && parentUrl.isValid()) {
            ancestors->append(parentUrl);
        }

        if (parentUrl == VaultController::makeVaultUrl("/")) {
            return true;
        }

        const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(Q_NULLPTR, parentUrl);

        if (!fileInfo) {
            break;
        }

        const DUrl &pu = fileInfo->parentUrl();

        if (pu == parentUrl) {
            break;
        }

        parentUrl = pu;
    }

    return false;
}

bool VaultFileInfo::isRootDirectory() const
{
    bool bRootDir = false;
    QString localFilePath = VaultController::getVaultController()->makeVaultLocalPath();
    QString vrfilePath = DUrl::fromVaultFile("/").toString();
    QString path = DAbstractFileInfo::filePath();
    if (localFilePath == path || vrfilePath == path || localFilePath + "/" == path || localFilePath == path + "/") {
        bRootDir = true;
    }
    return bRootDir;
}

