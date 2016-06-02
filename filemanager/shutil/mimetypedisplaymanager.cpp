#include "mimetypedisplaymanager.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include "standardpath.h"

QStringList MimeTypeDisplayManager::ArchiveMimeTypes;
QStringList MimeTypeDisplayManager::TextMimeTypes;
QStringList MimeTypeDisplayManager::VideoMimeTypes;
QStringList MimeTypeDisplayManager::AudioMimeTypes;
QStringList MimeTypeDisplayManager::ImageMimeTypes;
QStringList MimeTypeDisplayManager::ExecutableMimeTypes;


MimeTypeDisplayManager::MimeTypeDisplayManager(QObject *parent) : QObject(parent)
{
    initData();
    initConnect();
}

MimeTypeDisplayManager::~MimeTypeDisplayManager()
{

}

void MimeTypeDisplayManager::initData()
{
    m_displayNames["directory"] = tr("directory");
    m_displayNames["desktop"] = tr("application");
    m_displayNames["video"] = tr("video");
    m_displayNames["audio"] = tr("audio");
    m_displayNames["image"] = tr("image");
    m_displayNames["archive"] = tr("archive");
    m_displayNames["text"] = tr("text");
    m_displayNames["executable"] = tr("executable");
    m_displayNames["unknown"] = tr("unknown");


    m_defaultIconNames["directory"] = "inode/directory";
    m_defaultIconNames["desktop"] = "application-default-icon";
    m_defaultIconNames["video"] = "video";
    m_defaultIconNames["audio"] = "music";
    m_defaultIconNames["image"] = "image";
    m_defaultIconNames["archive"] = "application-x-archive";
    m_defaultIconNames["text"] = "text-plain";
    m_defaultIconNames["executable"] = "application-x-executable";
    m_defaultIconNames["unknown"] = "application-default-icon";

    loadSupportMimeTypes();
}

void MimeTypeDisplayManager::initConnect()
{

}

QString MimeTypeDisplayManager::displayName(const QString &mimeType)
{
    if (mimeType == "application/x-desktop"){
        return m_displayNames.value("desktop");
    }else if (mimeType == "inode/directory"){
        return m_displayNames.value("directory");
    }else if (mimeType == "application/x-executable" || ExecutableMimeTypes.contains(mimeType)){
        return m_displayNames.value("executable");
    }else if (mimeType.startsWith("video/") || VideoMimeTypes.contains(mimeType)){
        return m_displayNames.value("video");
    }else if (mimeType.startsWith("audio/") || AudioMimeTypes.contains(mimeType)){
        return m_displayNames.value("audio");
    }else if (mimeType.startsWith("image/") || ImageMimeTypes.contains(mimeType)){
        return m_displayNames.value("image");
    }else if (mimeType.startsWith("text/") || TextMimeTypes.contains(mimeType)){
        return m_displayNames.value("text");
    }else if (ArchiveMimeTypes.contains(mimeType)){
        return m_displayNames.value("archive");
    }else{
        return m_displayNames.value("unknown");
    }
}

MimeTypeDisplayManager::MimeDisplayNameOrder MimeTypeDisplayManager::displayNameOrder(const QString &mimeType)
{
    if (mimeType == "application/x-desktop"){
        return DesktopApplication;
    }else if (mimeType == "inode/directory"){
        return Directory;
    }else if (mimeType == "application/x-executable" || ExecutableMimeTypes.contains(mimeType)){
        return Executable;
    }else if (mimeType.startsWith("video/") || VideoMimeTypes.contains(mimeType)){
        return Video;
    }else if (mimeType.startsWith("audio/") || AudioMimeTypes.contains(mimeType)){
        return Audio;
    }else if (mimeType.startsWith("image/") || ImageMimeTypes.contains(mimeType)){
        return Image;
    }else if (mimeType.startsWith("text/") || TextMimeTypes.contains(mimeType)){
        return Text;
    }else if (ArchiveMimeTypes.contains(mimeType)){
        return Archive;
    }else{
        return Unknown;
    }
}

QString MimeTypeDisplayManager::defaultIcon(const QString &mimeType)
{
    if (mimeType == "application/x-desktop"){
        return m_defaultIconNames.value("desktop");
    }else if (mimeType == "inode/directory"){
        return m_defaultIconNames.value("directory");
    }else if (mimeType == "application/x-executable" || ExecutableMimeTypes.contains(mimeType)){
        return m_defaultIconNames.value("executable");
    }else if (mimeType.startsWith("video/") || VideoMimeTypes.contains(mimeType)){
        return m_defaultIconNames.value("video");
    }else if (mimeType.startsWith("audio/") || AudioMimeTypes.contains(mimeType)){
        return m_defaultIconNames.value("audio");
    }else if (mimeType.startsWith("image/") || ImageMimeTypes.contains(mimeType)){
        return m_defaultIconNames.value("image");
    }else if (mimeType.startsWith("text/") || TextMimeTypes.contains(mimeType)){
        return m_defaultIconNames.value("text");
    }else if (ArchiveMimeTypes.contains(mimeType)){
        return m_defaultIconNames.value("archive");
    }else{
        return m_defaultIconNames.value("unknown");
    }
}

QStringList MimeTypeDisplayManager::readlines(const QString &path)
{
    QStringList result;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
      return result;
    }
    QTextStream in(&file);
    while (!in.atEnd()) {
      // Read new line
      QString line = in.readLine();
      // Skip empty line or line with invalid format
      if (line.trimmed().isEmpty()) {
        continue;
      }
      result.append(line.trimmed());
    }
    file.close();
    return result;
}

void MimeTypeDisplayManager::loadSupportMimeTypes()
{
    QString textPath = QString("%1/%2/%3").arg(StandardPath::getAppConfigPath(), "mimetypes", "text.mimetype");
    QString archivePath = QString("%1/%2/%3").arg(StandardPath::getAppConfigPath(), "mimetypes", "archive.mimetype");
    QString videoPath = QString("%1/%2/%3").arg(StandardPath::getAppConfigPath(), "mimetypes", "video.mimetype");
    QString audioPath = QString("%1/%2/%3").arg(StandardPath::getAppConfigPath(), "mimetypes", "audio.mimetype");
    QString imagePath = QString("%1/%2/%3").arg(StandardPath::getAppConfigPath(), "mimetypes", "image.mimetype");
    QString executablePath = QString("%1/%2/%3").arg(StandardPath::getAppConfigPath(), "mimetypes", "executable.mimetype");
    TextMimeTypes = readlines(textPath);
    ArchiveMimeTypes = readlines(archivePath);
    VideoMimeTypes = readlines(videoPath);
    AudioMimeTypes = readlines(audioPath);
    ImageMimeTypes = readlines(imagePath);
    ExecutableMimeTypes = readlines(executablePath);
 }

