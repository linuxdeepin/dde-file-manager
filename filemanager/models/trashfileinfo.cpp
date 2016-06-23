#include "trashfileinfo.h"
#include "fileinfo.h"

#include "../controllers/trashmanager.h"

#include "../app/global.h"

#include "../shutil/iconprovider.h"

#include <QMimeType>

TrashFileInfo::TrashFileInfo()
    : AbstractFileInfo()
{

}

TrashFileInfo::TrashFileInfo(const DUrl &url)
    : AbstractFileInfo()
{
    TrashFileInfo::setUrl(url);
}

TrashFileInfo::TrashFileInfo(const QString &url)
    : AbstractFileInfo()
{
    TrashFileInfo::setUrl(DUrl(url));
}

bool TrashFileInfo::isCanRename() const
{
    return false;
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
    AbstractFileInfo::setUrl(fileUrl);

    data->fileInfo.setFile(TRASHFILEPATH + fileUrl.path());

    updateInfo();
}

QIcon TrashFileInfo::fileIcon() const
{
    return fileIconProvider->getFileIcon(absoluteFilePath(), mimeTypeName());
}

QString TrashFileInfo::mimeTypeName() const
{
    if(data->mimeTypeName.isNull()) {
        data->mimeTypeName = FileInfo::mimeType(absoluteFilePath()).name();
    }

    return data->mimeTypeName;
}

QFileDevice::Permissions TrashFileInfo::permissions() const
{
    QFileDevice::Permissions p = AbstractFileInfo::permissions();

    p &= ~QFileDevice::WriteOwner;
    p &= ~QFileDevice::WriteUser;
    p &= ~QFileDevice::WriteGroup;
    p &= ~QFileDevice::WriteOther;

    return p;
}

QVector<MenuAction> TrashFileInfo::menuActionList(AbstractFileInfo::MenuType type) const
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
        list << MenuAction::Property;
        list << MenuAction::SortBy;
    }

    return list;
}

quint8 TrashFileInfo::userColumnCount() const
{
    return 1;
}

QVariant TrashFileInfo::userColumnData(quint8 /*userColumnType*/) const
{
    return originalFilePath;
}

QVariant TrashFileInfo::userColumnDisplayName(quint8 /*userColumnType*/) const
{
    return QObject::tr("Source Path");
}

bool TrashFileInfo::restore(const FMEvent &event) const
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

    return fileService->renameFile(DUrl::fromLocalFile(absoluteFilePath()), DUrl::fromLocalFile(originalFilePath), event);
}

void TrashFileInfo::updateInfo()
{
    const QString &filePath = this->absoluteFilePath();
    const QString &basePath = TRASHFILEPATH;
    const QString &fileBaseName = filePath.mid(basePath.size(), filePath.indexOf('/', basePath.size() + 1) - basePath.size());

    if(QFile::exists(TRASHINFOPATH + fileBaseName + ".trashinfo")) {
        QSettings setting(TRASHINFOPATH + fileBaseName + ".trashinfo", QSettings::NativeFormat);

        setting.beginGroup("Trash Info");
        setting.setIniCodec("utf-8");

        originalFilePath = QByteArray::fromPercentEncoding(setting.value("Path").toByteArray()) + filePath.mid(basePath.size() + fileBaseName.size());


        if (originalFilePath.endsWith("desktop"))
            m_displayName = AbstractFileInfo::displayName();
        else
            m_displayName = originalFilePath.mid(originalFilePath.lastIndexOf('/') + 1);

        deletionDate = setting.value("DeletionDate").toString();
    } else {
        m_displayName = QObject::tr("Trash");
    }
}
