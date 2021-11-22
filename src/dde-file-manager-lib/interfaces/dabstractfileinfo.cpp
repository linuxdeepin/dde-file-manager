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

#include "gio/gio.h"

#include "dabstractfileinfo.h"
#include "private/dabstractfileinfo_p.h"
#include "views/dfileview.h"
#include "dfilemenu.h"

#include "dfilesystemmodel.h"
#include "dfilemenumanager.h"

#include "shutil/fileutils.h"
#include "shutil/mimetypedisplaymanager.h"

#include "controllers/bookmarkmanager.h"
#include "controllers/pathmanager.h"
#include "controllers/vaultcontroller.h"
#include "dfileservices.h"
#include "dmimedatabase.h"

#include "app/define.h"
#include "shutil/mimesappsmanager.h"
#include "controllers/appcontroller.h"

#include "singleton.h"
#include "plugins/pluginmanager.h"
#include "dde-file-manager-plugins/plugininterfaces/menu/menuinterface.h"

#include "deviceinfo/udisklistener.h"
#include "ddiskmanager.h"
#include "ddiskdevice.h"

#include "bluetooth/bluetoothmanager.h"
#include "bluetooth/bluetoothmodel.h"

#include "io/dstorageinfo.h"

#include <DDesktopEntry>

#include <QDateTime>
#include <QDebug>
#include <QApplication>
#include <QCollator>
#include <QWriteLocker>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardPaths>
#include <QLocale>

#include <dglibutils.h>
#include <memory> // std::unique_ptr

DCORE_USE_NAMESPACE

#ifdef SW_LABEL
#include "sw_label/filemanagerlibrary.h"
#include "dfilemenumanager.h"
#endif


namespace FileSortFunction {
//fix 多线程排序时，该处的全局变量在compareByString函数中可能导致软件崩溃
//QCollator sortCollator;
class DCollator : public QCollator
{
public:
    DCollator() : QCollator()
    {
        setNumericMode(true);
        setCaseSensitivity(Qt::CaseInsensitive);
    }
};

bool compareByString(const QString &str1, const QString &str2, Qt::SortOrder order)
{
    thread_local static DCollator sortCollator;
    //其他符号要排在最后，需要在中文前先做判断
    if (DFMGlobal::startWithSymbol(str1)) {
        if (!DFMGlobal::startWithSymbol(str2))
            return order == Qt::DescendingOrder;
    } else if (DFMGlobal::startWithSymbol(str2))
        return order != Qt::DescendingOrder;

    if (DFMGlobal::startWithHanzi(str1)) {
        if (!DFMGlobal::startWithHanzi(str2)) {
            return order == Qt::DescendingOrder;
        }
    } else if (DFMGlobal::startWithHanzi(str2)) {
        return order != Qt::DescendingOrder;
    }

    return ((order == Qt::DescendingOrder) ^ (sortCollator.compare(str1, str2) < 0)) == 0x01;
}

COMPARE_FUN_DEFINE(fileDisplayName, DisplayName, DAbstractFileInfo)
COMPARE_FUN_DEFINE(fileSize, Size, DAbstractFileInfo)
COMPARE_FUN_DEFINE(lastModified, Modified, DAbstractFileInfo)
COMPARE_FUN_DEFINE(fileTypeDisplayName, Mime, DAbstractFileInfo)
COMPARE_FUN_DEFINE(created, Created, DAbstractFileInfo)
COMPARE_FUN_DEFINE(lastRead, LastRead, DAbstractFileInfo)
} /// end namespace FileSortFunction

#define CALL_PROXY(Fun)\
    Q_D(const DAbstractFileInfo);\
    if (d->proxy) return d->proxy->Fun;

QMap<DUrl, DAbstractFileInfo *> DAbstractFileInfoPrivate::urlToFileInfoMap;
QReadWriteLock *DAbstractFileInfoPrivate::urlToFileInfoMapLock = new QReadWriteLock();
DMimeDatabase DAbstractFileInfoPrivate::mimeDatabase;

DAbstractFileInfoPrivate::DAbstractFileInfoPrivate(const DUrl &url, DAbstractFileInfo *qq, bool hasCache)
    : q_ptr(qq)
    , fileUrl(url)
{
    //###(zccrs): 只在主线程中开启缓存，防止不同线程中持有同一对象时的竞争问题
    if (hasCache && (url.isValid() && (QThread::currentThread()) &&  qApp && qApp->thread() && QThread::currentThread() == qApp->thread())) {
        QWriteLocker locker(urlToFileInfoMapLock);
        Q_UNUSED(locker)

        urlToFileInfoMap[url] = qq;
    }
}

DAbstractFileInfoPrivate::~DAbstractFileInfoPrivate()
{
    QReadLocker read_locker(urlToFileInfoMapLock);
    if (urlToFileInfoMap.value(fileUrl) == q_ptr) {
        read_locker.unlock();
        QWriteLocker write_locker(urlToFileInfoMapLock);
        Q_UNUSED(write_locker)
        urlToFileInfoMap.remove(fileUrl);
    } else {
        read_locker.unlock();
    }
}

void DAbstractFileInfoPrivate::setUrl(const DUrl &url, bool hasCache)
{
    if (url == fileUrl) {
        return;
    }

    if (urlToFileInfoMap.value(fileUrl) == q_ptr) {
        QWriteLocker locker(urlToFileInfoMapLock);
        Q_UNUSED(locker)
        urlToFileInfoMap.remove(fileUrl);
    }

    if (hasCache) {
        QWriteLocker locker(urlToFileInfoMapLock);
        Q_UNUSED(locker)
        urlToFileInfoMap[url] = q_ptr;
    }
    fileUrl = url;
}

DAbstractFileInfo *DAbstractFileInfoPrivate::getFileInfo(const DUrl &fileUrl)
{
    //###(zccrs): 只在主线程中开启缓存，防止不同线程中持有同一对象时的竞争问题,优化都可以
    if (!QThread::currentThread() || !qApp  || !qApp->thread() || QThread::currentThread() != qApp->thread()) {
        return nullptr;
    }

    if (!fileUrl.isValid()) {
        return nullptr;
    }
    return urlToFileInfoMap.value(fileUrl);
}

DAbstractFileInfo::DAbstractFileInfo(const DUrl &url, bool hasCache)
    : d_ptr(new DAbstractFileInfoPrivate(url, this, hasCache))
{

}

DAbstractFileInfo::DAbstractFileInfo(const DUrl &url, const QMimeType &mimetype, bool hasCache)
    : d_ptr(new DAbstractFileInfoPrivate(url, this, hasCache))
{
    Q_UNUSED(mimetype);
}

DAbstractFileInfo::~DAbstractFileInfo()
{

}

const DAbstractFileInfoPointer DAbstractFileInfo::getFileInfo(const DUrl &fileUrl)
{
    return DAbstractFileInfoPointer(DAbstractFileInfoPrivate::getFileInfo(fileUrl));
}

bool DAbstractFileInfo::exists() const
{
    CALL_PROXY(exists());

    return false;
}

bool DAbstractFileInfo::isPrivate() const
{
    return false;
}

