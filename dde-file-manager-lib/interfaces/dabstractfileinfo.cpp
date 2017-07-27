#include "dabstractfileinfo.h"
#include "private/dabstractfileinfo_p.h"
#include "views/dfileview.h"
#include "dfilemenu.h"

#include "dfilesystemmodel.h"
#include "dfilemenumanager.h"

#include "shutil/fileutils.h"
#include "shutil/mimetypedisplaymanager.h"

#include "controllers/pathmanager.h"
#include "dfileservices.h"
#include "dmimedatabase.h"

#include "app/define.h"
#include "controllers/appcontroller.h"

#include "singleton.h"
#include "plugins/pluginmanager.h"
#include "dde-file-manager-plugins/plugininterfaces/menu/menuinterface.h"

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

#ifdef SW_LABEL
#include "sw_label/filemanagerlibrary.h"
#include "dfilemenumanager.h"
#endif


namespace FileSortFunction {
QCollator sortCollator;

bool compareByString(const QString &str1, const QString &str2, Qt::SortOrder order)
{
    if (DFMGlobal::startWithHanzi(str1)) {
        if (!DFMGlobal::startWithHanzi(str2))
            return order == Qt::DescendingOrder;
    } else if (DFMGlobal::startWithHanzi(str2)) {
        return order != Qt::DescendingOrder;
    }

    return ((order == Qt::DescendingOrder) ^ (sortCollator.compare(str1, str2) < 0)) == 0x01;
}

COMPARE_FUN_DEFINE(fileDisplayName, DisplayName, DAbstractFileInfo)
COMPARE_FUN_DEFINE(fileSize, Size, DAbstractFileInfo)
COMPARE_FUN_DEFINE(lastModified, Modified, DAbstractFileInfo)
COMPARE_FUN_DEFINE(fileType, Mime, DAbstractFileInfo)
COMPARE_FUN_DEFINE(created, Created, DAbstractFileInfo)
} /// end namespace FileSortFunction

#define CALL_PROXY(Fun)\
    Q_D(const DAbstractFileInfo);\
    if (d->proxy) return d->proxy->Fun;

QMap<DUrl, DAbstractFileInfo*> DAbstractFileInfoPrivate::urlToFileInfoMap;
QReadWriteLock *DAbstractFileInfoPrivate::urlToFileInfoMapLock = new QReadWriteLock();
DMimeDatabase DAbstractFileInfoPrivate::mimeDatabase;

DAbstractFileInfoPrivate::DAbstractFileInfoPrivate(const DUrl &url, DAbstractFileInfo *qq, bool hasCache)
    : q_ptr(qq)
    , fileUrl(url)
{
    //###(zccrs): 只在主线程中开启缓存，防止不同线程中持有同一对象时的竞争问题
    if (hasCache && QThread::currentThread() == qApp->thread()) {
        QWriteLocker locker(urlToFileInfoMapLock);
        Q_UNUSED(locker)

        urlToFileInfoMap[url] = qq;
    }

    FileSortFunction::sortCollator.setNumericMode(true);
    FileSortFunction::sortCollator.setCaseSensitivity(Qt::CaseInsensitive);
}

DAbstractFileInfoPrivate::~DAbstractFileInfoPrivate()
{
    if (urlToFileInfoMap.value(fileUrl) == q_ptr) {
        QWriteLocker locker(urlToFileInfoMapLock);
        Q_UNUSED(locker)
        urlToFileInfoMap.remove(fileUrl);
    }
}

void DAbstractFileInfoPrivate::setUrl(const DUrl &url, bool hasCache)
{
    if (url == fileUrl)
        return;

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
    //###(zccrs): 只在主线程中开启缓存，防止不同线程中持有同一对象时的竞争问题
    if (QThread::currentThread() != qApp->thread())
        return 0;

    return urlToFileInfoMap.value(fileUrl);
}

