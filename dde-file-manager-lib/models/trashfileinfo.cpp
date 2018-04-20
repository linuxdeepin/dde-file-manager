/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "trashfileinfo.h"
#include "dfileinfo.h"
#include "private/dabstractfileinfo_p.h"
#include "controllers/trashmanager.h"
#include "dfileservices.h"
#include "controllers/pathmanager.h"

#include "app/define.h"
#include "interfaces/dfmstandardpaths.h"
#include "dfilesystemmodel.h"
#include "singleton.h"
#include "fileoperations/filejob.h"
#include "dialogs/dialogmanager.h"

#include <QMimeType>
#include <QSettings>
#include <QIcon>

namespace FileSortFunction {
COMPARE_FUN_DEFINE(deletionDate, DeletionDate, TrashFileInfo)
COMPARE_FUN_DEFINE(sourceFilePath, SourceFilePath, TrashFileInfo)
}

class TrashFileInfoPrivate : public DAbstractFileInfoPrivate
{
public:
    TrashFileInfoPrivate(const DUrl &url, TrashFileInfo *qq)
        : DAbstractFileInfoPrivate(url, qq, true) {
        columnCompact = true;
    }

    QString desktopIconName;
    QString displayName;
    QString originalFilePath;
    QString displayDeletionDate;
    QDateTime deletionDate;

    void updateInfo();
    void inheritParentTrashInfo();
};

void TrashFileInfoPrivate::updateInfo()
{
    const QString &filePath = proxy->absoluteFilePath();
    const QString &basePath = DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath);
    const QString &fileBaseName = QDir::separator() + proxy->fileName();

    if (QFile::exists(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashInfosPath) + fileBaseName + ".trashinfo")) {
        QSettings setting(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashInfosPath) + fileBaseName + ".trashinfo", QSettings::NativeFormat);

        setting.beginGroup("Trash Info");
        setting.setIniCodec("utf-8");

        originalFilePath = QByteArray::fromPercentEncoding(setting.value("Path").toByteArray()) + filePath.mid(basePath.size() + fileBaseName.size());

        displayName = originalFilePath.mid(originalFilePath.lastIndexOf('/') + 1);

        deletionDate = QDateTime::fromString(setting.value("DeletionDate").toString(), Qt::ISODate);
        displayDeletionDate = deletionDate.toString(DAbstractFileInfo::dateTimeFormat());

        if (displayDeletionDate.isEmpty())
            displayDeletionDate = setting.value("DeletionDate").toString();
    } else {
        //inherits from parent trash info
        inheritParentTrashInfo();

        // is trash root path
        if (filePath == basePath || filePath == basePath + "/") {
            displayName = QCoreApplication::translate("PathManager", "Trash");

            return;
        }

        if (systemPathManager->isSystemPath(filePath))
            displayName = systemPathManager->getSystemPathDisplayNameByPath(filePath);
        else
            displayName = proxy->fileName();
    }
}

void TrashFileInfoPrivate::inheritParentTrashInfo()
{
    const QString &filePath = proxy->absoluteFilePath();
    QString nameLayer = filePath.right(filePath.length() - DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath).length() - 1);
    QStringList names = nameLayer.split("/");

    QString name = names.takeFirst();
    QString restPath;
    foreach (QString str, names) {
        restPath += "/" + str;
    }

    if(QFile::exists(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashInfosPath) + QDir::separator() + name + ".trashinfo")){
        QSettings setting(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashInfosPath) + QDir::separator() + name + ".trashinfo", QSettings::NativeFormat);

        setting.beginGroup("Trash Info");
        setting.setIniCodec("utf-8");

        originalFilePath = QByteArray::fromPercentEncoding(setting.value("Path").toByteArray()) + restPath;

        deletionDate = QDateTime::fromString(setting.value("DeletionDate").toString(), Qt::ISODate);
        displayDeletionDate = deletionDate.toString(DAbstractFileInfo::dateTimeFormat());

        if (displayDeletionDate.isEmpty())
            displayDeletionDate = setting.value("DeletionDate").toString();
    }
}

