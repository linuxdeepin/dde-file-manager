#include "dabstractfileinfo.h"

#include "views/dfileview.h"

#include "dfilesystemmodel.h"

#include "shutil/fileutils.h"
#include "shutil/mimetypedisplaymanager.h"

#include "controllers/pathmanager.h"
#include "dfileservices.h"

#include "app/define.h"

#include "widgets/singleton.h"

#include <QDateTime>
#include <QDebug>
#include <QApplication>
#include <QCollator>

namespace FileSortFunction {
Qt::SortOrder sortOrderGlobal;
DAbstractFileInfo::sortFunction sortFun;
QCollator sortCollator;

bool sortByString(const QString &str1, const QString &str2, Qt::SortOrder order)
{
    if (DFMGlobal::startWithHanzi(str1)) {
        if (!DFMGlobal::startWithHanzi(str2))
            return order != Qt::AscendingOrder;
    } else if (DFMGlobal::startWithHanzi(str2)) {
        return order == Qt::AscendingOrder;
    }

    return ((order == Qt::DescendingOrder) ^ (sortCollator.compare(str1, str2) < 0)) == 0x01;
}

SORT_FUN_DEFINE(displayName, DisplayName, DAbstractFileInfo)
SORT_FUN_DEFINE(size, Size, DAbstractFileInfo)
SORT_FUN_DEFINE(lastModified, Modified, DAbstractFileInfo)
SORT_FUN_DEFINE(mimeTypeDisplayNameOrder, Mime, DAbstractFileInfo)
SORT_FUN_DEFINE(created, Created, DAbstractFileInfo)

bool sort(const AbstractFileInfoPointer &info1, const AbstractFileInfoPointer &info2)
{
    return sortFun(info1, info2, sortOrderGlobal);
}
} /// end namespace FileSortFunction

class AbstractFileInfoPrivate
{

};

QMap<DUrl, DAbstractFileInfo::FileMetaData> DAbstractFileInfo::metaDataCacheMap;

DAbstractFileInfo::DAbstractFileInfo()
    : data(new FileInfoData)
{
    init();
}

DAbstractFileInfo::DAbstractFileInfo(const DUrl &url)
    : data(new FileInfoData)
{
    data->url = url;
    data->fileInfo.setFile(url.path());

    init();
//    updateFileMetaData();
    updateFileInfo();
}

DAbstractFileInfo::DAbstractFileInfo(const QString &url)
    : data(new FileInfoData)
{
    data->url = DUrl::fromUserInput(url);
    data->fileInfo.setFile(data->url.path());

    init();
//    updateFileMetaData();
    updateFileInfo();
}

DAbstractFileInfo::~DAbstractFileInfo()
{
    delete data;
}

void DAbstractFileInfo::setUrl(const DUrl &url)
{
    data->url = url;
    data->fileInfo.setFile(url.path());

//    updateFileMetaData();
    updateFileInfo();
}

bool DAbstractFileInfo::exists() const
{
    return data->fileInfo.exists();
}

QString DAbstractFileInfo::filePath() const
{
    if (data->filePath.isEmpty()){
        data->filePath = data->fileInfo.filePath();
    }
    return data->filePath;
}

QString DAbstractFileInfo::absoluteFilePath() const
{
    if (data->absoluteFilePath.isEmpty()){
        data->absoluteFilePath = data->fileInfo.absoluteFilePath();
    }
    return data->absoluteFilePath;
}

QString DAbstractFileInfo::baseName() const
{
    return data->fileInfo.baseName();
}

QString DAbstractFileInfo::fileName() const
{
    if (data->fileName.isEmpty()){
        data->fileName = data->fileInfo.fileName();
    }
    return data->fileName;
}

QString DAbstractFileInfo::displayName() const
{
    return fileName();
}

QString DAbstractFileInfo::pinyinName() const
{
    const QString &diaplayName = this->displayName();

    if (data->pinyinName.isEmpty())
        data->pinyinName = DFMGlobal::toPinyin(diaplayName);

    return data->pinyinName;
}

QString DAbstractFileInfo::path() const
{
    if (data->path.isEmpty()){
        data->path = data->fileInfo.path();
    }
    return data->path;
}

QString DAbstractFileInfo::absolutePath() const
{
    if (data->absolutePath.isEmpty()){
        data->absolutePath = data->fileInfo.absolutePath();
    }
    return data->absolutePath;
}

bool DAbstractFileInfo::isCanShare() const
{
    return false;
}

