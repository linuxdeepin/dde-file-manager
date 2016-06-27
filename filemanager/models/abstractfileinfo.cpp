#include "abstractfileinfo.h"

#include "../views/dfileview.h"

#include "../models/dfilesystemmodel.h"

#include "../shutil/fileutils.h"
#include "../controllers/pathmanager.h"
#include "../app/global.h"
#include <QDateTime>
#include <QDebug>


namespace FileSortFunction {
Qt::SortOrder sortOrderGlobal;
AbstractFileInfo::sortFunction sortFun;

bool sortFileListByDisplayName(const AbstractFileInfoPointer &info1, const AbstractFileInfoPointer &info2, Qt::SortOrder order)
{
    if(info1->isDir()) {
        if(!info2->isDir())
            return true;
    } else {
        if(info2->isDir())
            return false;
    }

    return ((order == Qt::DescendingOrder) ^ (info1->displayName().toLower() < info2->displayName().toLower())) == 0x01;
}

SORT_FUN_DEFINE(size, Size, AbstractFileInfo)
SORT_FUN_DEFINE(lastModified, Modified, AbstractFileInfo)
SORT_FUN_DEFINE(mimeTypeDisplayNameOrder, Mime, AbstractFileInfo)
SORT_FUN_DEFINE(created, Created, AbstractFileInfo)

bool sort(const AbstractFileInfoPointer &info1, const AbstractFileInfoPointer &info2)
{
    return sortFun(info1, info2, sortOrderGlobal);
}
} /// end namespace FileSortFunction

QMap<DUrl, AbstractFileInfo::FileMetaData> AbstractFileInfo::metaDataCacheMap;

AbstractFileInfo::AbstractFileInfo()
    : data(new FileInfoData)
{
    init();
}

AbstractFileInfo::AbstractFileInfo(const DUrl &url)
    : data(new FileInfoData)
{
    data->url = url;
    data->fileInfo.setFile(url.path());

    init();
    updateFileMetaData();
}

AbstractFileInfo::AbstractFileInfo(const QString &url)
    : data(new FileInfoData)
{
    data->url = DUrl::fromUserInput(url);
    data->fileInfo.setFile(data->url.path());

    init();
    updateFileMetaData();
}

AbstractFileInfo::~AbstractFileInfo()
{
    delete data;
}

void AbstractFileInfo::setUrl(const DUrl &url)
{
    data->url = url;
    data->fileInfo.setFile(url.path());

    updateFileMetaData();
}

bool AbstractFileInfo::exists() const
{
    return data->fileInfo.exists();
}

QString AbstractFileInfo::filePath() const
{
    if (data->filePath.isNull() || data->filePath.isEmpty()){
        data->filePath = data->fileInfo.filePath();
    }
    return data->filePath;
}

QString AbstractFileInfo::absoluteFilePath() const
{
    if (data->absoluteFilePath.isNull() || data->absoluteFilePath.isEmpty()){
        data->absoluteFilePath = data->fileInfo.absoluteFilePath();
    }
    return data->absoluteFilePath;
}

QString AbstractFileInfo::fileName() const
{
    if (data->fileName.isNull() || data->fileName.isEmpty()){
        data->fileName = data->fileInfo.fileName();
    }
    return data->fileName;
}

QString AbstractFileInfo::displayName() const
{
    if (systemPathManager->isSystemPath(filePath())) {
        QString displayName = systemPathManager->getSystemPathDisplayNameByPath(filePath());
        if (displayName.isEmpty())
            return fileName();
        else
            return displayName;

    } else {
        if (isDesktopFile()){
            return DesktopFile(absoluteFilePath()).getName();
        }

        return fileName();
    }

    return QString();
}

QString AbstractFileInfo::path() const
{
    if (data->path.isNull() || data->path.isEmpty()){
        data->path = data->fileInfo.path();
    }
    return data->path;
}

QString AbstractFileInfo::absolutePath() const
{
    if (data->absolutePath.isNull() || data->absolutePath.isEmpty()){
        data->absolutePath = data->fileInfo.absolutePath();
    }
    return data->absolutePath;
}

