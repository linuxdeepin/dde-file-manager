#include "searchfileinfo.h"

#include "../views/dfileview.h"

#include "../controllers/fileservices.h"

#include "../models/dfilesystemmodel.h"

#include "../app/global.h"

#include <QIcon>
#include <QDateTime>
#include <QUrlQuery>

namespace FileSortFunction {
SORT_FUN_DEFINE(absoluteFilePath, FilePath, SearchFileInfo)
}

SearchFileInfo::SearchFileInfo()
    : AbstractFileInfo()
{
    init();
}

SearchFileInfo::SearchFileInfo(const DUrl &url)
    : AbstractFileInfo(url)
{
    if (url.searchedFileUrl().isValid()) {
        m_parentUrl = url;
        m_parentUrl.setSearchedFileUrl(DUrl());

        realFileInfo = FileServices::instance()->createFileInfo(url.searchedFileUrl());
    } else {
        data->fileInfo = QFileInfo();
    }

    init();
}

bool SearchFileInfo::exists() const
{
    return !realFileInfo || realFileInfo->exists();
}

QString SearchFileInfo::filePath() const
{
    if (!realFileInfo)
        return QString();

    return realFileInfo->filePath();
}

QString SearchFileInfo::absoluteFilePath() const
{
    if (!realFileInfo)
        return QString();

    return realFileInfo->absoluteFilePath();
}

QString SearchFileInfo::fileName() const
{
    if (!realFileInfo)
        return QString();

    return realFileInfo->fileName();
}

QString SearchFileInfo::displayName() const
{
    if (!realFileInfo) {
        return QString();
    }

    return realFileInfo->displayName();
}

QString SearchFileInfo::path() const
{
    if (!realFileInfo)
        return QString();

    return realFileInfo->path();
}

QString SearchFileInfo::absolutePath() const
{
    if (!realFileInfo)
        return QString();

    return realFileInfo->absolutePath();
}

bool SearchFileInfo::isCanRename() const
{
    if(!realFileInfo)
        return false;

    return realFileInfo->isCanRename();
}

bool SearchFileInfo::isReadable() const
{
    if(!realFileInfo)
        return true;

    return realFileInfo->isReadable();
}

bool SearchFileInfo::isWritable() const
{
    if(!realFileInfo)
        return false;

    return realFileInfo->isWritable();
}

bool SearchFileInfo::isExecutable() const
{
    return realFileInfo && realFileInfo->isExecutable();
}

bool SearchFileInfo::isHidden() const
{
    return realFileInfo && realFileInfo->isHidden();
}

bool SearchFileInfo::isFile() const
{
    return realFileInfo && realFileInfo->isFile();
}

bool SearchFileInfo::isDir() const
{
    if(!realFileInfo)
        return true;

    return realFileInfo->isDir();
}

bool SearchFileInfo::isSymLink() const
{
    return realFileInfo && realFileInfo->isSymLink();
}

QString SearchFileInfo::owner() const
{
    if (!realFileInfo)
        return QString();

    return realFileInfo->owner();
}

uint SearchFileInfo::ownerId() const
{
    if (!realFileInfo)
        return 0;

    return realFileInfo->ownerId();
}

QString SearchFileInfo::group() const
{
    if (!realFileInfo)
        return QString();

    return realFileInfo->group();
}

uint SearchFileInfo::groupId() const
{
    if (!realFileInfo)
        return 0;

    return realFileInfo->groupId();
}

qint64 SearchFileInfo::size() const
{
    if (!realFileInfo)
        return 0;

    return realFileInfo->size();
}

QDateTime SearchFileInfo::created() const
{
    if (!realFileInfo)
        return QDateTime();

    return realFileInfo->created();
}

QDateTime SearchFileInfo::lastModified() const
{
    if (!realFileInfo)
        return QDateTime();

    return realFileInfo->lastModified();
}

QDateTime SearchFileInfo::lastRead() const
{
    if (!realFileInfo)
        return QDateTime();

    return realFileInfo->lastRead();
}

QString SearchFileInfo::lastModifiedDisplayName() const
{
    if (!realFileInfo)
        return QString();

    return realFileInfo->lastModifiedDisplayName();
}

QString SearchFileInfo::createdDisplayName() const
{
    if (!realFileInfo)
        return QString();

    return realFileInfo->createdDisplayName();
}

QString SearchFileInfo::sizeDisplayName() const
{
    if (!realFileInfo)
        return QString();

    return realFileInfo->sizeDisplayName();
}

QString SearchFileInfo::mimeTypeDisplayName() const
{
    if (!realFileInfo)
        return QString();

    return realFileInfo->mimeTypeDisplayName();
}

