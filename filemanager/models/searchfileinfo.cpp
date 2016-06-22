#include "searchfileinfo.h"

#include "../views/dfileview.h"

#include "../controllers/fileservices.h"

#include <QIcon>
#include <QDateTime>

SearchFileInfo::SearchFileInfo()
    : AbstractFileInfo()
{

}

SearchFileInfo::SearchFileInfo(const DUrl &url)
    : AbstractFileInfo(url)
{
    const QString &fragment = url.fragment();

    if(!fragment.isEmpty() && !url.isStopSearch()) {
        m_parentUrl = url;
        m_parentUrl.setFragment(QString());

        realFileInfo = FileServices::instance()->createFileInfo(DUrl(fragment));
    } else {
        data->fileInfo = QFileInfo();
    }
}

bool SearchFileInfo::exists() const
{
    return realFileInfo && realFileInfo->exists();
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
                                       quint8 columnType, Qt::SortOrder order) const
{
    Q_UNUSED(columnType)
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

quint8 SearchFileInfo::userColumnCount() const
{
    return 1;
}

QVariant SearchFileInfo::userColumnDisplayName(quint8 userColumnType) const
{
    Q_UNUSED(userColumnType)

    return QObject::tr("absolute path");
}

QVariant SearchFileInfo::userColumnData(quint8 userColumnType) const
{
    Q_UNUSED(userColumnType)

    return absoluteFilePath();
}

bool SearchFileInfo::canRedirectionFileUrl() const
{
    return realFileInfo;
}

DUrl SearchFileInfo::redirectedFileUrl() const
{
    return realFileInfo->fileUrl();
}

QVector<MenuAction> SearchFileInfo::menuActionList(AbstractFileInfo::MenuType type) const
{
    QVector<MenuAction> actions = AbstractFileInfo::menuActionList(type);

    if (type == SpaceArea)
        return actions;

    actions.insert(1, MenuAction::OpenFileLocation);

    return actions;
}

QSet<MenuAction> SearchFileInfo::disableMenuActionList() const
{
    QSet<MenuAction> actions = AbstractFileInfo::disableMenuActionList();

    actions << MenuAction::OpenInTerminal;

    if (path().isEmpty())
        actions << MenuAction::Property;

    return actions;
}

bool SearchFileInfo::isEmptyFloder() const
{
    if (path().isEmpty())
        return false;

    const AbstractFileInfoPointer &fileInfo = FileServices::instance()->createFileInfo(DUrl(fileUrl().fragment()));

    return fileInfo && fileInfo->isEmptyFloder();
}
