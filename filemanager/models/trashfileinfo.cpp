#include "trashfileinfo.h"
#include "fileinfo.h"

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

    QString user = getenv("USER");
    QString pathPrefix = "/home/" + user + "/.local/share/Trash/files";

    data->fileInfo.setFile(pathPrefix + fileUrl.path());

    updateInfo();
}

QIcon TrashFileInfo::fileIcon() const
{
    return iconProvider->getFileIcon(absoluteFilePath());
}

QString TrashFileInfo::mimeTypeName() const
{
    if(data->mimeTypeName.isNull()) {
        QString user = getenv("USER");
        QString pathPrefix = "/home/" + user + "/.local/share/Trash/files";

        data->mimeTypeName = FileInfo::mimeType(pathPrefix + this->absoluteFilePath()).name();
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

QVector<AbstractFileInfo::MenuAction> TrashFileInfo::menuActionList(AbstractFileInfo::MenuType type) const
{
    QVector<MenuAction> actionKeys;

    if(type == SpaceArea) {
        actionKeys.reserve(7);

        actionKeys << MenuAction::OpenInNewWindow
                   << MenuAction::Separator
                   << MenuAction::Paste
                   << MenuAction::SelectAll
                   << MenuAction::ClearTrash
                   << MenuAction::Separator
                   << MenuAction::Property;
    } else {
        actionKeys.reserve(12);

        actionKeys << MenuAction::Open << (isDir() ? OpenInNewWindow : OpenWith)
                   << MenuAction::Separator
                   << MenuAction::Compress << MenuAction::Separator
                   << MenuAction::Copy << MenuAction::Cut << MenuAction::Separator
                   << MenuAction::Restore << MenuAction::CompleteDeletion
                   << MenuAction::Separator
                   << MenuAction::Property;
    }

    return actionKeys;
}

void TrashFileInfo::updateInfo()
{
    QString user = getenv("USER");
    QString infoPath = "/home/" + user + "/.local/share/Trash/info/";

    QSettings setting(infoPath + fileName() + ".trashinfo", QSettings::IniFormat);

    setting.beginGroup("Trash Info");

    if(absoluteFilePath() == "/home/" + user + "/.local/share/Trash/files/") {
        originalPath = setting.value("Path").toString();
        m_displayName = originalPath.mid(originalPath.lastIndexOf('/') + 1);
    } else {
        originalPath = this->absoluteFilePath();
        m_displayName = this->fileName();
    }

    deletionDate = setting.value("DeletionDate").toString();
}
