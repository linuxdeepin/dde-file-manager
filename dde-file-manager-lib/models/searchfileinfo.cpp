#include "searchfileinfo.h"
#include "private/dabstractfileinfo_p.h"
#include "views/dfileview.h"

#include "dfileservices.h"

#include "dfilesystemmodel.h"

#include "app/define.h"

#include <QIcon>
#include <QDateTime>
#include <QUrlQuery>

namespace FileSortFunction {
COMPARE_FUN_DEFINE(absoluteFilePath, FilePath, SearchFileInfo)
}

SearchFileInfo::SearchFileInfo(const DUrl &url)
    : DAbstractFileInfo(url)
{
    if (url.searchedFileUrl().isValid()) {
        m_parentUrl = url;
        m_parentUrl.setSearchedFileUrl(DUrl());

        setProxy(DFileService::instance()->createFileInfo(Q_NULLPTR, url.searchedFileUrl()));
    }
}

bool SearchFileInfo::exists() const
{
    Q_D(const DAbstractFileInfo);

    return !d->proxy || d->proxy->exists();
}

bool SearchFileInfo::isReadable() const
{
    Q_D(const DAbstractFileInfo);

    if (!d->proxy)
        return true;

    return d->proxy->isReadable();
}

bool SearchFileInfo::isDir() const
{
    Q_D(const DAbstractFileInfo);

    if (!d->proxy)
        return true;

    return d->proxy->isDir();
}

int SearchFileInfo::filesCount() const
{
    Q_D(const DAbstractFileInfo);

    if (!d->proxy)
        return 0;

    return d->proxy->filesCount();
}

DUrl SearchFileInfo::parentUrl() const
{
    return m_parentUrl;
}

QList<int> SearchFileInfo::userColumnRoles() const
{
    static QList<int> userColumnRoles = QList<int>() << DFileSystemModel::FileUserRole + 1
                                                     << DFileSystemModel::FileLastModifiedRole
                                                     << DFileSystemModel::FileSizeRole
                                                     << DFileSystemModel::FileMimeTypeRole;

    return userColumnRoles;
}

QVariant SearchFileInfo::userColumnDisplayName(int userColumnRole) const
{
    if (userColumnRole == DFileSystemModel::FileUserRole + 1)
        return QObject::tr("Path", "SearchFileInfo");

    return DAbstractFileInfo::userColumnDisplayName(userColumnRole);
}

QVariant SearchFileInfo::userColumnData(int userColumnRole) const
{
    Q_D(const DAbstractFileInfo);

    if (userColumnRole == DFileSystemModel::FileUserRole + 1) {
        const DUrl &fileUrl = d->proxy->fileUrl();

        if (fileUrl.isLocalFile()) {
            return absolutePath();
        } else {
            DUrl newUrl = fileUrl;

            newUrl.setPath(QFileInfo(newUrl.path()).path());

            return newUrl.toString();
        }
    }

    return DAbstractFileInfo::userColumnData(userColumnRole);
}

int SearchFileInfo::userColumnWidth(int userColumnRole, const QFontMetrics &fontMetrics) const
{
    if (userColumnRole == DFileSystemModel::FileUserRole + 1)
        return -1;

    return DAbstractFileInfo::userColumnWidth(userColumnRole, fontMetrics);
}

MenuAction SearchFileInfo::menuActionByColumnRole(int userColumnRole) const
{
    if (userColumnRole == DFileSystemModel::FileUserRole + 1)
        return MenuAction::AbsolutePath;

    return DAbstractFileInfo::menuActionByColumnRole(userColumnRole);
}

bool SearchFileInfo::canRedirectionFileUrl() const
{
    Q_D(const DAbstractFileInfo);

    if (d->proxy)
        return true;

    const DAbstractFileInfoPointer &targetFileInfo = DFileService::instance()->createFileInfo(Q_NULLPTR, fileUrl().searchTargetUrl());

    return targetFileInfo && !targetFileInfo->canIteratorDir();
}

DUrl SearchFileInfo::redirectedFileUrl() const
{
    Q_D(const DAbstractFileInfo);

    if (d->proxy)
        return d->proxy->fileUrl();

    return fileUrl().searchTargetUrl();
}

bool SearchFileInfo::canIteratorDir() const
{
    Q_D(const DAbstractFileInfo);

    return !d->proxy;
}

QVector<MenuAction> SearchFileInfo::menuActionList(DAbstractFileInfo::MenuType type) const
{
    Q_D(const DAbstractFileInfo);

    QVector<MenuAction> actions;

    if (!d->proxy && type == SpaceArea) {
        actions << MenuAction::DisplayAs;
        actions << MenuAction::SortBy;
        actions << MenuAction::SelectAll;

        return actions;
    }

    actions = d->proxy->menuActionList(type);
    actions.insert(1, MenuAction::OpenFileLocation);

    return actions;
}

QSet<MenuAction> SearchFileInfo::disableMenuActionList() const
{
    Q_D(const DAbstractFileInfo);

    if (!d->proxy)
        return QSet<MenuAction>();

    QSet<MenuAction> actions = d->proxy->disableMenuActionList();

    actions << MenuAction::DecompressHere;

    if (path().isEmpty())
        actions << MenuAction::Property << MenuAction::OpenInTerminal;

    return actions;
}

bool SearchFileInfo::isEmptyFloder(const QDir::Filters &filters) const
{
    if (path().isEmpty())
        return false;

    const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(Q_NULLPTR, fileUrl().searchedFileUrl());

    return fileInfo && fileInfo->isEmptyFloder(filters);
}

DAbstractFileInfo::CompareFunction SearchFileInfo::compareFunByColumn(int columnRole) const
{
    if (columnRole == DFileSystemModel::FileUserRole + 1)
        return FileSortFunction::compareFileListByFilePath;

    return DAbstractFileInfo::compareFunByColumn(columnRole);
}

bool SearchFileInfo::hasOrderly() const
{
    return false;
}

DUrl SearchFileInfo::getUrlByNewFileName(const QString &fileName) const
{
    Q_D(const DAbstractFileInfo);

    DUrl url = fileUrl();

    if (d->proxy)
        url.setSearchedFileUrl(d->proxy->getUrlByNewFileName(fileName));

    return url;
}

QString SearchFileInfo::loadingTip() const
{
    return QObject::tr("Searching...");
}

QString SearchFileInfo::subtitleForEmptyFloder() const
{
    return QObject::tr("No results");
}

QString SearchFileInfo::fileDisplayName() const
{
    Q_D(const DAbstractFileInfo);

    if (d->proxy)
        return d->proxy->fileDisplayName();

    const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(Q_NULLPTR, fileUrl().searchTargetUrl());

    if (fileInfo)
        return fileInfo->fileDisplayName();

    return QString();
}

DUrl SearchFileInfo::mimeDataUrl() const
{
    const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(Q_NULLPTR, fileUrl().searchedFileUrl());

    if (info)
        return info->mimeDataUrl();

    return fileUrl().searchedFileUrl();
}

QString SearchFileInfo::toLocalFile() const
{
    const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(Q_NULLPTR, fileUrl().searchedFileUrl());

    if (info)
        return info->toLocalFile();

    return fileUrl().searchedFileUrl().toLocalFile();
}