bool AbstractFileInfo::isReadable() const
{
    return metaData().isReadable;
}

bool AbstractFileInfo::isWritable() const
{
    return metaData().isWritable;
}

bool AbstractFileInfo::isExecutable() const
{
    return metaData().isExecutable;
}

bool AbstractFileInfo::isHidden() const
{
    return data->fileInfo.isHidden();
}

bool AbstractFileInfo::isRelative() const
{
    return data->fileInfo.isRelative();
}

bool AbstractFileInfo::isAbsolute() const
{
    return data->fileInfo.isAbsolute();
}

bool AbstractFileInfo::makeAbsolute()
{
    bool ok = data->fileInfo.makeAbsolute();

    data->url.setPath(data->fileInfo.filePath());

    return ok;
}

bool AbstractFileInfo::isFile() const
{
    return data->fileInfo.isFile();
}

bool AbstractFileInfo::isDir() const
{
    return data->fileInfo.isDir();
}

bool AbstractFileInfo::isSymLink() const
{
    return data->fileInfo.isSymLink();
}

bool AbstractFileInfo::isDesktopFile() const
{
    return mimeTypeName() == "application/x-desktop";
}

QString AbstractFileInfo::readLink() const
{
    return data->fileInfo.readLink();
}

QString AbstractFileInfo::owner() const
{
    return data->fileInfo.owner();
}

uint AbstractFileInfo::ownerId() const
{
    return data->fileInfo.ownerId();
}

QString AbstractFileInfo::group() const
{
    return data->fileInfo.group();
}

uint AbstractFileInfo::groupId() const
{
    return data->fileInfo.groupId();
}

QFileDevice::Permissions AbstractFileInfo::permissions() const
{
    return metaData().permissions;
}

qint64 AbstractFileInfo::size() const
{
    if (isFile()){
        if (data->size == -1){
            data->size = data->fileInfo.size();
        }
        return data->size;
    }else{
        return filesCount();
    }
}

qint64 AbstractFileInfo::filesCount() const
{
    return FileUtils::filesCount(data->fileInfo.absoluteFilePath());
}

QDateTime AbstractFileInfo::created() const
{
    if (data->created.isNull()){
        data->created = data->fileInfo.created();
    }
    return data->created;
}

QDateTime AbstractFileInfo::lastModified() const
{
    if (data->lastModified.isNull()){
        data->lastModified = data->fileInfo.lastModified();
    }
    return data->lastModified;
}

QDateTime AbstractFileInfo::lastRead() const
{
    return data->fileInfo.lastRead();
}

QString AbstractFileInfo::lastModifiedDisplayName() const
{
    return lastModified().toString(timeFormat());
}

QString AbstractFileInfo::createdDisplayName() const
{
    return created().toString(timeFormat());
}

QString AbstractFileInfo::sizeDisplayName() const
{
    if (isFile()){
        return FileUtils::formatSize(size());
    }else{
        return QObject::tr("%1 item(s)").arg(size());
    }
}

QString AbstractFileInfo::mimeTypeDisplayName() const
{
    return mimeTypeDisplayManager->displayName(mimeTypeName());
}

int AbstractFileInfo::mimeTypeDisplayNameOrder() const
{
    return static_cast<int>(mimeTypeDisplayManager->displayNameOrder(mimeTypeName()));
}

DUrl AbstractFileInfo::parentUrl() const
{
    DUrl url = data->url;
    const QString &path = url.path();

    url.setPath(path.left(path.lastIndexOf('/') + 1));

    return url;
}