bool DAbstractFileInfo::isReadable() const
{
    return metaData().isReadable;
}

bool DAbstractFileInfo::isWritable() const
{
    return metaData().isWritable;
}

bool DAbstractFileInfo::isExecutable() const
{
    return metaData().isExecutable;
}

bool DAbstractFileInfo::isHidden() const
{
    return data->fileInfo.isHidden();
}

bool DAbstractFileInfo::isRelative() const
{
    return data->fileInfo.isRelative();
}

bool DAbstractFileInfo::isAbsolute() const
{
    return data->fileInfo.isAbsolute();
}

bool DAbstractFileInfo::isShared() const
{
    return false;
}

bool DAbstractFileInfo::makeAbsolute()
{
    bool ok = data->fileInfo.makeAbsolute();

    data->url.setPath(data->fileInfo.filePath());

    return ok;
}

bool DAbstractFileInfo::isFile() const
{
    return data->fileInfo.isFile();
}

bool DAbstractFileInfo::isDir() const
{
    return data->fileInfo.isDir();
}

bool DAbstractFileInfo::isSymLink() const
{
    return data->fileInfo.isSymLink();
}

bool DAbstractFileInfo::isDesktopFile() const
{
    return mimeTypeName() == "application/x-desktop";
}

QString DAbstractFileInfo::readLink() const
{
    return data->fileInfo.readLink();
}

QString DAbstractFileInfo::owner() const
{
    return data->fileInfo.owner();
}

uint DAbstractFileInfo::ownerId() const
{
    return data->fileInfo.ownerId();
}

QString DAbstractFileInfo::group() const
{
    return data->fileInfo.group();
}

uint DAbstractFileInfo::groupId() const
{
    return data->fileInfo.groupId();
}

QFileDevice::Permissions DAbstractFileInfo::permissions() const
{
    return metaData().permissions;
}

qint64 DAbstractFileInfo::size() const
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

qint64 DAbstractFileInfo::filesCount() const
{
    return FileUtils::filesCount(data->fileInfo.absoluteFilePath());
}

QDateTime DAbstractFileInfo::created() const
{
    if (data->created.isNull()){
        data->created = data->fileInfo.created();
    }
    return data->created;
}

QDateTime DAbstractFileInfo::lastModified() const
{
    if (data->lastModified.isNull()){
        data->lastModified = data->fileInfo.lastModified();
    }
    return data->lastModified;
}

QDateTime DAbstractFileInfo::lastRead() const
{
    return data->fileInfo.lastRead();
}

QString DAbstractFileInfo::lastReadDisplayName() const
{
    return lastRead().toString(timeFormat());
}

QString DAbstractFileInfo::lastModifiedDisplayName() const
{
    return lastModified().toString(timeFormat());
}

QString DAbstractFileInfo::createdDisplayName() const
{
    return created().toString(timeFormat());
}

QString DAbstractFileInfo::sizeDisplayName() const
{
    if (isFile()){
        return FileUtils::formatSize(size());
    }else{
        if (size() <= 1){
            return QObject::tr("%1 item").arg(size());
        }else{
            return QObject::tr("%1 items").arg(size());
        }
    }
}

QString DAbstractFileInfo::mimeTypeDisplayName() const
{
    return mimeTypeDisplayManager->displayName(mimeTypeName());
}

int DAbstractFileInfo::mimeTypeDisplayNameOrder() const
{
    return static_cast<int>(mimeTypeDisplayManager->displayNameOrder(mimeTypeName()));
}

