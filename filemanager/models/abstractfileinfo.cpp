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

    return ((AbstractFileInfo::sortOrderGlobal == Qt::DescendingOrder)
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

    return ((AbstractFileInfo::sortOrderGlobal == Qt::DescendingOrder)
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

    return ((AbstractFileInfo::sortOrderGlobal == Qt::DescendingOrder)
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

    return ((AbstractFileInfo::sortOrderGlobal == Qt::DescendingOrder)
            ^ (info1->mimeTypeName() < info2->mimeTypeName())) == 0x01;
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

    return ((AbstractFileInfo::sortOrderGlobal == Qt::DescendingOrder)
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
    return data->fileInfo.filePath();
}

QString AbstractFileInfo::absoluteFilePath() const
{
    return data->fileInfo.absoluteFilePath();
}

QString AbstractFileInfo::fileName() const
{
    return data->fileInfo.fileName();
}

QString AbstractFileInfo::displayName() const
{

    if (systemPathManager->systemPaths().values().contains(filePath())){
        foreach (QString key, systemPathManager->systemPaths().keys()) {
            if (systemPathManager->systemPaths().value(key) == filePath()){
                if (fileName() == key){
                    return systemPathManager->getSystemPathDisplayName(key);
                }else{
                    return fileName();
                }
            }
        }
    }else{
        if (fileName().endsWith("desktop")){
            return DesktopFile(absoluteFilePath()).getName();
        }
        return fileName();
    }
}

QString AbstractFileInfo::path() const
{
    return data->fileInfo.path();
}

QString AbstractFileInfo::absolutePath() const
{
    return data->fileInfo.absolutePath();
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
    return data->fileInfo.size();
}

QDateTime AbstractFileInfo::created() const
{
    return data->fileInfo.created();
}

QDateTime AbstractFileInfo::lastModified() const
{
    return data->fileInfo.lastModified();
}

QDateTime AbstractFileInfo::lastRead() const
{
    return data->fileInfo.lastRead();
}

QString AbstractFileInfo::lastModifiedDisplayName() const
{
    return lastModified().toString("yyyy/MM/dd HH:MM:ss");
}

QString AbstractFileInfo::createdDisplayName() const
{
    return created().toString("yyyy/MM/dd HH:MM:ss");
}

QString AbstractFileInfo::sizeDisplayName() const
{
    return FileUtils::formatSize(size());
}

QString AbstractFileInfo::mimeTypeDisplayName() const
{
    return mimeTypeDisplayManager->displayName(mimeTypeName());
}

DUrl AbstractFileInfo::parentUrl() const
{
    DUrl url = data->url;
    const QString &path = url.path();

    url.setPath(path.left(path.lastIndexOf('/') + 1));

    return url;
}

QVector<AbstractFileInfo::MenuAction> AbstractFileInfo::menuActionList(AbstractFileInfo::MenuType type) const
{
    QVector<MenuAction> actionKeys;

    if(type == SpaceArea) {
        actionKeys.reserve(9);

        actionKeys << NewFolder
                   << NewDocument
                   << Separator
                   << DisplayAs
                   << SortBy
                   << OpenInTerminal
                   << Separator
                   << Paste
                   << SelectAll
                   << Separator
                   << Property;
    } else {
        actionKeys.reserve(11);

        actionKeys << Open << (isDir() ? OpenInNewWindow : OpenWith)
                   << Separator;
        if (isDir()){
            actionKeys << Compress;
        }else if(isFile()){
            if (FileUtils::isArchive(absoluteFilePath())){
                actionKeys << Decompress << DecompressHere;
            }else{
                actionKeys << Compress;
            }
        }
        actionKeys << Separator
                   << Cut
                   << Copy
                   << CreateSoftLink
                   << SendToDesktop;
        if (systemPathManager->systemPaths().values().contains(filePath())){
            foreach (QString key, systemPathManager->systemPaths().keys()) {
                if (systemPathManager->systemPaths().value(key) == filePath()){
                    if (fileName() != key){
                        actionKeys << AddToBookMark;
                    }
                }
            }
        }else{
            actionKeys << AddToBookMark;
        }
        actionKeys << Rename;
        QPixmap tempPixmap;
        DUrl url = data->url;
        if (tempPixmap.load(url.toLocalFile())){
            actionKeys << SetAsWallpaper;
        }

        actionKeys << Separator
                   << Delete
                   << Separator
                   << Property;
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

QMap<AbstractFileInfo::MenuAction, QVector<AbstractFileInfo::MenuAction> > AbstractFileInfo::subMenuActionList() const
{
    QMap<MenuAction, QVector<MenuAction> > actions;

    QVector<MenuAction> openwithMenuActionKeys;
    openwithMenuActionKeys << OpenWithCustom;
    actions.insert(OpenWith, openwithMenuActionKeys);


    QVector<MenuAction> docmentMenuActionKeys;
    docmentMenuActionKeys << NewWord
                          << NewExcel
                          << NewPowerpoint
                          << NewText;
    actions.insert(NewDocument, docmentMenuActionKeys);

    QVector<MenuAction> displayAsMenuActionKeys;
    displayAsMenuActionKeys << IconView
                          << ListView
                          << ExtendView;

    actions.insert(DisplayAs, displayAsMenuActionKeys);


    QVector<MenuAction> sortByMenuActionKeys;
    sortByMenuActionKeys << Name
                          << Size
                          << Type
                          << CreatedDate
                          << LastModifiedDate;
    actions.insert(SortBy, sortByMenuActionKeys);

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