QVector<MenuAction> AbstractFileInfo::menuActionList(AbstractFileInfo::MenuType type) const
{

    QVector<MenuAction> actionKeys;

    if(type == SpaceArea) {
        actionKeys.reserve(9);

        actionKeys << MenuAction::NewFolder
                   << MenuAction::NewDocument
                   << MenuAction::Separator
                   << MenuAction::DisplayAs
                   << MenuAction::SortBy
                   << MenuAction::OpenInTerminal
                   << MenuAction::Separator
                   << MenuAction::Paste
                   << MenuAction::SelectAll
                   << MenuAction::Separator
                   << MenuAction::Property;
    } else if (type == SingleFile){

        if (isDir() && systemPathManager->isSystemPath(filePath())){
            actionKeys << MenuAction::Open
                       << MenuAction::OpenInNewWindow
                       << MenuAction::Separator
                       << MenuAction::Copy
                       << MenuAction::CreateSoftLink
                       << MenuAction::SendToDesktop
                       << MenuAction::Separator
                       << MenuAction::Compress
                       << MenuAction::Separator
                       << MenuAction::Property;
        }else{
            actionKeys << MenuAction::Open;

            if (isDir()){
                actionKeys << MenuAction::OpenInNewWindow;
            }else{
                if (!isDesktopFile())
                    actionKeys << MenuAction::OpenWith;
            }
            actionKeys << MenuAction::Separator
                       << MenuAction::Cut
                       << MenuAction::Copy
                       << MenuAction::CreateSoftLink
                       << MenuAction::SendToDesktop;
            if (isDir()){
                actionKeys << MenuAction::AddToBookMark;
            }


            actionKeys << MenuAction::Rename;
            QPixmap tempPixmap;
            DUrl url = data->url;
            if (tempPixmap.load(url.toLocalFile())){
                actionKeys << MenuAction::SetAsWallpaper;
            }

            actionKeys << MenuAction::Separator;
            if (isDir()){
                actionKeys << MenuAction::Compress;
            }else if(isFile()){
                if (FileUtils::isArchive(absoluteFilePath())){
                    actionKeys << MenuAction::Decompress << MenuAction::DecompressHere;
                }else{
                    actionKeys << MenuAction::Compress;
                }
            }

            actionKeys << MenuAction::Separator
                       << MenuAction::Delete
                       << MenuAction::Separator
                       << MenuAction::Property;
        }
    }else if(type == MultiFiles){
        actionKeys << MenuAction::Open
                   << MenuAction::Separator
                   << MenuAction::Cut
                   << MenuAction::Copy
                   << MenuAction::SendToDesktop
                   << MenuAction::Separator
                   << MenuAction::Compress
                   << MenuAction::Separator
                   << MenuAction::Delete
                   << MenuAction::Separator
                   << MenuAction::Property;
    }else if(type == MultiFilesSystemPathIncluded){
        actionKeys << MenuAction::Open
                   << MenuAction::Separator
                   << MenuAction::Copy
                   << MenuAction::SendToDesktop
                   << MenuAction::Separator
                   << MenuAction::Compress
                   << MenuAction::Separator
                   << MenuAction::Property;
    }

    return actionKeys;
}


quint8 AbstractFileInfo::supportViewMode() const
{
    return DFileView::AllViewMode;
}

QVariant AbstractFileInfo::userColumnDisplayName(int userColumnRole) const
{
    return DFileSystemModel::roleName(userColumnRole);
}

QVariant AbstractFileInfo::userColumnData(int userColumnRole) const
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

int AbstractFileInfo::userColumnWidth(int userColumnRole) const
{
    switch (userColumnRole) {
    case DFileSystemModel::FileSizeRole:
        return 100;
    case DFileSystemModel::FileMimeTypeRole:
        return 100;
    default:
        return 140;
    }
}

bool AbstractFileInfo::columnDefaultVisibleForRole(int role) const
{
    return !(role == DFileSystemModel::FileCreatedRole
             || role == DFileSystemModel::FileMimeTypeRole);
}

AbstractFileInfo::sortFunction AbstractFileInfo::sortFunByColumn(int columnRole) const
{
    switch (columnRole) {
    case DFileSystemModel::FileDisplayNameRole:
        return FileSortFunction::sortFileListByDisplayName;
    case DFileSystemModel::FileLastModifiedRole:
        return FileSortFunction::sortFileListByModified;
    case DFileSystemModel::FileSizeRole:
        return FileSortFunction::sortFileListBySize;
    case DFileSystemModel::FileMimeTypeRole:
        return FileSortFunction::sortFileListByMime;
    case DFileSystemModel::FileCreatedRole:
        return FileSortFunction::sortFileListByCreated;
    default:
        return sortFunction();
    }
}