QString DAbstractFileInfo::path() const
{
    CALL_PROXY(path());

    const QString &filePath = this->filePath();

    int index = filePath.lastIndexOf(QDir::separator());

    if (index >= 0) {
        return filePath.left(index);
    }

    return filePath;
}

QString DAbstractFileInfo::filePath() const
{
    CALL_PROXY(filePath());

    return fileUrl().path();
}

QString DAbstractFileInfo::absolutePath() const
{
    CALL_PROXY(absolutePath());

    if (isAbsolute()) {
        return path();
    }

    QFileInfo info(filePath());

    return info.absolutePath();
}

QString DAbstractFileInfo::absoluteFilePath() const
{
    CALL_PROXY(absoluteFilePath());

    if (isAbsolute()) {
        return filePath();
    }

    QFileInfo info(filePath());

    return info.absoluteFilePath();
}

QString DAbstractFileInfo::baseName() const
{
    CALL_PROXY(baseName());

    const QString &fileName = this->fileName();
    const QString &suffix = this->suffix();

    if (suffix.isEmpty()) {
        return fileName;
    }

    return fileName.left(fileName.length() - suffix.length() - 1);
}

QString DAbstractFileInfo::baseNameOfRename() const
{
    return baseName();
}

QString DAbstractFileInfo::fileName() const
{
    CALL_PROXY(fileName());

    QString filePath = this->filePath();

    if (filePath.endsWith(QDir::separator())) {
        filePath.chop(1);
    }

    int index = filePath.lastIndexOf(QDir::separator());

    if (index >= 0) {
        return filePath.mid(index + 1);
    }

    return filePath;
}

QString DAbstractFileInfo::fileNameOfRename() const
{
    return fileName();
}

QString DAbstractFileInfo::fileDisplayName() const
{
    CALL_PROXY(fileDisplayName());

    return fileName();
}

QString DAbstractFileInfo::fileSharedName() const
{
    CALL_PROXY(fileSharedName());

    return QString();
}

QString DAbstractFileInfo::fileDisplayPinyinName() const
{
    Q_D(const DAbstractFileInfo);

    const QString &diaplayName = this->fileDisplayName();

    if (d->pinyinName.isEmpty()) {
        d->pinyinName = DFMGlobal::toPinyin(diaplayName);
    }

    return d->pinyinName;
}

bool DAbstractFileInfo::canRename() const
{
    CALL_PROXY(canRename());

    return false;
}

bool DAbstractFileInfo::canShare() const
{
    CALL_PROXY(canShare());

    return false;
}

bool DAbstractFileInfo::canFetch() const
{
    CALL_PROXY(canFetch());

    return isDir() && !isPrivate();
}

bool DAbstractFileInfo::isReadable() const
{
    CALL_PROXY(isReadable());

    return permission(QFile::ReadUser);
}

bool DAbstractFileInfo::isWritable() const
{
    CALL_PROXY(isWritable());

    return permission(QFile::WriteUser);
}

bool DAbstractFileInfo::isExecutable() const
{
    CALL_PROXY(isExecutable());

    return permission(QFile::ExeUser);
}

bool DAbstractFileInfo::isHidden() const
{
    CALL_PROXY(isHidden());

    return false;
}

bool DAbstractFileInfo::isRelative() const
{
    CALL_PROXY(isRelative());

    return false;
}

bool DAbstractFileInfo::isAbsolute() const
{
    CALL_PROXY(isAbsolute());

    return false;
}

bool DAbstractFileInfo::isShared() const
{
    CALL_PROXY(isShared());

    return false;
}

bool DAbstractFileInfo::isTaged() const
{
    CALL_PROXY(isTaged());

    return false;
}

bool DAbstractFileInfo::canTag() const
{
    CALL_PROXY(canTag());

    return false;
}

bool DAbstractFileInfo::isWritableShared() const
{
    CALL_PROXY(isWritableShared());

    return false;
}

bool DAbstractFileInfo::isAllowGuestShared() const
{
    CALL_PROXY(isAllowGuestShared());

    return false;
}

bool DAbstractFileInfo::makeAbsolute()
{
    CALL_PROXY(makeAbsolute());

    return false;
}

bool DAbstractFileInfo::canDragCompress() const
{
    CALL_PROXY(canDragCompress());

    return false;
}

bool DAbstractFileInfo::canManageAuth() const
{
    CALL_PROXY(canManageAuth());

    return true;
}

bool DAbstractFileInfo::canMoveOrCopy() const
{
    CALL_PROXY(canMoveOrCopy());

    return true;
}

DAbstractFileInfo::FileType DAbstractFileInfo::fileType() const
{
    CALL_PROXY(fileType());

    return Unknown;
}

bool DAbstractFileInfo::isFile() const
{
    CALL_PROXY(isFile());

    return false;
}

bool DAbstractFileInfo::isDir() const
{
    CALL_PROXY(isDir());

    return false;
}

bool DAbstractFileInfo::isSymLink() const
{
    CALL_PROXY(isSymLink());

    return false;
}

bool DAbstractFileInfo::isDesktopFile() const
{
    CALL_PROXY(isDesktopFile());

    return mimeTypeName() == "application/x-desktop";
}

// blumia: 任何对应不到实际文件的项都应当被视为虚拟的项，虚拟的项没有 MimeType , 最后修改时间等实际文件本身所应具有的相关属性。
bool DAbstractFileInfo::isVirtualEntry() const
{
    CALL_PROXY(isVirtualEntry());

    return false;
}

bool DAbstractFileInfo::needCompleteDelete() const
{
    return (isGvfsMountFile()
            || VaultController::isVaultFile(toLocalFile())
            || deviceListener->isInRemovableDeviceFolder(absoluteFilePath()))
            && !isVirtualEntry();
}

bool DAbstractFileInfo::isDragCompressFileFormat() const
{
    CALL_PROXY(isDragCompressFileFormat());

    return false;
}

QString DAbstractFileInfo::symlinkTargetPath() const
{
    CALL_PROXY(symlinkTargetPath());

    return QString();
}

DUrl DAbstractFileInfo::symLinkTarget() const
{
    CALL_PROXY(symLinkTarget());

    return DUrl();
}

DUrl DAbstractFileInfo::rootSymLinkTarget() const
{
    DAbstractFileInfoPointer info = fileService->createFileInfo(Q_NULLPTR, fileUrl());

    while (info->isSymLink()) {
        DUrl targetUrl = info->symLinkTarget();

        if (targetUrl == info->fileUrl()) {
            break;
        }

        info = fileService->createFileInfo(Q_NULLPTR, targetUrl);
        if (!info) {
            return DUrl();
        }
    }

    return info->fileUrl();
}

QString DAbstractFileInfo::owner() const
{
    CALL_PROXY(owner());

    return QString();
}

uint DAbstractFileInfo::ownerId() const
{
    CALL_PROXY(ownerId());

    return 0;
}

QString DAbstractFileInfo::group() const
{
    CALL_PROXY(group());

    return QString();
}

uint DAbstractFileInfo::groupId() const
{
    CALL_PROXY(groupId());

    return 0;
}

