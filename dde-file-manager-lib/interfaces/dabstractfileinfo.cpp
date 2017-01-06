#include "dabstractfileinfo.h"
#include "private/dabstractfileinfo_p.h"
#include "views/dfileview.h"

#include "dfilesystemmodel.h"

#include "shutil/fileutils.h"
#include "shutil/mimetypedisplaymanager.h"

#include "controllers/pathmanager.h"
#include "dfileservices.h"

#include "app/define.h"

#include "widgets/singleton.h"
#include "plugins/pluginmanager.h"
#include "dde-file-manager-plugins/plugininterfaces/menu/menuinterface.h"

#include <QDateTime>
#include <QDebug>
#include <QApplication>
#include <QCollator>

namespace FileSortFunction {
QCollator sortCollator;

bool compareByString(const QString &str1, const QString &str2, Qt::SortOrder order)
{
    if (DFMGlobal::startWithHanzi(str1)) {
        if (!DFMGlobal::startWithHanzi(str2))
            return order != Qt::AscendingOrder;
    } else if (DFMGlobal::startWithHanzi(str2)) {
        return order == Qt::AscendingOrder;
    }

    return ((order == Qt::DescendingOrder) ^ (sortCollator.compare(str1, str2) < 0)) == 0x01;
}

COMPARE_FUN_DEFINE(fileDisplayName, DisplayName, DAbstractFileInfo)
COMPARE_FUN_DEFINE(fileSize, Size, DAbstractFileInfo)
COMPARE_FUN_DEFINE(lastModified, Modified, DAbstractFileInfo)
COMPARE_FUN_DEFINE(mimeTypeDisplayName, Mime, DAbstractFileInfo)
COMPARE_FUN_DEFINE(created, Created, DAbstractFileInfo)
} /// end namespace FileSortFunction

#define CALL_PROXY(Fun)\
    Q_D(const DAbstractFileInfo);\
    if (d->proxy) return d->proxy->Fun;

QMap<DUrl, DAbstractFileInfo*> DAbstractFileInfoPrivate::urlToFileInfoMap;

DAbstractFileInfoPrivate::DAbstractFileInfoPrivate(const DUrl &url, DAbstractFileInfo *qq, bool hasCache)
    : q_ptr(qq)
    , fileUrl(url)
{
    if (hasCache) {
        urlToFileInfoMap[url] = qq;
    }

    FileSortFunction::sortCollator.setNumericMode(true);
    FileSortFunction::sortCollator.setCaseSensitivity(Qt::CaseInsensitive);
}

DAbstractFileInfoPrivate::~DAbstractFileInfoPrivate()
{
    if (urlToFileInfoMap.value(fileUrl) == q_ptr)
        urlToFileInfoMap.remove(fileUrl);
}

void DAbstractFileInfoPrivate::setUrl(const DUrl &url, bool hasCache)
{
    if (url == fileUrl)
        return;

    if (urlToFileInfoMap.value(fileUrl) == q_ptr)
        urlToFileInfoMap.remove(fileUrl);

    if (hasCache)
        urlToFileInfoMap[url] = q_ptr;

    fileUrl = url;
}

DAbstractFileInfo *DAbstractFileInfoPrivate::getFileInfo(const DUrl &fileUrl)
{
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

    const DDirIteratorPointer &iterator = fileService->createDirIterator(fileUrl(), QStringList(),
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

    if (isFile()) {
        return FileUtils::formatSize(size());
    } else {
        int size = filesCount();

        if (size <= 1){
            return QObject::tr("%1 item").arg(size);
        }else{
            return QObject::tr("%1 items").arg(size);
        }
    }
}

QString DAbstractFileInfo::mimeTypeDisplayName() const
{
    CALL_PROXY(mimeTypeDisplayName());

    return mimeTypeDisplayManager->displayName(mimeTypeName());
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

        const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(parentUrl);

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
//                   << MenuAction::OpenAsAdmin
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
//                       << MenuAction::OpenAsAdmin
                       << MenuAction::Separator
                       << MenuAction::Copy
                       << MenuAction::Compress;

            if (canShare() && !isShared()){
                actionKeys << MenuAction::Share;
            }else if(isShared()){
                actionKeys << MenuAction::UnShare;
            }
            actionKeys << MenuAction::CreateSymlink
                       << MenuAction::SendToDesktop
                       << MenuAction::OpenInTerminal
                       << MenuAction::Separator;

            actionKeys  << MenuAction::Property;

        } else {
            actionKeys << MenuAction::Open;

            if (isDir()){
                actionKeys << MenuAction::OpenInNewWindow
                           << MenuAction::OpenInNewTab;
//                           << MenuAction::OpenAsAdmin;
            }else{
                if (!isDesktopFile())
                    actionKeys << MenuAction::OpenWith;
            }
            actionKeys << MenuAction::Separator
                       << MenuAction::Cut
                       << MenuAction::Copy
                       << MenuAction::Rename;

            if (isDir()) {
                actionKeys << MenuAction::Compress;

                if (canShare() && !isShared()){
                    actionKeys << MenuAction::Share;
                }else if(isShared()){
                    actionKeys << MenuAction::UnShare;
                }

            }else if(isFile()) {
                if (!FileUtils::isArchive(absoluteFilePath())){
                    actionKeys << MenuAction::Compress;
                }
            }

            actionKeys << MenuAction::CreateSymlink
                       << MenuAction::SendToDesktop;

            if (isDir()) {
                actionKeys << MenuAction::AddToBookMark
                           << MenuAction::OpenInTerminal;
            } else if(isFile()) {
                if (mimeTypeName().startsWith("image") && isReadable()) {
                    actionKeys << MenuAction::SetAsWallpaper;
                }
            }

            actionKeys << MenuAction::Delete
                       << MenuAction::Separator;

            if (isFile()){
                if (FileUtils::isArchive(absoluteFilePath())){
                    actionKeys << MenuAction::Decompress
                               << MenuAction::DecompressHere
                               << MenuAction::Separator;
                }
            }

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
    return !(role == DFileSystemModel::FileCreatedRole
             || role == DFileSystemModel::FileMimeTypeRole);
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

    DDirIteratorPointer it = DFileService::instance()->createDirIterator(fileUrl(), QStringList(),
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

    QString suffix;
    const QString &fileName = this->fileName();
    int index = fileName.lastIndexOf('.');

    if (index >= 0)
        suffix = fileName.mid(index + 1);

    const QString &completeSuffix = this->completeSuffix();

    if (completeSuffix != suffix) {
        QStringList suffixes = completeSuffix.split(".");

        if (suffixes.length() >= 2 && suffixes.at(suffixes.length() - 2) == "tar") {
            return QString("%1.%2").arg("tar", suffix);
        }
    }

    return suffix;
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

        const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(url);

        if (fileInfo && fileInfo->exists()) {
            break;
        }
    }

    return extistParentUrl.isValid() ? extistParentUrl : DUrl::fromLocalFile(QDir::homePath());
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
}

DAbstractFileInfo::DAbstractFileInfo(DAbstractFileInfoPrivate &dd)
    : d_ptr(&dd)
{

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
        list << MenuAction::Cut << MenuAction::Rename << MenuAction::Remove << MenuAction::Delete;
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