DAbstractFileInfo::DAbstractFileInfo(const DUrl &url, bool hasCache)
    : d_ptr(new DAbstractFileInfoPrivate(url, this, hasCache))
{

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

QString DAbstractFileInfo::path() const
{
    CALL_PROXY(path());

    const QString &filePath = this->filePath();

    int index = filePath.lastIndexOf(QDir::separator());

    if (index >= 0)
        return filePath.left(index);

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

    if (isAbsolute())
        return path();

    QFileInfo info(filePath());

    return info.absolutePath();
}

QString DAbstractFileInfo::absoluteFilePath() const
{
    CALL_PROXY(absoluteFilePath());

    if (isAbsolute())
        return filePath();

    QFileInfo info(filePath());

    return info.absoluteFilePath();
}

QString DAbstractFileInfo::baseName() const
{
    CALL_PROXY(baseName());

    const QString &fileName = this->fileName();
    const QString &suffix = this->suffix();

    if (suffix.isEmpty())
        return fileName;

    return fileName.left(fileName.length() - suffix.length() - 1);
}

QString DAbstractFileInfo::fileName() const
{
    CALL_PROXY(fileName());

    QString filePath = this->filePath();

    if (filePath.endsWith(QDir::separator()))
        filePath.chop(1);

    int index = filePath.lastIndexOf(QDir::separator());

    if (index >= 0)
        return filePath.mid(index + 1);

    return filePath;
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

    if (d->pinyinName.isEmpty())
        d->pinyinName = DFMGlobal::toPinyin(diaplayName);

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

    return isDir();
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
        if (targetUrl == fileUrl()){
            break;
        }
        info = fileService->createFileInfo(Q_NULLPTR, targetUrl);
        if (!info){
            return fileUrl();
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

    if (!iterator)
        return -1;

    while (iterator->hasNext()) {
        iterator->next();

        ++count;
    }

    return count;
}

qint64 DAbstractFileInfo::fileSize() const
{
    if (isDir())
        return filesCount();

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

        if (size <= 1){
            return QObject::tr("%1 item").arg(size);
        }else{
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

QString DAbstractFileInfo::fileType() const
{
    CALL_PROXY(fileType());

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

    if (icon.isNull())
        icon = QIcon::fromTheme("application-default-icon");

    return icon;
}

QList<QIcon> DAbstractFileInfo::additionalIcon() const
{
    QList<QIcon> icons;

    if (isSymLink()) {
        icons << QIcon::fromTheme("emblem-symbolic-link", DFMGlobal::instance()->standardIcon(DFMGlobal::LinkIcon));
    }

    if (!isWritable())
        icons << QIcon::fromTheme("emblem-readonly", DFMGlobal::instance()->standardIcon(DFMGlobal::LockIcon));

    if (!isReadable())
        icons << QIcon::fromTheme("emblem-unreadable", DFMGlobal::instance()->standardIcon(DFMGlobal::UnreadableIcon));

    if (isShared())
        icons << QIcon::fromTheme("emblem-shared", DFMGlobal::instance()->standardIcon(DFMGlobal::ShareIcon));

    foreach (MenuInterface* menuInterface, PluginManager::instance()->getMenuInterfaces()) {
        QList<QIcon> pluginIcons = menuInterface->additionalIcons(filePath());
        foreach (const QIcon &icon, pluginIcons) {
            icons << icon;
        }
    }

#ifdef SW_LABEL
    QString labelIconPath = getLabelIcon();
    if (!labelIconPath.isEmpty()){
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
        if (ancestors && parentUrl.isValid())
            ancestors->append(parentUrl);

        if (parentUrl == url)
            return true;

        const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(Q_NULLPTR, parentUrl);

        if (!fileInfo)
            break;

        const DUrl &pu = fileInfo->parentUrl();

        if (pu == parentUrl)
            break;

        parentUrl = pu;
    }

    return false;
}

QVector<MenuAction> DAbstractFileInfo::menuActionList(DAbstractFileInfo::MenuType type) const
{
    QVector<MenuAction> actionKeys;

    if (type == SpaceArea) {
        actionKeys.reserve(9);

        actionKeys << MenuAction::NewFolder
                   << MenuAction::NewDocument
                   << MenuAction::Separator
                   << MenuAction::DisplayAs
                   << MenuAction::SortBy
                   << MenuAction::OpenAsAdmin
                   << MenuAction::OpenInTerminal
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
                       << MenuAction::Separator
                       << MenuAction::Copy
                       << MenuAction::Separator
                       << MenuAction::Compress
                       << MenuAction::Separator;

            if (canShare() && !isShared()){
                actionKeys << MenuAction::Share
                           << MenuAction::Separator;
            }else if(isShared()){
                actionKeys << MenuAction::UnShare
                           << MenuAction::Separator;
            }
            actionKeys << MenuAction::CreateSymlink
                       << MenuAction::SendToDesktop
                       << MenuAction::Separator
                       << MenuAction::OpenAsAdmin
                       << MenuAction::OpenInTerminal
                       << MenuAction::Separator;

            actionKeys  << MenuAction::Property;

        } else {
            actionKeys << MenuAction::Open;

            if (isDir()){
                actionKeys << MenuAction::OpenInNewWindow
                           << MenuAction::OpenInNewTab;
            }else{
                actionKeys << MenuAction::OpenWith;
            }
            actionKeys << MenuAction::Separator
                       << MenuAction::Cut
                       << MenuAction::Copy
                       << MenuAction::Rename
                       << MenuAction::Delete
                       << MenuAction::Separator;

            if (isDir()) {
                actionKeys << MenuAction::Compress;

                actionKeys << MenuAction::Separator;

                if (canShare() && !isShared()){
                    actionKeys << MenuAction::Share
                               << MenuAction::Separator;
                }else if(isShared()){
                    actionKeys << MenuAction::UnShare
                               << MenuAction::Separator;
                }

            }else if(isFile()) {
                if (!FileUtils::isArchive(absoluteFilePath())){
                    actionKeys << MenuAction::Compress
                               << MenuAction::Separator;
                }
            }

            if (isFile()){
                if (FileUtils::isArchive(absoluteFilePath())){
                    actionKeys << MenuAction::Decompress
                               << MenuAction::DecompressHere
                               << MenuAction::Separator;
                }
            }

            actionKeys << MenuAction::CreateSymlink
                       << MenuAction::SendToDesktop;

            if (isDir()) {
                actionKeys << MenuAction::AddToBookMark
                           << MenuAction::Separator
                           << MenuAction::OpenAsAdmin
                           << MenuAction::OpenInTerminal
                           << MenuAction::Separator;
            } else if(isFile()) {
                if (mimeTypeName().startsWith("image") && isReadable()) {
                    actionKeys << MenuAction::SetAsWallpaper
                               << MenuAction::Separator;
                }
            }

#ifdef SW_LABEL
            qDebug() << m_labelMenuItemIds;
//            if (m_labelMenuItemIds.length() == 1){
//                foreach (QString id, m_labelMenuItemIds) {
//                    if(id == "020100"){
//                        fileMenuManger->setActionString(MenuAction::SetLabel, m_labelMenuItemData[id].label);
//                    }
//                }
//                actionKeys << MenuAction::SetLabel;
//            }else{
//                foreach (QString id, m_labelMenuItemIds) {
//                    if(id == "010101"){
//                        actionKeys << MenuAction::ViewLabel;
//                        fileMenuManger->setActionString(MenuAction::ViewLabel, m_labelMenuItemData[id].label);
//                    }else if(id == "010201"){
//                        actionKeys << MenuAction::EditLabel;
//                        fileMenuManger->setActionString(MenuAction::EditLabel, m_labelMenuItemData[id].label);
//                    }else if(id == "010501"){
//                        actionKeys << MenuAction::PrivateFileToPublic;
//                        fileMenuManger->setActionString(MenuAction::PrivateFileToPublic, m_labelMenuItemData[id].label);
//                    }
//                }
//            }

            foreach (QString id, m_labelMenuItemIds) {
                int index = m_labelMenuItemIds.indexOf(id);
                MenuAction actionType = MenuAction (MenuAction::Unknow + index + 1);
                actionKeys << actionType;
                fileMenuManger->setActionString(actionType, m_labelMenuItemData[id].label);
                fileMenuManger->setActionID(actionType, id);
            }

            actionKeys << MenuAction::Separator;
#endif

            actionKeys  << MenuAction::Property;
        }
    } else if (type == MultiFiles) {
        actionKeys << MenuAction::Open
                   << MenuAction::Separator
                   << MenuAction::Cut
                   << MenuAction::Copy
                   << MenuAction::Compress
                   << MenuAction::SendToDesktop
                   << MenuAction::Delete
                   << MenuAction::Separator
                   << MenuAction::Property;
    } else if (type == MultiFilesSystemPathIncluded) {
        actionKeys << MenuAction::Open
                   << MenuAction::Separator
                   << MenuAction::Copy
                   << MenuAction::Compress
                   << MenuAction::SendToDesktop
                   << MenuAction::Separator
                   << MenuAction::Property;
    }

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

QList<int> DAbstractFileInfo::userColumnRoles() const
{
    static QList<int> userColumnRoles = QList<int>() << DFileSystemModel::FileLastModifiedRole
                                                     << DFileSystemModel::FileSizeRole
                                                     << DFileSystemModel::FileMimeTypeRole;

    return userColumnRoles;
}

QVariant DAbstractFileInfo::userColumnDisplayName(int userColumnRole) const
{
    return DFileSystemModel::roleName(userColumnRole);
}

QVariant DAbstractFileInfo::userColumnData(int userColumnRole) const
{
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

int DAbstractFileInfo::userColumnWidth(int userColumnRole) const
{
    return userColumnWidth(userColumnRole, qApp->fontMetrics());
}

int DAbstractFileInfo::userColumnWidth(int userColumnRole, const QFontMetrics &fontMetrics) const
{
    switch (userColumnRole) {
    case DFileSystemModel::FileSizeRole:
        return 80;
    case DFileSystemModel::FileMimeTypeRole:
        return 80;
    default:
        return fontMetrics.width("0000/00/00 00:00:00");
    }
}

bool DAbstractFileInfo::columnDefaultVisibleForRole(int role) const
{
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
    if (isSymLink())
        return symLinkTarget();

    return fileUrl();
}

bool DAbstractFileInfo::isEmptyFloder(const QDir::Filters &filters) const
{
    CALL_PROXY(isEmptyFloder(filters));

    if (!isDir())
        return false;

    DDirIteratorPointer it = DFileService::instance()->createDirIterator(Q_NULLPTR, fileUrl(), QStringList(),
                                                                         filters,
                                                                         QDirIterator::NoIteratorFlags);

    return it && !it->hasNext();
}

Qt::ItemFlags DAbstractFileInfo::fileItemDisableFlags() const
{
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
    if (!isDir())
        return DUrl();

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
    if (isWritable())
        return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;

    if (canDrop())
        return Qt::CopyAction | Qt::MoveAction;

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

    if (!isFile())
        return QString();

    return d->mimeDatabase.suffixForFileName(this->fileName());
}

QString DAbstractFileInfo::completeSuffix() const
{
    CALL_PROXY(completeSuffix());

    if (!isFile())
        return QString();

    const QString &fileName = this->fileName();

    int index = fileName.indexOf('.');

    if (index >= 0)
        return fileName.mid(index);

    return QString();
}

void DAbstractFileInfo::makeToInactive()
{
    CALL_PROXY(makeToInactive());

    if (!d->active)
        return;
}

DUrl DAbstractFileInfo::goToUrlWhenDeleted() const
{
    CALL_PROXY(goToUrlWhenDeleted());

    DUrl extistParentUrl;

    foreach (const DUrl &url, parentUrlList()) {
        extistParentUrl = url;

        const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(Q_NULLPTR, url);

        if (fileInfo && fileInfo->exists()) {
            break;
        }
    }

    return extistParentUrl.isValid() ? extistParentUrl : DUrl::fromLocalFile(QDir::homePath());
}

QString DAbstractFileInfo::toLocalFile() const
{
    return fileUrl().isLocalFile() ? fileUrl().toLocalFile() : QString();
}

bool DAbstractFileInfo::canDrop() const
{
    if (isDir()) {
        return true;
    }

    DAbstractFileInfoPointer info(const_cast<DAbstractFileInfo*>(this));

    while (info->isSymLink()) {
        const DUrl &targetUrl = info->symLinkTarget();

        if (targetUrl == fileUrl()) {
            break;
        }

        info = fileService->createFileInfo(Q_NULLPTR, targetUrl);

        if (!info) {
            return false;
        }
    }

    return info->suffix() == "desktop";
}

void DAbstractFileInfo::makeToActive()
{
    CALL_PROXY(makeToActive());

    if (d->active)
        return;
}

bool DAbstractFileInfo::isActive() const
{
    CALL_PROXY(isActive());

    return d->active;
}

void DAbstractFileInfo::refresh()
{
    CALL_PROXY(refresh());
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

    if (target_dir.isEmpty())
        return;

    const DFileMenu *menu = qvariant_cast<DFileMenu*>(action->property("_dfm_menu"));
    const QPair<QString, QString> &data = qvariant_cast<QPair<QString, QString>>(action->data());

    const QWidget *w = menu;

    while (w->parentWidget())
        w = w->parentWidget();

    menu = qobject_cast<const DFileMenu*>(w);

    AppController::createFile(data.first, target_dir, data.second, menu ? menu->eventId() : -1);
}

static QList<QAction*> getTemplateFileList()
{
    QList<QAction*> result;

    const QString &path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir template_dir(path + "/templates");
    const QString &old_current_path = QDir::currentPath();

    // set current path
    QDir::setCurrent(template_dir.absolutePath());

    const QStringList & config_list = template_dir.entryList(QStringList(QStringLiteral("*.template")), QDir::Files | QDir::Readable | QDir::NoSymLinks);

    result.reserve(config_list.size());

    for (const QString &file : config_list) {
        Properties settings(template_dir.absoluteFilePath(file), "DFM Template Entry");

        const QString &text = settings.value(QString("Text[%1]").arg(QLocale::system().name()), settings.value("Text")).toString();
        if (text.isEmpty()) {
            qWarning() << "Invalid template config file(The \"Text\" section is empty):" << file;
            continue;
        }

        QString source = settings.value("Source").toString();

        if (source.isEmpty()) {
            qWarning() << "Invalid template config file(The \"Source\" section is empty):" << file;
            continue;
        }

        if (source.contains("/")) {
            qWarning() << "Invalid template config file(The \"Source\" section is invalid, Can't contains the '/' character'):" << file;
            continue;
        }

        source = template_dir.absoluteFilePath(source);

        if (!QFile::exists(source)) {
            qWarning() << QString("Invalid template config file(The \"%1\" file is't exists):").arg(source) << file;
            continue;
        }

        const QString &new_file_base_name = settings.value(QString("BaseName[%1]").arg(QLocale::system().name()), settings.value("BaseName")).toString();

        if (new_file_base_name.isEmpty()) {
            qWarning() << "Invalid template config file(The \"BaseName\" section is empty):" << file;
            continue;
        }

        if (new_file_base_name.contains("/")) {
            qWarning() << "Invalid template config file(The \"BaseName\" section is invalid, Can't contains the '/' character'):" << file;
            continue;
        }

        const QString &icon_name = settings.value("Icon").toString();

        QAction *action = new QAction(text, Q_NULLPTR);

        if (!icon_name.isEmpty()) {
            QIcon icon = QIcon::fromTheme(icon_name);

            if (icon.isNull())
                icon = QIcon(icon_name);

            if (!icon.isNull())
                action->setIcon(icon);
        }

        action->setData(QVariant::fromValue(qMakePair(source, new_file_base_name)));
        QObject::connect(action, &QAction::triggered, action, [action] {
            onActionTriggered(action);
        });

        result << action;
    }

    // restore current path
    QDir::setCurrent(old_current_path);

    return result;
}

static QVector<MenuAction> getMenuActionTypeListByAction(const QList<QAction*> &list)
{
    QVector<MenuAction> type_list;

    type_list.reserve(type_list.size());

    for (QAction *action : list) {
        type_list.append(DFileMenuManager::registerMenuActionType(action));
    }

    return type_list;
}

QMap<MenuAction, QVector<MenuAction> > DAbstractFileInfo::subMenuActionList() const
{
    QMap<MenuAction, QVector<MenuAction> > actions;

    QVector<MenuAction> openwithMenuActionKeys;
    actions.insert(MenuAction::OpenWith, openwithMenuActionKeys);


    QVector<MenuAction> docmentMenuActionKeys;
    docmentMenuActionKeys << MenuAction::NewWord
                          << MenuAction::NewExcel
                          << MenuAction::NewPowerpoint
                          << MenuAction::NewText;

    static const QList<QAction*> template_file_list = getTemplateFileList();
    static const QVector<MenuAction> action_type_list = getMenuActionTypeListByAction(template_file_list);

    for (QAction *action : template_file_list) {
        action->setProperty("_fileinfo_path", fileUrl().toLocalFile());
    }

    docmentMenuActionKeys << action_type_list;
    actions.insert(MenuAction::NewDocument, docmentMenuActionKeys);

    QVector<MenuAction> displayAsMenuActionKeys;

    int support_view_mode = supportViewMode();

    if ((support_view_mode & DListView::IconMode) == DListView::IconMode)
        displayAsMenuActionKeys << MenuAction::IconView;

    if ((support_view_mode & DListView::ListMode) == DListView::ListMode)
        displayAsMenuActionKeys << MenuAction::ListView;

    actions.insert(MenuAction::DisplayAs, displayAsMenuActionKeys);

    QVector<MenuAction> sortByMenuActionKeys;
    sortByMenuActionKeys << MenuAction::Name;

    for (int role : userColumnRoles()) {
        sortByMenuActionKeys << menuActionByColumnRole(role);
    }

    actions.insert(MenuAction::SortBy, sortByMenuActionKeys);

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
        list << MenuAction::Cut << MenuAction::Rename << MenuAction::Delete;
    }

    if (FileUtils::isGvfsMountFile(absoluteFilePath())){
        list << MenuAction::Delete;
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
    default:
        return MenuAction::Unknow;
    }
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

#ifdef SW_LABEL
QString DAbstractFileInfo::getLabelIcon() const
{
    if (FileManagerLibrary::instance()->isCompletion()){
        std::string path = fileUrl().toLocalFile().toStdString();
//        char* icon = auto_add_emblem(const_cast<char*>(path.c_str()));
        char* icon = FileManagerLibrary::instance()->auto_add_emblem()(const_cast<char*>(path.c_str()));
        if (QString::fromLocal8Bit(icon) == "No"){
            return "";
        }else{
            return QString::fromLocal8Bit(icon);
        }
    }
    return "";
}

void DAbstractFileInfo::updateLabelMenuItems()
{
    if (FileManagerLibrary::instance()->isCompletion()){
        m_labelMenuItemIds.clear();
        m_labelMenuItemData.clear();
    //    QString menu = "{\"id\":[\"010101\",\"010201\",\"010501\"],\"label\":[\"查看标签\",\"编辑标签\",\"转换为公有\"],\"tip\":[\"\",\"\",\"\"],\"icon\":[\"viewlabel.svg\",\"editlabel.svg\",\"editlabel.svg\"],\"sub0\":\"\",\"sub1\":\"\",\"sub2\":\"\"}";
    //    QString menu = "{\"id\":[\"020100\"],\"label\":[\"设置标签\"],\"tip\":[\"\"],\"icon\":[\"setlabel.svg\"],\"sub0\":\"\"}";
        std::string path = fileUrl().toLocalFile().toStdString();
//        QString menu = auto_add_rightmenu(const_cast<char*>(path.c_str()));
        QString menu = FileManagerLibrary::instance()->auto_add_rightmenu()(const_cast<char*>(path.c_str()));
        QJsonParseError error;
        QJsonDocument doc=QJsonDocument::fromJson(menu.toLocal8Bit(),&error);
        if (error.error == QJsonParseError::NoError){
            QJsonObject obj = doc.object();
            QJsonArray ids =  obj.value("id").toArray();
            QJsonArray labels =  obj.value("label").toArray();
            QJsonArray tips =  obj.value("tip").toArray();
            QJsonArray icons =  obj.value("icon").toArray();

            for(int i=0; i< ids.count(); i++){
                LabelMenuItemData item;
                item.id = ids.at(i).toString();
                item.label = labels.at(i).toString();
                item.tip = tips.at(i).toString();
                item.icon = icons.at(i).toString();
                m_labelMenuItemIds.append(item.id);
                m_labelMenuItemData.insert(item.id, item);
                qDebug() << item.id << item.icon << item.label;
            }
        }else{
            qDebug() << "load menu fail: " << error.errorString();
        }
    }
}
#endif
