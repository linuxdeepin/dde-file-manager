#include "abstractfileinfo.h"

#include "../views/dfileview.h"

#include "../shutil/fileutils.h"
#include "../controllers/pathmanager.h"
#include "../app/global.h"
#include <QDateTime>
#include <QDebug>

Qt::SortOrder AbstractFileInfo::sortOrderGlobal;

namespace FileSortFunction {
bool sortFileListByDisplayName(const AbstractFileInfoPointer &info1, const AbstractFileInfoPointer &info2)
{
    if(info1->isDir()) {
        if(!info2->isDir())
            return true;
    } else {
        if(info2->isDir())
            return false;
    }

    return ((AbstractFileInfo::sortOrderGlobal == Qt::AscendingOrder)
            ^ (info1->displayName().toLower() < info2->displayName().toLower())) == 0x01;
}

bool sortFileListBySize(const AbstractFileInfoPointer &info1, const AbstractFileInfoPointer &info2)
{
    if(info1->isDir()) {
        if(!info2->isDir())
            return true;
    } else {
        if(info2->isDir())
            return false;
    }

    if (info1->isDir() && info2->isDir() && (info1->size() == info2->size()))
        return sortFileListByDisplayName(info1, info2);
    else if (info1->isFile() && info2->isFile() && (info1->size() == info2->size()))
        return sortFileListByDisplayName(info1, info2);

    return ((AbstractFileInfo::sortOrderGlobal == Qt::AscendingOrder)
            ^ (info1->size() < info2->size())) == 0x01;
}

bool sortFileListByModified(const AbstractFileInfoPointer &info1, const AbstractFileInfoPointer &info2)
{
    if(info1->isDir()) {
        if(!info2->isDir())
            return true;
    } else {
        if(info2->isDir())
            return false;
    }

    if (info1->isDir() && info2->isDir() && (info1->lastModified() == info2->lastModified()))
        return sortFileListByDisplayName(info1, info2);
    else if (info1->isFile() && info2->isFile() && (info1->lastModified() == info2->lastModified()))
        return sortFileListByDisplayName(info1, info2);

    return ((AbstractFileInfo::sortOrderGlobal == Qt::AscendingOrder)
            ^ (info1->lastModified() < info2->lastModified())) == 0x01;
}

bool sortFileListByMime(const AbstractFileInfoPointer &info1, const AbstractFileInfoPointer &info2)
{
    if(info1->isDir()) {
        if(!info2->isDir())
            return true;
    } else {
        if(info2->isDir())
            return false;
    }

    if (info1->isDir() && info2->isDir() && (info1->mimeTypeDisplayName() == info2->mimeTypeDisplayName()))
        return sortFileListByDisplayName(info1, info2);
    else if (info1->isFile() && info2->isFile() && (info1->mimeTypeDisplayName() == info2->mimeTypeDisplayName()))
        return sortFileListByDisplayName(info1, info2);

    return ((AbstractFileInfo::sortOrderGlobal == Qt::AscendingOrder)
            ^ (info1->mimeTypeDisplayNameOrder() < info2->mimeTypeDisplayNameOrder())) == 0x01;
}

bool sortFileListByCreated(const AbstractFileInfoPointer &info1, const AbstractFileInfoPointer &info2)
{
    if(info1->isDir()) {
        if(!info2->isDir())
            return true;
    } else {
        if(info2->isDir())
            return false;
    }

    if (info1->isDir() && info2->isDir() && (info1->created() == info2->created()))
        return sortFileListByDisplayName(info1, info2);
    else if (info1->isFile() && info2->isFile() && (info1->created() == info2->created()))
        return sortFileListByDisplayName(info1, info2);

    return ((AbstractFileInfo::sortOrderGlobal == Qt::AscendingOrder)
            ^ (info1->created() < info2->created())) == 0x01;
}
} /// end namespace FileSortFunction