QMimeType SearchFileInfo::mimeType() const
{
    if (!realFileInfo)
        return QMimeType();

    return realFileInfo->mimeType();
}

QIcon SearchFileInfo::fileIcon() const
{
    if(!realFileInfo)
        return QIcon();

    return realFileInfo->fileIcon();
}

DUrl SearchFileInfo::parentUrl() const
{
    return m_parentUrl;
}

int SearchFileInfo::getIndexByFileInfo(getFileInfoFun fun, const AbstractFileInfoPointer &info,
                                       int columnRole, Qt::SortOrder order) const
{
    Q_UNUSED(columnRole)
    Q_UNUSED(order)

    /// if is file then return -1(insert last)

    if(info->isFile())
        return -1;

    int index = 0;

    forever {
        const AbstractFileInfoPointer tmp_info = fun(index);

        if(!tmp_info)
            break;

        if(tmp_info->isFile()) {
            break;
        }

        ++index;
    }

    return index;
}

QVariant SearchFileInfo::userColumnDisplayName(int userColumnRole) const
{
    if (userColumnRole == DFileSystemModel::FileUserRole + 1)
        return QObject::tr("Path", "SearchFileInfo");

    return AbstractFileInfo::userColumnDisplayName(userColumnRole);
}

QVariant SearchFileInfo::userColumnData(int userColumnRole) const
{
    if (userColumnRole == DFileSystemModel::FileUserRole + 1)
        return absoluteFilePath();

    return AbstractFileInfo::userColumnData(userColumnRole);
}

int SearchFileInfo::userColumnWidth(int userColumnRole) const
{
    if (userColumnRole == DFileSystemModel::FileUserRole + 1)
        return -1;

    return AbstractFileInfo::userColumnWidth(userColumnRole);
}

MenuAction SearchFileInfo::menuActionByColumnRole(int userColumnRole) const
{
    if (userColumnRole == DFileSystemModel::FileUserRole + 1)
        return MenuAction::AbsolutePath;

    return AbstractFileInfo::menuActionByColumnRole(userColumnRole);
}

bool SearchFileInfo::canRedirectionFileUrl() const
{
    if (realFileInfo)
        return true;

    const AbstractFileInfoPointer &targetFileInfo = FileServices::instance()->createFileInfo(DUrl(QUrlQuery(data->url).queryItemValue("url")));

    return targetFileInfo && !targetFileInfo->canIteratorDir();
}

DUrl SearchFileInfo::redirectedFileUrl() const
{
    if (realFileInfo)
        return realFileInfo->fileUrl();

    return DUrl(QUrlQuery(data->url).queryItemValue("url"));
}

QVector<MenuAction> SearchFileInfo::menuActionList(AbstractFileInfo::MenuType type) const
{
    if (!realFileInfo)
        return QVector<MenuAction>();

    QVector<MenuAction> actions = realFileInfo->menuActionList(type);

    if (type == SpaceArea)
        return actions;

    actions.insert(1, MenuAction::OpenFileLocation);

    return actions;
}

QSet<MenuAction> SearchFileInfo::disableMenuActionList() const
{
    if (!realFileInfo)
        return QSet<MenuAction>();

    QSet<MenuAction> actions = realFileInfo->disableMenuActionList();

    actions << MenuAction::DecompressHere;

    if (path().isEmpty())
        actions << MenuAction::Property << MenuAction::OpenInTerminal;

    return actions;
}

bool SearchFileInfo::isEmptyFloder() const
{
    if (path().isEmpty())
        return false;

    const AbstractFileInfoPointer &fileInfo = FileServices::instance()->createFileInfo(fileUrl().searchedFileUrl());

    return fileInfo && fileInfo->isEmptyFloder();
}

AbstractFileInfo::sortFunction SearchFileInfo::sortFunByColumn(int columnRole) const
{
    if (columnRole == DFileSystemModel::FileUserRole + 1)
        return FileSortFunction::sortFileListByFilePath;

    return AbstractFileInfo::sortFunByColumn(columnRole);
}

DUrl SearchFileInfo::getUrlByNewFileName(const QString &fileName) const
{
    DUrl url = fileUrl();

    if (realFileInfo)
        url.setSearchedFileUrl(realFileInfo->getUrlByNewFileName(fileName));

    return url;
}

QString SearchFileInfo::loadingTip() const
{
    return QObject::tr("Searching...");
}

QString SearchFileInfo::subtitleForEmptyFloder() const
{
    return QObject::tr("Files not search to");
}

void SearchFileInfo::init()
{
    m_userColumnRoles.clear();
    m_userColumnRoles << DFileSystemModel::FileUserRole + 1 << DFileSystemModel::FileSizeRole;
}