DUrl DAbstractFileInfo::parentUrl() const
{
    return DUrl::parentUrl(data->url);
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
                   << MenuAction::OpenAsAdmain
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
                       << MenuAction::OpenAsAdmain
                       << MenuAction::Separator
                       << MenuAction::Copy
                       << MenuAction::CreateSymlink
                       << MenuAction::SendToDesktop
                       << MenuAction::OpenInTerminal
                       << MenuAction::Separator
                       << MenuAction::Compress
                       << MenuAction::Separator;
            if(isShared())
                actionKeys << MenuAction::UnShare
                           << MenuAction::Separator;

            actionKeys  << MenuAction::Property;

        } else {
            actionKeys << MenuAction::Open;

            if (isDir()){
                actionKeys << MenuAction::OpenInNewWindow
                           << MenuAction::OpenInNewTab
                           << MenuAction::OpenAsAdmain;
            }else{
                if (!isDesktopFile())
                    actionKeys << MenuAction::OpenWith;
            }
            actionKeys << MenuAction::Separator
                       << MenuAction::Cut
                       << MenuAction::Copy
                       << MenuAction::CreateSymlink
                       << MenuAction::SendToDesktop;
            if (isDir()){
                actionKeys << MenuAction::AddToBookMark;
            }


            actionKeys << MenuAction::Rename;

            if (isDir()) {
                actionKeys << MenuAction::Compress << MenuAction::OpenInTerminal;
            } else if(isFile()) {
                if (mimeTypeName().startsWith("image") && isReadable()) {
                    actionKeys << MenuAction::SetAsWallpaper;
                }

                actionKeys << MenuAction::Separator;

                if (FileUtils::isArchive(absoluteFilePath())){
                    actionKeys << MenuAction::Decompress << MenuAction::DecompressHere;
                }else{
                    actionKeys << MenuAction::Compress;
                }
            }

            actionKeys << MenuAction::Separator
                       << MenuAction::Delete
                       << MenuAction::Separator;
            if(isDir() && isShared())
                actionKeys << MenuAction::UnShare
                           << MenuAction::Separator;

            actionKeys  << MenuAction::Property;
        }
    } else if (type == MultiFiles) {
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
    } else if (type == MultiFilesSystemPathIncluded) {
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


quint8 DAbstractFileInfo::supportViewMode() const
{
    return DFileView::AllViewMode;
}

QAbstractItemView::SelectionMode DAbstractFileInfo::supportSelectionMode() const
{
    return QAbstractItemView::ExtendedSelection;
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
    switch (userColumnRole) {
    case DFileSystemModel::FileSizeRole:
        return 80;
    case DFileSystemModel::FileMimeTypeRole:
        return 80;
    default:
        return qApp->fontMetrics().width("0000/00/00 00:00:00");
    }
}

bool DAbstractFileInfo::columnDefaultVisibleForRole(int role) const
{
    return !(role == DFileSystemModel::FileCreatedRole
             || role == DFileSystemModel::FileMimeTypeRole);
}

DAbstractFileInfo::sortFunction DAbstractFileInfo::sortFunByColumn(int columnRole) const
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

void DAbstractFileInfo::sortByColumnRole(QList<AbstractFileInfoPointer> &fileList, int columnRole, Qt::SortOrder order) const
{
    FileSortFunction::sortOrderGlobal = order;
    FileSortFunction::sortFun = sortFunByColumn(columnRole);

    if (!FileSortFunction::sortFun)
        return;

    qSort(fileList.begin(), fileList.end(), FileSortFunction::sort);
}

int DAbstractFileInfo::getIndexByFileInfo(getFileInfoFun fun, const AbstractFileInfoPointer &info, int columnType, Qt::SortOrder order) const
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

bool DAbstractFileInfo::canRedirectionFileUrl() const
{
    return false;
}

DUrl DAbstractFileInfo::redirectedFileUrl() const
{
    return fileUrl();
}

bool DAbstractFileInfo::isEmptyFloder() const
{
    if (!isDir())
        return false;

    DDirIteratorPointer it = DFileService::instance()->createDirIterator(fileUrl(), QStringList(),
                                                                         QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System,
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

    url.setPath(absolutePath() + "/" + fileName);

    return url;
}

DUrl DAbstractFileInfo::mimeDataUrl() const
{
    if (canRedirectionFileUrl())
        return redirectedFileUrl();

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
    const QString &suffix = data->fileInfo.suffix();
    const QString &completeSuffix = data->fileInfo.completeSuffix();

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
    return data->fileInfo.completeSuffix();
}

void DAbstractFileInfo::updateFileInfo()
{
    metaDataCacheMap.remove(data->url);

    data->fileInfo.refresh();
    data->size = -1;
    data->created = QDateTime();
    data->lastModified = QDateTime();

    updateFileMetaData();
}

void DAbstractFileInfo::updateFileMetaData()
{
    this->data->pinyinName = QString();

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

void DAbstractFileInfo::init()
{
    m_userColumnRoles << DFileSystemModel::FileLastModifiedRole << DFileSystemModel::FileSizeRole
                      << DFileSystemModel::FileMimeTypeRole;

    FileSortFunction::sortCollator.setNumericMode(true);
    FileSortFunction::sortCollator.setCaseSensitivity(Qt::CaseInsensitive);
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

    if (!isCanRename()) {
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