AbstractFileInfo::AbstractFileInfo()
    : data(new FileInfoData)
{

}

AbstractFileInfo::AbstractFileInfo(const DUrl &url)
    : data(new FileInfoData)
{
    data->url = url;
    data->fileInfo.setFile(url.path());
}

AbstractFileInfo::AbstractFileInfo(const QString &url)
    : data(new FileInfoData)
{
    data->url = DUrl::fromUserInput(url);
    data->fileInfo.setFile(data->url.path());
}

AbstractFileInfo::~AbstractFileInfo()
{
    delete data;
}

void AbstractFileInfo::setUrl(const DUrl &url)
{
    data->url = url;
    data->fileInfo.setFile(url.path());
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
    return data->fileInfo.isReadable();
}

bool AbstractFileInfo::isWritable() const
{
    return data->fileInfo.isWritable();
}

bool AbstractFileInfo::isExecutable() const
{
    return data->fileInfo.isExecutable();
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
    return data->fileInfo.permissions();
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

quint8 AbstractFileInfo::userColumnCount() const
{
    return 0;
}

QVariant AbstractFileInfo::userColumnDisplayName(quint8 userColumnType) const
{
    Q_UNUSED(userColumnType)

    return QVariant();
}

QVariant AbstractFileInfo::userColumnData(quint8 userColumnType) const
{
    Q_UNUSED(userColumnType);

    return QVariant();
}

void AbstractFileInfo::sortByColumn(QList<AbstractFileInfoPointer> &fileList, quint8 columnType, Qt::SortOrder order) const
{
    sortOrderGlobal = order;

    switch (columnType) {
    case DisplayNameType:
        qSort(fileList.begin(), fileList.end(), FileSortFunction::sortFileListByDisplayName);
        break;
    case LastModifiedDateType:
        qSort(fileList.begin(), fileList.end(), FileSortFunction::sortFileListByModified);
        break;
    case SizeType:
        qSort(fileList.begin(), fileList.end(), FileSortFunction::sortFileListBySize);
        break;
    case FileMimeType:
        qSort(fileList.begin(), fileList.end(), FileSortFunction::sortFileListByMime);
        break;
    case CreatedDateType:
        qSort(fileList.begin(), fileList.end(), FileSortFunction::sortFileListByCreated);
        break;
    default:
        sortByUserColumn(fileList, columnType, order);
        break;
    }
}

int AbstractFileInfo::getIndexByFileInfo(getFileInfoFun fun, const AbstractFileInfoPointer &info, quint8 columnType, Qt::SortOrder order) const
{
    std::function<bool(const AbstractFileInfoPointer&, const AbstractFileInfoPointer&)> sortFun;

    sortOrderGlobal = order;

    switch (columnType) {
    case DisplayNameType:
        sortFun = FileSortFunction::sortFileListByDisplayName;
        break;
    case LastModifiedDateType:
        sortFun = FileSortFunction::sortFileListByModified;
        break;
    case SizeType:
        sortFun = FileSortFunction::sortFileListBySize;
        break;
    case FileMimeType:
        sortFun = FileSortFunction::sortFileListByMime;
        break;
    case CreatedDateType:
        sortFun = FileSortFunction::sortFileListByCreated;
        break;
    default:
        return -1;
    }

    int index = 0;

    forever {
        const AbstractFileInfoPointer &tmp_info = fun(index++);

        if(!tmp_info)
            break;

        if(sortFun(info, tmp_info)) {
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

void AbstractFileInfo::sortByUserColumn(QList<AbstractFileInfoPointer> &fileList, quint8 columnType, Qt::SortOrder order) const
{
    Q_UNUSED(fileList)
    Q_UNUSED(columnType)
    Q_UNUSED(order)
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

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug deg, const AbstractFileInfo &info)
{
    deg << "file url:" << info.fileUrl()
        << "mime type:" << info.mimeTypeName();

    return deg;
}
QT_END_NAMESPACE
