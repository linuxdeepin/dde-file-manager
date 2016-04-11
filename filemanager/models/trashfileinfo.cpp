#include "trashfileinfo.h"

#include "../app/global.h"

#include "../shutil/iconprovider.h"

TrashFileInfo::TrashFileInfo()
    : AbstractFileInfo()
{

}

TrashFileInfo::TrashFileInfo(const QUrl &url)
    : AbstractFileInfo()
{
    TrashFileInfo::setUrl(url);
}

TrashFileInfo::TrashFileInfo(const QString &url)
    : AbstractFileInfo()
{
    TrashFileInfo::setUrl(url);
}

bool TrashFileInfo::isCanRename() const
{
    return false;
}

bool TrashFileInfo::isWritable() const
{
    return false;
}

bool TrashFileInfo::isReadable() const
{
    if(absoluteFilePath() == "/")
        return true;

    return AbstractFileInfo::isReadable();
}

bool TrashFileInfo::isDir() const
{
    if(absoluteFilePath() == "/")
        return true;

    return AbstractFileInfo::isDir();
}

QString TrashFileInfo::displayName() const
{
    return m_displayName;
}

void TrashFileInfo::setUrl(const QString &fileUrl)
{
    setUrl(QUrl(fileUrl));
}

void TrashFileInfo::setUrl(const QUrl &fileUrl)
{
    AbstractFileInfo::setUrl(fileUrl);

    if(fileUrl.path() != "/")
        updateInfo();
}

QIcon TrashFileInfo::fileIcon() const
{
    return iconProvider->getFileIcon(absoluteFilePath());
}

QString TrashFileInfo::parentUrl() const
{
    return scheme() + ":///";
}

void TrashFileInfo::updateInfo()
{
    QString user = getenv("USER");
    QString infoPath = "/home/" + user + "/.local/share/Trash/info/";

    QSettings setting(infoPath + fileName() + ".trashinfo", QSettings::IniFormat);

    setting.beginGroup("Trash Info");

    originalPath = setting.value("Path").toString();
    deletionDate = setting.value("DeletionDate").toString();

    m_displayName = originalPath.mid(originalPath.lastIndexOf('/') + 1);
}