void AbstractFileInfo::sortByColumn(QList<AbstractFileInfoPointer> &fileList, int columnRole, Qt::SortOrder order) const
{
    FileSortFunction::sortOrderGlobal = order;
    FileSortFunction::sortFun = sortFunByColumn(columnRole);

    if (!FileSortFunction::sortFun)
        return;

    qSort(fileList.begin(), fileList.end(), FileSortFunction::sort);
}

int AbstractFileInfo::getIndexByFileInfo(getFileInfoFun fun, const AbstractFileInfoPointer &info, int columnType, Qt::SortOrder order) const
{
    FileSortFunction::sortOrderGlobal = order;
    FileSortFunction::sortFun = sortFunByColumn(columnType);

    if (!FileSortFunction::sortFun)
        return -1;

    int index = -1;

    forever {
        const AbstractFileInfoPointer &tmp_info = fun(++index);

        if(!tmp_info)
            break;

        if(FileSortFunction::sort(info, tmp_info)) {
            break;
        }
    }

    return index;
}

bool AbstractFileInfo::canRedirectionFileUrl() const
{
    return false;
}

DUrl AbstractFileInfo::redirectedFileUrl() const
{
    return fileUrl();
}

bool AbstractFileInfo::isEmptyFloder() const
{
    if (!isDir())
        return false;

    DDirIteratorPointer it = FileServices::instance()->createDirIterator(fileUrl(), QDir::AllEntries | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);

    return it && !it->hasNext();
}

void AbstractFileInfo::updateFileMetaData()
{
    if (metaDataCacheMap.contains(this->data->url))
        return;

    QFile::Permissions permissions = this->data->fileInfo.permissions();

    if (permissions == 0 && !exists()) {
        return;
    }

    FileMetaData data;

    data.isExecutable = this->data->fileInfo.isExecutable();
    data.isReadable = this->data->fileInfo.isReadable();
    data.isWritable = this->data->fileInfo.isWritable();
    data.permissions = permissions;

    metaDataCacheMap[this->data->url] = data;
}

void AbstractFileInfo::init()
{
    m_userColumnRoles << DFileSystemModel::FileLastModifiedRole << DFileSystemModel::FileSizeRole
                      << DFileSystemModel::FileMimeTypeRole << DFileSystemModel::FileCreatedRole;
}

QMap<MenuAction, QVector<MenuAction> > AbstractFileInfo::subMenuActionList() const
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
    displayAsMenuActionKeys << MenuAction::IconView
                          << MenuAction::ListView;
//                          << MenuAction::ExtendView;

    actions.insert(MenuAction::DisplayAs, displayAsMenuActionKeys);


    QVector<MenuAction> sortByMenuActionKeys;
    sortByMenuActionKeys << MenuAction::Name
                          << MenuAction::Size
                          << MenuAction::Type
                          << MenuAction::CreatedDate
                          << MenuAction::LastModifiedDate;
    actions.insert(MenuAction::SortBy, sortByMenuActionKeys);

    return actions;
}

QSet<MenuAction> AbstractFileInfo::disableMenuActionList() const
{
    QSet<MenuAction> list;

    if (!isWritable()) {
        list << MenuAction::NewFolder
             << MenuAction::NewDocument
             << MenuAction::Cut << MenuAction::Rename << MenuAction::Paste << MenuAction::Remove << MenuAction::Delete;
    } else if (!isCanRename()) {
        list << MenuAction::Cut << MenuAction::Rename << MenuAction::Remove << MenuAction::Delete;
    }

    return list;
}

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug deg, const AbstractFileInfo &info)
{
    deg << "file url:" << info.fileUrl()
        << "mime type:" << info.mimeTypeName();

    return deg;
}
QT_END_NAMESPACE