TrashFileInfo::TrashFileInfo(const DUrl &url)
    : DAbstractFileInfo(*new TrashFileInfoPrivate(url, this))
{
    Q_D(TrashFileInfo);

    const QString &trashFilesPath = DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath);

    if (!QDir().mkpath(trashFilesPath)) {
        qWarning() << "mkpath trash files path failed, path =" << trashFilesPath;
    }

    setProxy(DAbstractFileInfoPointer(new DFileInfo(trashFilesPath + url.path())));
    d->updateInfo();
}

bool TrashFileInfo::exists() const
{
    return DAbstractFileInfo::exists() || fileUrl() == DUrl::fromTrashFile("/");
}

bool TrashFileInfo::canRename() const
{
    return false;
}

bool TrashFileInfo::isReadable() const
{
    return true;
}

bool TrashFileInfo::isWritable() const
{
    return true;
}

bool TrashFileInfo::canShare() const
{
    return false;
}

QString TrashFileInfo::fileDisplayName() const
{
    Q_D(const TrashFileInfo);

    return d->displayName;
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

            actionKeys << MenuAction::Open
                       << MenuAction::OpenInNewWindow
                       << MenuAction::Separator;
        }
        actionKeys << MenuAction::Restore
                   << MenuAction::CompleteDeletion
                   << MenuAction::Copy
                   << MenuAction::Separator
                   << MenuAction::Property;

    }else if (type == MultiFiles){
        actionKeys.reserve(12);
        if(isDir()){
            actionKeys << MenuAction::Open
                       << MenuAction::OpenInNewWindow
                       << MenuAction::Separator;
        }
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

    if (fileUrl() != DUrl::fromTrashFile("/")) {
        list << MenuAction::RestoreAll;
        list << MenuAction::ClearTrash;
    }

    if (parentUrl() != DUrl::fromTrashFile("/")) {
        list << MenuAction::Restore;
    }

    return list;
}

void TrashFileInfo::setColumnCompact(bool)
{

}

QList<int> TrashFileInfo::userColumnRoles() const
{
    static QList<int> userColumnRoles = QList<int>() << DFileSystemModel::FileUserRole + 1
                                                     << DFileSystemModel::FileUserRole + 2
                                                     << DFileSystemModel::FileDisplayNameRole
                                                     << DFileSystemModel::FileUserRole + 3 /*Source Path*/
                                                     << DFileSystemModel::FileUserRole + 4 /*Time Deleted*/
                                                     << DFileSystemModel::FileSizeRole
                                                     << DFileSystemModel::FileMimeTypeRole;

    return userColumnRoles;
}

QVariant TrashFileInfo::userColumnData(int userColumnRole) const
{
    Q_D(const TrashFileInfo);
    if (userColumnRole == DFileSystemModel::FileUserRole + 1){
        QString sourcePath;
        if (d->originalFilePath.isEmpty())
            sourcePath = d->originalFilePath;
        else
            sourcePath = QFileInfo(d->originalFilePath).absolutePath();
        return QVariant::fromValue(QPair<QString, QString>(fileDisplayName(), sourcePath));
    }
    if (userColumnRole == DFileSystemModel::FileUserRole + 2)
        return QVariant::fromValue(qMakePair(d->displayDeletionDate, qMakePair(sizeDisplayName(), mimeTypeDisplayName())));

    if (userColumnRole == DFileSystemModel::FileUserRole + 3) {
        if (d->originalFilePath.isEmpty())
            return d->originalFilePath;

        return QFileInfo(d->originalFilePath).absolutePath();
    }

    if (userColumnRole == DFileSystemModel::FileUserRole + 4)
        return d->displayDeletionDate;

    return DAbstractFileInfo::userColumnData(userColumnRole);
}

QVariant TrashFileInfo::userColumnDisplayName(int userColumnRole) const
{
    if (userColumnRole == DFileSystemModel::FileUserRole + 1)
        return qApp->translate("DFileSystemModel",  "Name");

    if (userColumnRole == DFileSystemModel::FileUserRole + 2)
        return QObject::tr("Time deleted");

    if (userColumnRole == DFileSystemModel::FileUserRole + 3)
        return QObject::tr("Source Path", "TrashFileInfo");

    if (userColumnRole == DFileSystemModel::FileUserRole + 4)
        return QObject::tr("Time deleted");

    return DAbstractFileInfo::userColumnDisplayName(userColumnRole);
}

