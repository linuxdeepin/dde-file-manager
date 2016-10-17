#include "trashfileinfo.h"
#include "fileinfo.h"

#include "controllers/trashmanager.h"
#include "dfileservices.h"
#include "controllers/pathmanager.h"

#include "app/define.h"
#include "interfaces/dfmstandardpaths.h"
#include "shutil/iconprovider.h"
#include "dfilesystemmodel.h"
#include "widgets/singleton.h"

#include <QMimeType>
#include <QSettings>

namespace FileSortFunction {
SORT_FUN_DEFINE(deletionDate, DeletionDate, TrashFileInfo)
SORT_FUN_DEFINE(sourceFilePath, SourceFilePath, TrashFileInfo)
}

TrashFileInfo::TrashFileInfo()
    : DAbstractFileInfo()
{
    init();
}

TrashFileInfo::TrashFileInfo(const DUrl &url)
    : DAbstractFileInfo()
{
    TrashFileInfo::setUrl(url);
    init();
}

TrashFileInfo::TrashFileInfo(const QString &url)
    : DAbstractFileInfo()
{
    TrashFileInfo::setUrl(DUrl(url));
    init();
}

bool TrashFileInfo::isCanRename() const
{
    return false;
}

bool TrashFileInfo::isReadable() const
{
    return true;
}

bool TrashFileInfo::isWritable() const
{
    return false;
}

QString TrashFileInfo::displayName() const
{
    return m_displayName;
}

void TrashFileInfo::setUrl(const DUrl &fileUrl)
{
    DAbstractFileInfo::setUrl(fileUrl);

    data->fileInfo.setFile(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath) + fileUrl.path());

    updateInfo();
}

QIcon TrashFileInfo::fileIcon() const
{
    return fileIconProvider->getFileIcon(fileUrl(), mimeTypeName());
}

QMimeType TrashFileInfo::mimeType() const
{
    if (!data->mimeType.isValid()) {
        data->mimeType = FileInfo::mimeType(data->fileInfo.absoluteFilePath());
    }

    return data->mimeType;
}

QFileDevice::Permissions TrashFileInfo::permissions() const
{
    QFileDevice::Permissions p = DAbstractFileInfo::permissions();

    p &= ~QFileDevice::WriteOwner;
    p &= ~QFileDevice::WriteUser;
    p &= ~QFileDevice::WriteGroup;
    p &= ~QFileDevice::WriteOther;

    return p;
}

QVector<MenuAction> TrashFileInfo::menuActionList(DAbstractFileInfo::MenuType type) const
{
    QVector<MenuAction> actionKeys;

    if(type == SpaceArea) {
        actionKeys.reserve(7);

        actionKeys << MenuAction::RestoreAll
                   << MenuAction::ClearTrash
                   << MenuAction::Separator
                   << MenuAction::DisplayAs
                   << MenuAction::SortBy
                   << MenuAction::Separator
                   << MenuAction::Property;
    } else if (type == SingleFile){
        actionKeys.reserve(12);
        if(isDir()){
            actionKeys << MenuAction::OpenInNewWindow
                       << MenuAction::Separator;
        }
        actionKeys << MenuAction::Restore
                   << MenuAction::CompleteDeletion
                   << MenuAction::Copy
                   << MenuAction::Separator
                   << MenuAction::Property;

    }else if (type == MultiFiles){
        actionKeys.reserve(12);
        actionKeys << MenuAction::Restore
                   << MenuAction::CompleteDeletion
                   << MenuAction::Copy
                   << MenuAction::Separator
                   << MenuAction::Property;
    }

    return actionKeys;
}

QSet<MenuAction> TrashFileInfo::disableMenuActionList() const
{
    QSet<MenuAction> list;

    if (TrashManager::isEmpty()) {
        list << MenuAction::RestoreAll;
        list << MenuAction::ClearTrash;
    }

    return list;
}

QVariant TrashFileInfo::userColumnData(int userColumnRole) const
{
    if (userColumnRole == DFileSystemModel::FileUserRole + 1)
        return displayDeletionDate;

    if (userColumnRole == DFileSystemModel::FileUserRole + 2)
        return originalFilePath;

    return DAbstractFileInfo::userColumnData(userColumnRole);
}

QVariant TrashFileInfo::userColumnDisplayName(int userColumnRole) const
{
    if (userColumnRole == DFileSystemModel::FileUserRole + 1)
        return QObject::tr("Time deleted");

    if (userColumnRole == DFileSystemModel::FileUserRole + 2)
        return QObject::tr("Path", "TrashFileInfo");

    return DAbstractFileInfo::userColumnDisplayName(userColumnRole);
}