bool DAbstractFileInfo::permission(QFileDevice::Permissions permissions) const
{
    CALL_PROXY(permission(permissions));

    return (permissions & this->permissions()) == permissions;
}

QFileDevice::Permissions DAbstractFileInfo::permissions() const
{
    CALL_PROXY(permissions());

    return QFileDevice::Permissions();
}

qint64 DAbstractFileInfo::size() const
{
    CALL_PROXY(size());

    return -1;
}

int DAbstractFileInfo::filesCount() const
{
    CALL_PROXY(filesCount());

    const DDirIteratorPointer &iterator = fileService->createDirIterator(Q_NULLPTR, fileUrl(), QStringList(),
                                                                         QDir::AllEntries | QDir::System
                                                                         | QDir::NoDotAndDotDot | QDir::Hidden,
                                                                         QDirIterator::NoIteratorFlags);

    int count = 0;

    if (!iterator) {
        return -1;
    }

    while (iterator->hasNext()) {
        iterator->next();

        ++count;
    }

    return count;
}

qint64 DAbstractFileInfo::fileSize() const
{
    if (isDir()) {
        return filesCount();
    }

    return size();
}

QDateTime DAbstractFileInfo::created() const
{
    CALL_PROXY(created());

    return QDateTime();
}

QDateTime DAbstractFileInfo::lastModified() const
{
    CALL_PROXY(lastModified());

    return QDateTime();
}

QDateTime DAbstractFileInfo::lastRead() const
{
    CALL_PROXY(lastRead());

    return QDateTime();
}

QMimeType DAbstractFileInfo::mimeType(QMimeDatabase::MatchMode mode) const
{
    CALL_PROXY(mimeType(mode));

    return QMimeType();
}

QString DAbstractFileInfo::iconName() const
{
    return mimeType().iconName();
}

QString DAbstractFileInfo::genericIconName() const
{
    return mimeType().genericIconName();
}

QString DAbstractFileInfo::lastReadDisplayName() const
{
    CALL_PROXY(lastReadDisplayName());

    return lastRead().toString(dateTimeFormat());
}

QString DAbstractFileInfo::lastModifiedDisplayName() const
{
    CALL_PROXY(lastModifiedDisplayName());

    if (!lastModified().isValid())
        return qApp->translate("MimeTypeDisplayManager", "Unknown");

    return lastModified().toString(dateTimeFormat());
}

QString DAbstractFileInfo::createdDisplayName() const
{
    CALL_PROXY(createdDisplayName());

    return created().toString(dateTimeFormat());
}

QString DAbstractFileInfo::sizeDisplayName() const
{
    CALL_PROXY(sizeDisplayName());

    if (isDir()) {
        int size = filesCount();

        if (size <= 1) {
            return QObject::tr("%1 item").arg(size);
        } else {
            return QObject::tr("%1 items").arg(size);
        }
    } else {
        return FileUtils::formatSize(size());
    }
}

QString DAbstractFileInfo::mimeTypeDisplayName() const
{
    CALL_PROXY(mimeTypeDisplayName());

    return mimeTypeDisplayManager->displayName(mimeTypeName());
}

QString DAbstractFileInfo::fileTypeDisplayName() const
{
    CALL_PROXY(fileTypeDisplayName());

    return QString::number(mimeTypeDisplayManager->displayNameToEnum(mimeTypeName())).append(suffix());
}

DUrl DAbstractFileInfo::fileUrl() const
{
    Q_D(const DAbstractFileInfo);

    return d->fileUrl;
}

QIcon DAbstractFileInfo::fileIcon() const
{
    CALL_PROXY(fileIcon());

    QIcon icon = QIcon::fromTheme(iconName());

    if (icon.isNull()) {
        icon = QIcon::fromTheme("application-default-icon");
    }
    return icon;
}

QList<QIcon> DAbstractFileInfo::additionalIcon() const
{
    CALL_PROXY(additionalIcon());

    QList<QIcon> icons;

    if (isSymLink()) {
        icons << QIcon::fromTheme("emblem-symbolic-link", DFMGlobal::instance()->standardIcon(DFMGlobal::LinkIcon));
    }

    if (!isWritable()) {
        icons << QIcon::fromTheme("emblem-readonly", DFMGlobal::instance()->standardIcon(DFMGlobal::LockIcon));
    }

    if (!isReadable()) {
        icons << QIcon::fromTheme("emblem-unreadable", DFMGlobal::instance()->standardIcon(DFMGlobal::UnreadableIcon));
    }

    if (isShared()) {
        icons << QIcon::fromTheme("emblem-shared", DFMGlobal::instance()->standardIcon(DFMGlobal::ShareIcon));
    }

#ifdef SW_LABEL
    QString labelIconPath = getLabelIcon();
    if (!labelIconPath.isEmpty()) {
        icons << QIcon(labelIconPath);
    }
#endif

    return icons;
}

DUrl DAbstractFileInfo::parentUrl() const
{
    return DUrl::parentUrl(fileUrl());
}

DUrlList DAbstractFileInfo::parentUrlList() const
{
    QList<DUrl> urlList;

    Q_UNUSED(isAncestorsUrl(DUrl(), &urlList));

    return urlList;
}