QList<int> TrashFileInfo::userColumnChildRoles(int column) const
{
    QList<int> userColumnRoles{};
    if (column == 0){
        userColumnRoles << DFileSystemModel::FileDisplayNameRole
                        << DFileSystemModel::FileUserRole + 3;
    }else if (column == 1){
        userColumnRoles << DFileSystemModel::FileUserRole + 4
                        << DFileSystemModel::FileSizeRole
                        << DFileSystemModel::FileMimeTypeRole;
    }
    return userColumnRoles;
}

int TrashFileInfo::userColumnWidth(int userColumnRole, const QFontMetrics &fontMetrics) const
{
    if (userColumnRole == DFileSystemModel::FileUserRole + 1)
        return -1;

    return DAbstractFileInfo::userColumnWidth(userColumnRole, fontMetrics);
}

int TrashFileInfo::userRowHeight(const QFontMetrics &fontMetrics) const
{
    return fontMetrics.height() * 2 + 10;
}

bool TrashFileInfo::columnDefaultVisibleForRole(int userColumnRole) const
{
    return (userColumnRole == DFileSystemModel::FileUserRole + 1 ||
            userColumnRole == DFileSystemModel::FileUserRole + 2);
}

MenuAction TrashFileInfo::menuActionByColumnRole(int userColumnRole) const
{
    if (userColumnRole == DFileSystemModel::FileUserRole + 3)
        return MenuAction::SourcePath;

    if (userColumnRole == DFileSystemModel::FileUserRole + 4)
        return MenuAction::DeletionDate;


    return DAbstractFileInfo::menuActionByColumnRole(userColumnRole);
}

QList<int> TrashFileInfo::sortSubMenuActionUserColumnRoles() const
{
    QList<int> roles;
    roles << DFileSystemModel::FileDisplayNameRole
          << DFileSystemModel::FileUserRole + 3 /*Source Path*/
          << DFileSystemModel::FileUserRole + 4 /*Time Deleted*/
          << DFileSystemModel::FileSizeRole
          << DFileSystemModel::FileMimeTypeRole;
    return roles;
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
    return DUrl::fromLocalFile(absoluteFilePath());
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

QList<QIcon> TrashFileInfo::additionalIcon() const
{
    QList<QIcon> icons;

    if (isSymLink()) {
        icons << QIcon::fromTheme("emblem-symbolic-link", DFMGlobal::instance()->standardIcon(DFMGlobal::LinkIcon));
    }

    return icons;
}

DUrl TrashFileInfo::goToUrlWhenDeleted() const
{
    if (fileUrl() == DUrl::fromTrashFile("/")) {
        return fileUrl();
    }

    return DAbstractFileInfo::goToUrlWhenDeleted();
}

DAbstractFileInfo::CompareFunction TrashFileInfo::compareFunByColumn(int columnRole) const
{
    if (columnRole == DFileSystemModel::FileUserRole + 3)
        return FileSortFunction::compareFileListByDeletionDate;
    else if (columnRole == DFileSystemModel::FileUserRole + 4)
        return FileSortFunction::compareFileListBySourceFilePath;
    else
        return DAbstractFileInfo::compareFunByColumn(columnRole);
}

bool TrashFileInfo::restore() const
{
    Q_D(const TrashFileInfo);

    if (d->originalFilePath.isEmpty()) {
        qDebug() << "OriginalFile path ie empty.";

        return false;
    }

    QDir dir(d->originalFilePath.left(d->originalFilePath.lastIndexOf('/')));

    if (dir.isAbsolute() && !dir.mkpath(dir.absolutePath())) {
        qDebug() << "mk" << dir.absolutePath() << "failed!";

        return false;
    }

    FileJob job(FileJob::Restore);

    dialogManager->addJob(&job);

    job.doTrashRestore(absoluteFilePath(), d->originalFilePath);
    dialogManager->removeJob(job.getJobId());

    return true;
}

QDateTime TrashFileInfo::deletionDate() const
{
    Q_D(const TrashFileInfo);

    return d->deletionDate;
}

QString TrashFileInfo::sourceFilePath() const
{
    Q_D(const TrashFileInfo);

    return d->originalFilePath;
}