int TrashFileInfo::userColumnWidth(int userColumnRole) const
{
    if (userColumnRole == DFileSystemModel::FileUserRole + 2)
        return -1;

    return DAbstractFileInfo::userColumnWidth(userColumnRole);
}

bool TrashFileInfo::columnDefaultVisibleForRole(int userColumnRole) const
{
    if (userColumnRole == DFileSystemModel::FileLastModifiedRole)
        return false;

    return DAbstractFileInfo::columnDefaultVisibleForRole(userColumnRole);
}

MenuAction TrashFileInfo::menuActionByColumnRole(int userColumnRole) const
{
    if (userColumnRole == DFileSystemModel::FileUserRole + 1)
        return MenuAction::DeletionDate;

    if (userColumnRole == DFileSystemModel::FileUserRole + 2)
        return MenuAction::SourcePath;

    return DAbstractFileInfo::menuActionByColumnRole(userColumnRole);
}

bool TrashFileInfo::canIteratorDir() const
{
    return true;
}

bool TrashFileInfo::makeAbsolute()
{
    return true;
}

DUrl TrashFileInfo::mimeDataUrl() const
{
    return DUrl::fromLocalFile(data->fileInfo.absoluteFilePath());
}

Qt::DropActions TrashFileInfo::supportedDragActions() const
{
    return Qt::CopyAction;
}

Qt::DropActions TrashFileInfo::supportedDropActions() const
{
    const QString &path = fileUrl().path();

    return path.isEmpty() || path == "/" ? Qt::MoveAction : Qt::IgnoreAction;
}

DAbstractFileInfo::sortFunction TrashFileInfo::sortFunByColumn(int columnRole) const
{
    if (columnRole == DFileSystemModel::FileUserRole + 1)
        return FileSortFunction::sortFileListByDeletionDate;
    else if (columnRole == DFileSystemModel::FileUserRole + 2)
        return FileSortFunction::sortFileListBySourceFilePath;
    else
        return DAbstractFileInfo::sortFunByColumn(columnRole);
}

bool TrashFileInfo::restore(const DFMEvent &event) const
{
    if(originalFilePath.isEmpty()) {
        qDebug() << "OriginalFile path ie empty.";

        return false;
    }

    QDir dir(originalFilePath.left(originalFilePath.lastIndexOf('/')));

    if(dir.isAbsolute() && !dir.mkpath(dir.absolutePath())) {
        qDebug() << "mk" << dir.absolutePath() << "failed!";

        return false;
    }

    DUrl srcUrl = DUrl::fromLocalFile(data->fileInfo.absoluteFilePath());
    DUrl tarUrl = DUrl::fromLocalFile(originalFilePath);
    fileService->restoreFile(srcUrl, tarUrl, event);

    return true;
}

QDateTime TrashFileInfo::deletionDate() const
{
    return m_deletionDate;
}

QString TrashFileInfo::sourceFilePath() const
{
    return originalFilePath;
}

void TrashFileInfo::updateInfo()
{
    const QString &filePath = data->fileInfo.absoluteFilePath();
    const QString &basePath = DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath);
    const QString &fileBaseName = filePath.mid(basePath.size(), filePath.indexOf('/', basePath.size() + 1) - basePath.size());

    if (QFile::exists(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashInfosPath) + fileBaseName + ".trashinfo")) {
        QSettings setting(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashInfosPath) + fileBaseName + ".trashinfo", QSettings::NativeFormat);

        setting.beginGroup("Trash Info");
        setting.setIniCodec("utf-8");

        originalFilePath = QByteArray::fromPercentEncoding(setting.value("Path").toByteArray()) + filePath.mid(basePath.size() + fileBaseName.size());

        m_displayName = originalFilePath.mid(originalFilePath.lastIndexOf('/') + 1);

        m_deletionDate = QDateTime::fromString(setting.value("DeletionDate").toString(), Qt::ISODate);
        displayDeletionDate = m_deletionDate.toString(timeFormat());

        if (displayDeletionDate.isEmpty())
            displayDeletionDate = setting.value("DeletionDate").toString();
    } else {
        if (systemPathManager->isSystemPath(filePath))
            m_displayName = systemPathManager->getSystemPathDisplayNameByPath(filePath);
        else
            m_displayName = fileName();
    }
}

void TrashFileInfo::init()
{
    m_userColumnRoles.prepend(DFileSystemModel::FileUserRole + 1);
    m_userColumnRoles << DFileSystemModel::FileUserRole + 2;
}