bool DAbstractFileInfo::isAncestorsUrl(const DUrl &url, QList<DUrl> *ancestors) const
{
    DUrl parentUrl = this->parentUrl();

    forever {
        if (ancestors && parentUrl.isValid()) {
            ancestors->append(parentUrl);
        }

        if (parentUrl == url || DStorageInfo::isSameFile(parentUrl.path(), url.path())) {
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

QVector<MenuAction> DAbstractFileInfo::menuActionList(DAbstractFileInfo::MenuType type) const
{
    QVector<MenuAction> actionKeys;
//    Q_D(const DAbstractFileInfo); 未使用
    if (type == SpaceArea) {
        actionKeys.reserve(9);

        actionKeys << MenuAction::NewFolder
                   << MenuAction::NewDocument
                   << MenuAction::Separator
                   << MenuAction::DisplayAs
                   << MenuAction::SortBy
                   << MenuAction::OpenAsAdmin
                   << MenuAction::OpenInTerminal
                   << MenuAction::RefreshView
                   << MenuAction::Separator
                   << MenuAction::Paste
                   << MenuAction::SelectAll
                   << MenuAction::Separator
                   << MenuAction::Property;
    } else if (type == SingleFile) {

        if (isDir() && systemPathManager->isSystemPath(filePath())) {
            actionKeys << MenuAction::Open
                       << MenuAction::OpenInNewWindow
                       << MenuAction::OpenInNewTab
                       << MenuAction::OpenAsAdmin
                       << MenuAction::Separator
                       << MenuAction::Copy
                       << MenuAction::Separator
                       << MenuAction::Compress
                       << MenuAction::Separator;

            if (canShare() && !isShared()) {
                actionKeys << MenuAction::Share
                           << MenuAction::Separator;
            } else if (isShared()) {
                actionKeys << MenuAction::UnShare
                           << MenuAction::Separator;
            }
            actionKeys << MenuAction::CreateSymlink
                       << MenuAction::SendToDesktop
                       << MenuAction::Separator
                       << MenuAction::OpenInTerminal
                       << MenuAction::Separator;

            actionKeys  << MenuAction::Property;

        } else {
            actionKeys << MenuAction::Open;

            if (!isVirtualEntry()) {
                actionKeys << MenuAction::OpenWith;
            }

            if (isDir()) {
                actionKeys << MenuAction::OpenInNewWindow
                           << MenuAction::OpenInNewTab
                           << MenuAction::OpenAsAdmin;
            } else {
                QSet<QString> mountable = {"application/x-cd-image", "application/x-iso9660-image"};
                if (mountable.contains(mimeTypeName())) {
                    actionKeys << MenuAction::MountImage;
                }
            }
            actionKeys << MenuAction::Separator
                       << MenuAction::Cut
                       << MenuAction::Copy
                       << MenuAction::Rename;
            const_cast<DAbstractFileInfo *>(this)->checkMountFile();
            if (isGvfsMountFile() || deviceListener->isInRemovableDeviceFolder(absoluteFilePath())) {
                if (!isVirtualEntry()) {
                    actionKeys << MenuAction::CompleteDeletion;
                }
            } else {
                actionKeys << MenuAction::Delete;
            }
            actionKeys << MenuAction::Separator;

            if (isDir()) {
                actionKeys << MenuAction::Compress;

                actionKeys << MenuAction::Separator;

                if (canShare() && !isShared()) {
                    actionKeys << MenuAction::Share
                               << MenuAction::Separator;
                } else if (isShared()) {
                    actionKeys << MenuAction::UnShare
                               << MenuAction::Separator;
                }

            } else if (isFile()) {
                if (!FileUtils::isArchive(absoluteFilePath())) {
                    actionKeys << MenuAction::Compress
                               << MenuAction::Separator;
                }
            }

            if (isFile()) {
                if (FileUtils::isArchive(absoluteFilePath())) {
                    actionKeys << MenuAction::Decompress
                               << MenuAction::DecompressHere
                               << MenuAction::Separator;
                }
            }

            actionKeys << MenuAction::CreateSymlink
                       << MenuAction::SendToDesktop;

            if (deviceListener->getCanSendDisksByUrl(absoluteFilePath()).count() > 0
                    || bluetoothManager->hasAdapter()) {
                actionKeys << MenuAction::SendToRemovableDisk;
            }

            DDiskManager diskm;
            for (auto &devs : diskm.diskDevices()) {
                QScopedPointer<DDiskDevice> dev(DDiskManager::createDiskDevice(devs));
                if (dev->mediaCompatibility().join(' ').contains("_r") && dev->optical()) {
                    actionKeys << MenuAction::StageFileForBurning;
                    break;
                }
            }

            if (isDir()) {
                // FIXME: reimplement BookMark::exist() 's behavior and use it for check bookmark existance.
                //        after doing this, don't forget to remove the "bookmarkmanager.h" header file include.
                // if (DFileService::instance()->createFileInfo(nullptr, DUrl::fromBookMarkFile(fileUrl(), QString()))) {
                if (!isSymLink()) {
                    if (Singleton<BookMarkManager>::instance()->checkExist(DUrl::fromBookMarkFile(fileUrl(), QString()))) {
                        actionKeys << MenuAction::BookmarkRemove;
                    } else {
                        actionKeys << MenuAction::AddToBookMark;
                    }
                }

                actionKeys << MenuAction::Separator
                           << MenuAction::OpenInTerminal
                           << MenuAction::Separator;
            } else if (isFile()) {
                if (mimeTypeName().startsWith("image") && isReadable()
//                        && !mimeTypeName().endsWith("gif")
                        && !mimeTypeName().endsWith("svg+xml")
                        && !mimeTypeName().endsWith("raf")
                        && !mimeTypeName().endsWith("crw")) {
                    actionKeys << MenuAction::SetAsWallpaper
                               << MenuAction::Separator;
                }
            }

#ifdef SW_LABEL
            qDebug() << LabelMenuItemIds;
            foreach (QString id, LabelMenuItemIds) {
                int index = LabelMenuItemIds.indexOf(id);
                MenuAction actionType = MenuAction(MenuAction::Unknow + index + 1);
                actionKeys << actionType;
                fileMenuManger->setActionString(actionType, LabelMenuItemDatas[id].label);
                fileMenuManger->setActionID(actionType, id);
            }
            actionKeys << MenuAction::Separator;
#endif
            int size { actionKeys.size() };

            if (actionKeys[size - 1] != MenuAction::Separator) {
                actionKeys << MenuAction::Separator;
            }

            ///###: tag protocol
            actionKeys << MenuAction::TagInfo;
            actionKeys << MenuAction::TagFilesUseColor;

            actionKeys  << MenuAction::Property;
        }

    } else if (type == MultiFiles) {
        actionKeys << MenuAction::Open
                   << MenuAction::OpenWith
                   << MenuAction::Separator
                   << MenuAction::Cut
                   << MenuAction::Copy
                   << MenuAction::Rename
                   << MenuAction::Compress
                   << MenuAction::SendToDesktop;

        if (deviceListener->getCanSendDisksByUrl(absoluteFilePath()).count() > 0
                || bluetoothManager->hasAdapter()) {
            actionKeys << MenuAction::SendToRemovableDisk;
        }

        DDiskManager diskm;
        for (auto &devs : diskm.diskDevices()) {
            QScopedPointer<DDiskDevice> dev(DDiskManager::createDiskDevice(devs));
            if (dev->mediaCompatibility().join(' ').contains("_r") && dev->optical()) {
                actionKeys << MenuAction::StageFileForBurning;
                break;
            }
        }
        const_cast<DAbstractFileInfo *>(this)->checkMountFile();
        if (isGvfsMountFile() || deviceListener->isInRemovableDeviceFolder(absoluteFilePath())) {
            if (!isVirtualEntry()) {
                actionKeys << MenuAction::CompleteDeletion;
            }
        } else {
            actionKeys << MenuAction::Delete;
        }
        actionKeys << MenuAction::Separator
                   << MenuAction::Property;

        ///###: tag protocol.
        actionKeys << MenuAction::TagInfo;
        actionKeys << MenuAction::TagFilesUseColor;

        actionKeys  << MenuAction::Property;

    } else if (type == MultiFilesSystemPathIncluded) {
        actionKeys << MenuAction::Open
                   << MenuAction::Separator
                   << MenuAction::Copy
                   << MenuAction::Compress
                   << MenuAction::SendToDesktop
                   << MenuAction::Separator;

        ///###: tag protocol.
        actionKeys << MenuAction::TagInfo;
        actionKeys << MenuAction::TagFilesUseColor;

        actionKeys << MenuAction::Property;
    }

    if (isGvfsMountFile())
        actionKeys.removeAll(MenuAction::OpenAsAdmin);

    return actionKeys;
}


quint8 DAbstractFileInfo::supportViewMode() const
{
    return DFileView::AllViewMode;
}

QList<DAbstractFileInfo::SelectionMode> DAbstractFileInfo::supportSelectionModes() const
{
    return QList<SelectionMode>() << ExtendedSelection << SingleSelection << MultiSelection
           << ContiguousSelection << NoSelection;
}

void DAbstractFileInfo::setColumnCompact(bool compact)
{
    Q_D(DAbstractFileInfo);

    d->columnCompact = compact;
}

bool DAbstractFileInfo::columnIsCompact() const
{
    Q_D(const DAbstractFileInfo);

    return d->columnCompact;
}

QList<int> DAbstractFileInfo::userColumnRoles() const
{
    Q_D(const DAbstractFileInfo);

    if (d->columnCompact) {
        static QList<int> userColumnRoles {
            DFileSystemModel::FileDisplayNameRole,
            DFileSystemModel::FileUserRole + 1
        };

        return userColumnRoles;
    }

    static QList<int> userColumnRoles = QList<int>() << DFileSystemModel::FileDisplayNameRole
                                        << DFileSystemModel::FileLastModifiedRole
                                        << DFileSystemModel::FileSizeRole
                                        << DFileSystemModel::FileMimeTypeRole;

    return userColumnRoles;
}

QVariant DAbstractFileInfo::userColumnDisplayName(int userColumnRole) const
{
    Q_D(const DAbstractFileInfo);

    if (d->columnCompact && userColumnRole == DFileSystemModel::FileUserRole + 1) {
        return qApp->translate("DFileSystemModel",  "Time modified");
    }

    return DFileSystemModel::roleName(userColumnRole);
}

QVariant DAbstractFileInfo::userColumnData(int userColumnRole) const
{
    Q_D(const DAbstractFileInfo);

    if (d->columnCompact && userColumnRole == DFileSystemModel::FileUserRole + 1) {
        return QVariant::fromValue(qMakePair(lastModifiedDisplayName(), qMakePair(sizeDisplayName(), mimeTypeDisplayName())));
    }

    switch (userColumnRole) {
    case DFileSystemModel::FileLastModifiedRole:
        return lastModifiedDisplayName();
    case DFileSystemModel::FileSizeRole:
        return sizeDisplayName();
    case DFileSystemModel::FileMimeTypeRole:
        return mimeTypeDisplayName();
    case DFileSystemModel::FileCreatedRole:
        return createdDisplayName();
    default:
        break;
    }

    return QVariant();
}

QList<int> DAbstractFileInfo::userColumnChildRoles(int column) const
{
    Q_D(const DAbstractFileInfo);

    if (d->columnCompact && column == 1) {
        static QList<int> userColumnRoles {
            DFileSystemModel::FileLastModifiedRole,
            DFileSystemModel::FileSizeRole,
            DFileSystemModel::FileMimeTypeRole
        };

        return userColumnRoles;
    }

    return QList<int>();
}

int DAbstractFileInfo::userColumnWidth(int userColumnRole) const
{
    return userColumnWidth(userColumnRole, qApp->fontMetrics());
}

int DAbstractFileInfo::userColumnWidth(int userColumnRole, const QFontMetrics &fontMetrics) const
{
    switch (userColumnRole) {
    case DFileSystemModel::FileNameRole:
    case DFileSystemModel::FileDisplayNameRole:
        return -1;
    case DFileSystemModel::FileSizeRole:
        return 80;
    case DFileSystemModel::FileMimeTypeRole:
        return 80;
    default:
        return fontMetrics.width("0000/00/00 00:00:00");
    }
}

int DAbstractFileInfo::userRowHeight() const
{
    return userRowHeight(qApp->fontMetrics());
}

int DAbstractFileInfo::userRowHeight(const QFontMetrics &fontMetrics) const
{
    Q_D(const DAbstractFileInfo);

    if (d->columnCompact) {
        return fontMetrics.height() * 2 + 10;
    }

    return fontMetrics.height();
}

bool DAbstractFileInfo::columnDefaultVisibleForRole(int role) const
{
    Q_D(const DAbstractFileInfo);

    if (d->columnCompact && role == DFileSystemModel::FileUserRole + 1) {
        return true;
    }

    return !(role == DFileSystemModel::FileCreatedRole);
}

DAbstractFileInfo::CompareFunction DAbstractFileInfo::compareFunByColumn(int columnRole) const
{
    switch (columnRole) {
    case DFileSystemModel::FileDisplayNameRole:
        return FileSortFunction::compareFileListByDisplayName;
    case DFileSystemModel::FileLastModifiedRole:
        return FileSortFunction::compareFileListByModified;
    case DFileSystemModel::FileSizeRole:
        return FileSortFunction::compareFileListBySize;
    case DFileSystemModel::FileMimeTypeRole:
        return FileSortFunction::compareFileListByMime;
    case DFileSystemModel::FileCreatedRole:
        return FileSortFunction::compareFileListByCreated;
    case DFileSystemModel::FileLastReadRole:
        return FileSortFunction::compareFileListByLastRead;
    default:
        return CompareFunction();
    }
}

bool DAbstractFileInfo::hasOrderly() const
{
    return true;
}

bool DAbstractFileInfo::canRedirectionFileUrl() const
{
    return isSymLink();
}

DUrl DAbstractFileInfo::redirectedFileUrl() const
{
    if (isSymLink()) {
        return symLinkTarget();
    }

    return fileUrl();
}

bool DAbstractFileInfo::isEmptyFloder(const QDir::Filters &filters) const
{
    CALL_PROXY(isEmptyFloder(filters));

    if (!isDir()) {
        return false;
    }

    DDirIteratorPointer it = DFileService::instance()->createDirIterator(Q_NULLPTR, fileUrl(), QStringList(),
                                                                         filters,
                                                                         QDirIterator::NoIteratorFlags);

    return it && !it->hasNext();
}

Qt::ItemFlags DAbstractFileInfo::fileItemDisableFlags() const
{
    CALL_PROXY(fileItemDisableFlags());

    return Qt::ItemFlags();
}

bool DAbstractFileInfo::canIteratorDir() const
{
    return false;
}

DUrl DAbstractFileInfo::getUrlByNewFileName(const QString &fileName) const
{
    DUrl url = fileUrl();

    url.setPath(absolutePath() + QDir::separator() + fileName);

    return url;
}

DUrl DAbstractFileInfo::getUrlByChildFileName(const QString &fileName) const
{
    CALL_PROXY(getUrlByChildFileName(fileName));

    if (!isDir()) {
        return DUrl();
    }

    DUrl url = fileUrl();

    url.setPath(absoluteFilePath() + QDir::separator() + fileName);

    return url;
}

DUrl DAbstractFileInfo::mimeDataUrl() const
{
    return fileUrl();
}

Qt::DropActions DAbstractFileInfo::supportedDragActions() const
{
    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}

Qt::DropActions DAbstractFileInfo::supportedDropActions() const
{
    if (isWritable()) {
        return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
    }

    if (canDrop()) {
        return Qt::CopyAction | Qt::MoveAction;
    }

    return Qt::IgnoreAction;
}

QString DAbstractFileInfo::loadingTip() const
{
    return QObject::tr("Loading...");
}

QString DAbstractFileInfo::subtitleForEmptyFloder() const
{
    return QString();
}

QString DAbstractFileInfo::suffix() const
{
    CALL_PROXY(suffix());

    if (isDir()) {
        return QString();
    }
    // xushitong 20200424 修改后缀名获取策略为小数点后非空字符串
    const QString &strFileName = this->fileName();
    QString tmpName  = strFileName;
    int nIdx = 0;
    QString strSuffix;
    while (strSuffix.isEmpty()) {
        nIdx = tmpName.lastIndexOf(".");
        if (nIdx == -1)
            return QString();
        strSuffix = tmpName.mid(nIdx + 1);
        tmpName = tmpName.mid(0, nIdx);
    }
    return strFileName.mid(nIdx + 1);
}

QString DAbstractFileInfo::suffixOfRename() const
{
    return suffix();
}

QString DAbstractFileInfo::completeSuffix() const
{
    CALL_PROXY(completeSuffix());

    if (isDir()) {
        return QString();
    }

    const QString &fileName = this->fileName();

    int index = fileName.indexOf('.');

    if (index >= 0) {
        return fileName.mid(index + 1);
    }

    return QString();
}

void DAbstractFileInfo::makeToInactive()
{
    Q_D(DAbstractFileInfo);

    if (d->proxy) {
        d->proxy->makeToInactive();
    }

    if (!d->active) {
        return;
    }

    {
        Q_D(DAbstractFileInfo);

        d->active = false;
    }
}

DUrl DAbstractFileInfo::goToUrlWhenDeleted() const
{
    DUrl parentUrl;

    foreach (const DUrl &url, parentUrlList()) {
        parentUrl = url;

        const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(Q_NULLPTR, url);

        if (fileInfo && fileInfo->exists()) {
            break;
        }
    }

    return parentUrl.isValid() ? parentUrl : DUrl::fromLocalFile(QDir::homePath());
}

QString DAbstractFileInfo::toLocalFile() const
{
    CALL_PROXY(toLocalFile());

    if (fileUrl().isLocalFile()) {
        return fileUrl().toLocalFile();
    }

    const QFileInfo &info = toQFileInfo();

    if (!info.fileName().isEmpty()) {
        return info.absoluteFilePath();
    }

    return QString();
}

bool DAbstractFileInfo::canDrop() const
{
    if (isPrivate()) {
        return false;
    }

    if (!isSymLink()) {
        return isDir() || isDesktopFile() || (canDragCompress() && isDragCompressFileFormat());
    }

    DAbstractFileInfoPointer info(const_cast<DAbstractFileInfo *>(this));

    do {
        const DUrl &targetUrl = info->symLinkTarget();

        if (targetUrl == info->fileUrl()) {
            return false;
        }

        info = fileService->createFileInfo(Q_NULLPTR, targetUrl);

        if (!info) {
            return false;
        }
    } while (info->isSymLink());

    return info->canDrop();
}

QFileInfo DAbstractFileInfo::toQFileInfo() const
{
    CALL_PROXY(toQFileInfo());

    return QFileInfo();
}

QIODevice *DAbstractFileInfo::createIODevice() const
{
    CALL_PROXY(createIODevice());

    return 0;
}

QVariantHash DAbstractFileInfo::extraProperties() const
{
    CALL_PROXY(extraProperties());

    return QVariantHash();
}

bool DAbstractFileInfo::checkMpsStr(const QString &) const
{
    return false;
}

const QDateTime DAbstractFileInfo::getReadTime() const
{
    return QDateTime();
}

void DAbstractFileInfo::updateReadTime(const QDateTime &)
{

}

bool DAbstractFileInfo::isGvfsMountFile() const
{
    Q_D(const DAbstractFileInfo);
    if (-1 == d->gvfsMountFile) {
        return false;
    }
    return d->gvfsMountFile > 0;
}

qint8 DAbstractFileInfo::gvfsMountFile() const
{
    Q_D(const DAbstractFileInfo);
    return d->gvfsMountFile;
}

void DAbstractFileInfo::checkMountFile()
{
    Q_D(DAbstractFileInfo);
    if (-1 == gvfsMountFile()) {
        d->gvfsMountFile = FileUtils::isGvfsMountFile(absoluteFilePath());
    }
}

quint64 DAbstractFileInfo::inode() const
{
    return 0;
}

void DAbstractFileInfo::makeToActive()
{
    Q_D(DAbstractFileInfo);
    if (d->proxy) {
        d->proxy->makeToActive();
    }
    if (d->active) {
        return;
    }

    {
        Q_D(DAbstractFileInfo);

        d->active = true;
    }
}

bool DAbstractFileInfo::isActive() const
{
    CALL_PROXY(isActive());

    return d->active;
}

void DAbstractFileInfo::refresh(const bool isForce)
{
    CALL_PROXY(refresh(isForce));
#ifdef SW_LABEL
    updateLabelMenuItems();
#endif
}

DAbstractFileInfo::DAbstractFileInfo(DAbstractFileInfoPrivate &dd)
    : d_ptr(&dd)
{

}

static void onActionTriggered(QAction *action)
{
    const QString &target_dir = action->property("_fileinfo_path").toString();

    if (target_dir.isEmpty()) {
        return;
    }

    const DFileMenu *menu = qvariant_cast<DFileMenu *>(action->property("_dfm_menu"));
    const QPair<QString, QString> &data = qvariant_cast<QPair<QString, QString>>(action->data());

    const QWidget *w = menu;

    while (w->parentWidget()) {
        w = w->parentWidget();
    }

    menu = qobject_cast<const DFileMenu *>(w);

    AppController::createFile(data.first, target_dir, data.second, menu ? menu->eventId() : -1);
}

static QList<QAction *> getTemplateFileList()
{
    QList<QAction *> result;

    // blumia: Following is support for `xdg-user-dir TEMPLATES` Templates folder.
    //         It's suppored by many GNOME Nautilus based file manager. I don't think it's a good idea
    //         since we can't localization the file name text at all.
    // blumia: templateFolderPathCStr owned by glib, should NOT be freeed.
    QString templateFolderPath = DGlibUtils::userSpecialDir(DGlibUserDirectory::USER_DIRECTORY_TEMPLATES);
    if (!templateFolderPath.isEmpty()) {
        QDir templateFolder(templateFolderPath);
        if (templateFolder.exists() && templateFolder != QDir::home()) { // accroding to xdg-user-dir, dir point to home means disable.
            const QFileInfoList &templateFileInfoList = templateFolder.entryInfoList(QDir::Files | QDir::Readable | QDir::NoSymLinks);
            for (const QFileInfo &fileInfo : templateFileInfoList) {
                auto dfileinfo = DFileService::instance()->createFileInfo(nullptr, DUrl::fromLocalFile(fileInfo.filePath()));
                const QString entrySourcePath = fileInfo.absoluteFilePath();
                const QString entryText = fileInfo.baseName();
                const QString entryFileBaseName = entryText; // suffix is based on source file, only base name is okay here.
                QIcon icon = dfileinfo->fileIcon();
                QAction *action = new QAction(icon, entryText, Q_NULLPTR);
                action->setData(QVariant::fromValue(qMakePair(entrySourcePath, entryFileBaseName)));
                QObject::connect(action, &QAction::triggered, action, [action] {
                    onActionTriggered(action);
                });
                result << action;
            }
        }
    }

    // blumia: Following is support for `kf5-config --path templates` Templates folder.
    QStringList systemDataFolderList = DGlibUtils::systemDataDirs();
    QStringList templateFolderList;
    for (const QString &oneFolder : systemDataFolderList) {
        templateFolderList << (oneFolder + QStringLiteral("templates"));
    }
    // user-specific template dir
    templateFolderList << QString(DGlibUtils::userDataDir() + "templates");
    // start scan..
    for (const QString &oneTemplateFolder : templateFolderList) {
        QDir templateFolder(oneTemplateFolder);
        if (templateFolder.exists()) {
            const QStringList &templateFileList = templateFolder.entryList(QStringList(QStringLiteral("*.desktop")), QDir::Files | QDir::Readable | QDir::NoSymLinks);
            for (const QString &filePath : templateFileList) {
                DDesktopEntry desktopFile(templateFolder.absoluteFilePath(filePath));
                const QString entrySourcePath = templateFolder.absoluteFilePath(desktopFile.stringValue("URL"));
                const QString entryText = desktopFile.localizedValue("Name");
                const QString entryFileBaseName = entryText; // suffix is based on source file, only base name is okay here.
                if (!QFileInfo(entrySourcePath).exists() || entryText.isEmpty()) {
                    continue; // holy shit!
                }
                qDebug() << templateFolder.absoluteFilePath(filePath) << entrySourcePath;
                QIcon icon = QIcon::fromTheme(desktopFile.stringValue("Icon"));
                QAction *action = new QAction(icon, entryText, Q_NULLPTR);
                action->setData(QVariant::fromValue(qMakePair(entrySourcePath, entryFileBaseName)));
                QObject::connect(action, &QAction::triggered, action, [action] {
                    onActionTriggered(action);
                });
                result << action;
            }
        }
    }

    return result;
}

static QVector<MenuAction> getMenuActionTypeListByAction(const QList<QAction *> &list)
{
    QVector<MenuAction> type_list;

    type_list.reserve(type_list.size());

    for (QAction *action : list) {
        type_list.append(DFileMenuManager::registerMenuActionType(action));
    }

    return type_list;
}

QMap<MenuAction, QVector<MenuAction> > DAbstractFileInfo::subMenuActionList(MenuType type) const
{
    Q_UNUSED(type)
    QMap<MenuAction, QVector<MenuAction> > actions;

    QVector<MenuAction> openwithMenuActionKeys;
    actions.insert(MenuAction::OpenWith, openwithMenuActionKeys);


    QVector<MenuAction> docmentMenuActionKeys;
#ifdef DFM_MINIMUM
    docmentMenuActionKeys << MenuAction::NewText;
#else
    docmentMenuActionKeys << MenuAction::NewWord
                          << MenuAction::NewExcel
                          << MenuAction::NewPowerpoint
                          << MenuAction::NewText;
#endif

    static const QList<QAction *> template_file_list = getTemplateFileList();
    static const QVector<MenuAction> action_type_list = getMenuActionTypeListByAction(template_file_list);

    QString urlStr = toLocalFile();
    if (urlStr.isEmpty()) {
#ifdef QT_DEBUG
        qCritical() << fileUrl() << "scheme fileinfo seems doesn't implement toLocalFile() or implemented incorrectly.";
        qCritical() << "This can be a bug and should be fixed!!!!!!!!";
#endif
        urlStr = fileUrl().toLocalFile();
    }

    for (QAction *action : template_file_list) {
        action->setProperty("_fileinfo_path", urlStr);
    }

    docmentMenuActionKeys << action_type_list;
    actions.insert(MenuAction::NewDocument, docmentMenuActionKeys);

    QVector<MenuAction> displayAsMenuActionKeys;

    int support_view_mode = supportViewMode();

    if ((support_view_mode & DListView::IconMode) == DListView::IconMode) {
        displayAsMenuActionKeys << MenuAction::IconView;
    }

    if ((support_view_mode & DListView::ListMode) == DListView::ListMode) {
        displayAsMenuActionKeys << MenuAction::ListView;
    }

    actions.insert(MenuAction::DisplayAs, displayAsMenuActionKeys);

    QVector<MenuAction> sortByMenuActionKeys;
//    sortByMenuActionKeys << MenuAction::Name;

    for (int role : sortSubMenuActionUserColumnRoles()) {
        sortByMenuActionKeys << menuActionByColumnRole(role);
    }

    actions.insert(MenuAction::SortBy, sortByMenuActionKeys);

    if (deviceListener->isMountedRemovableDiskExits()
            || bluetoothManager->hasAdapter()) {
        QVector<MenuAction> diskMenuActionKeys;
        actions.insert(MenuAction::SendToRemovableDisk, diskMenuActionKeys);
    }

    DDiskManager diskm;
    for (auto &drvs : diskm.diskDevices()) {
        QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(drvs));
        if (drv->mediaCompatibility().join(' ').contains("_r") && drv->optical()) {
            actions.insert(MenuAction::StageFileForBurning, {});
            break;
        }
    }

    return actions;
}

QSet<MenuAction> DAbstractFileInfo::disableMenuActionList() const
{
    QSet<MenuAction> list;

    if (!isWritable()) {
        list << MenuAction::NewFolder
             << MenuAction::NewDocument
             << MenuAction::Paste;
    }

    if (!canRename()) {
        list << MenuAction::Cut << MenuAction::Rename << MenuAction::Delete << MenuAction::CompleteDeletion;
    }

    if (isVirtualEntry() || (!isReadable() && !isSymLink())) {
        list << MenuAction::Copy;
    }

    return list;
}

MenuAction DAbstractFileInfo::menuActionByColumnRole(int role) const
{
    switch (role) {
    case DFileSystemModel::FileDisplayNameRole:
    case DFileSystemModel::FileNameRole:
        return MenuAction::Name;
    case DFileSystemModel::FileSizeRole:
        return MenuAction::Size;
    case DFileSystemModel::FileMimeTypeRole:
        return MenuAction::Type;
    case DFileSystemModel::FileCreatedRole:
        return MenuAction::CreatedDate;
    case DFileSystemModel::FileLastModifiedRole:
        return MenuAction::LastModifiedDate;
    case DFileSystemModel::FileLastReadRole:
        return MenuAction::LastRead;
    default:
        return MenuAction::Unknow;
    }
}

QList<int> DAbstractFileInfo::sortSubMenuActionUserColumnRoles() const
{
    Q_D(const DAbstractFileInfo);

    if (!d->columnCompact) {
        return userColumnRoles();
    }

    QList<int> roles;

    int column = 0;

    for (int role : userColumnRoles()) {
        const QList<int> child_roles = userColumnChildRoles(column++);

        if (child_roles.isEmpty()) {
            roles << role;
        } else {
            roles << child_roles;
        }
    }

    return roles;
}

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug deg, const DAbstractFileInfo &info)
{
    deg << "file url:" << info.fileUrl()
        << "mime type:" << info.mimeTypeName();

    return deg;
}
QT_END_NAMESPACE

void DAbstractFileInfo::setProxy(const DAbstractFileInfoPointer &proxy)
{
    Q_D(DAbstractFileInfo);

    d->proxy = proxy;
}

void DAbstractFileInfo::setUrl(const DUrl &url)
{
    Q_D(DAbstractFileInfo);

    d->setUrl(url, false);
}

bool DAbstractFileInfo::loadFileEmblems(QList<QIcon> &iconList) const
{
    //如果没有位置可以显示徽标，则不显示
    if (iconList.length() >= 4) {
        return false;
    }

    std::string str = filePath().toStdString();

    //获取gfileinfo
    GFile *g_file = g_file_new_for_path(str.c_str());
    GError *g_error = nullptr;
    GFileInfo *g_fileInfo = g_file_query_info(g_file, "metadata::emblems", GFileQueryInfoFlags::G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, nullptr, &g_error);

    if (g_error != nullptr) {
        //report error
        g_object_unref(g_file);
        g_error_free(g_error);
        return false;
    }

    //通过gfileinfo获取文件徽标的值
    char **emblemStr = g_file_info_get_attribute_stringv(g_fileInfo, "metadata::emblems");
    if (!emblemStr) {
        g_object_unref(g_fileInfo);
        g_object_unref(g_file);
        return false;
    }

    QString emStr(*emblemStr);
    g_object_unref(g_fileInfo);
    g_object_unref(g_file);
    if (!emStr.isEmpty()) {
        QList<QIcon> newIcons = {QIcon(), QIcon(), QIcon(), QIcon()};
        //设置了多条徽标的情况
        if (emStr.contains("|")) {
            QStringList emblems = emStr.split("|");
            for (int i = 0; i < emblems.length(); i++) {
                QString pos;
                QIcon emblem;
                if (parseEmblemString(emblem, pos, emblems.at(i))) {
                    setEmblemIntoIcons(pos, emblem, newIcons);
                }
            }
        }
        //只设置了一条徽标的情况
        else {
            QString pos;
            QIcon emblem;
            if (parseEmblemString(emblem, pos, emStr)) {
                setEmblemIntoIcons(pos, emblem, newIcons);
            }
        }

        for (int i = 0; i < iconList.length(); i++) {
            newIcons[i] = iconList.at(i);
        }

        iconList = newIcons;

        return true;
    }

    return false;
}

bool DAbstractFileInfo::parseEmblemString(QIcon &emblem, QString &pos, const QString &emblemStr) const
{
    //默认位置在右下
    pos = "rd";

    if (!emblemStr.isEmpty()) {
        QIcon emblemIcon;
        QString imgPath;
        //位置参数和徽标图标由 ; 隔开
        if (emblemStr.contains(";")) {
            QStringList emStrList = emblemStr.split(";");
            imgPath = emStrList.at(0);
            pos = emStrList.at(1);
        } else {
            imgPath = emblemStr;
        }

        //修正主目录为标准路径
        if (imgPath.startsWith("~/")) {
            imgPath.replace(0, 1, QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
        }

//        QFile imgFile(imgPath);
        QFileInfo fileInfo(imgPath);
        if (fileInfo.exists()) {
            if (fileInfo.size() > 102400) { //图片大小大于100不显示
                return false;
            }

            //只支持部分格式的图片作为徽标图源
            if (fileInfo.completeSuffix() != "svg" &&
                    fileInfo.completeSuffix() != "png" &&
                    fileInfo.completeSuffix() != "gif" &&
                    fileInfo.completeSuffix() != "bmp" &&
                    fileInfo.completeSuffix() != "jpg") {
                return false;
            }

            emblemIcon = QIcon(imgPath);
            if (!emblemIcon.isNull()) {
                emblem = emblemIcon;
                return true;
            }
        }
    }

    return false;
}

void DAbstractFileInfo::setEmblemIntoIcons(const QString &pos, const QIcon &emblem, QList<QIcon> &iconList) const
{
    int emblemIndex = 0;    //徽标目标位置,默认位置右下rd=0

    //左下
    if (pos == "ld") {
        emblemIndex = 1;
    }
    //左上
    else if (pos == "lu") {
        emblemIndex = 2;
    }
    //右上
    else if (pos == "ru") {
        emblemIndex = 3;
    }

    iconList[emblemIndex] = emblem;
}

#ifdef SW_LABEL
QString DAbstractFileInfo::getLabelIcon() const
{
    if (FileManagerLibrary::instance()->isCompletion()) {
        std::string path = fileUrl().toLocalFile().toStdString();
//        char* icon = auto_add_emblem(const_cast<char*>(path.c_str()));
        char *icon = FileManagerLibrary::instance()->auto_add_emblem()(const_cast<char *>(path.c_str()));
        if (QString::fromLocal8Bit(icon) == "No") {
            return "";
        } else {
            return QString::fromLocal8Bit(icon);
        }
    }
    return "";
}

void DAbstractFileInfo::updateLabelMenuItems()
{
    if (FileManagerLibrary::instance()->isCompletion()) {
        LabelMenuItemIds.clear();
        LabelMenuItemDatas.clear();
        //    QString menu = "{\"id\":[\"010101\",\"010201\",\"010501\"],\"label\":[\"查看标签\",\"编辑标签\",\"转换为公有\"],\"tip\":[\"\",\"\",\"\"],\"icon\":[\"viewlabel.svg\",\"editlabel.svg\",\"editlabel.svg\"],\"sub0\":\"\",\"sub1\":\"\",\"sub2\":\"\"}";
        //    QString menu = "{\"id\":[\"020100\"],\"label\":[\"设置标签\"],\"tip\":[\"\"],\"icon\":[\"setlabel.svg\"],\"sub0\":\"\"}";
        std::string path = fileUrl().toLocalFile().toStdString();
//        QString menu = auto_add_rightmenu(const_cast<char*>(path.c_str()));
        QString menu = FileManagerLibrary::instance()->auto_add_rightmenu()(const_cast<char *>(path.c_str()));
        qDebug() << menu;
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(menu.toLocal8Bit(), &error);
        if (error.error == QJsonParseError::NoError) {
            QJsonObject obj = doc.object();
            QJsonArray ids =  obj.value("id").toArray();
            QJsonArray labels =  obj.value("label").toArray();
            QJsonArray tips =  obj.value("tip").toArray();
            QJsonArray icons =  obj.value("icon").toArray();

            for (int i = 0; i < ids.count(); i++) {
                LabelMenuItemData item;
                item.id = ids.at(i).toString();
                item.label = labels.at(i).toString();
                item.tip = tips.at(i).toString();
                item.icon = icons.at(i).toString();
                LabelMenuItemIds.append(item.id);
                LabelMenuItemDatas.insert(item.id, item);
                qDebug() << item.id << item.icon << item.label;
            }
        } else {
            qDebug() << "load menu fail: " << error.errorString();
        }
    }
}
#endif
