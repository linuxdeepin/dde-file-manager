// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfm-base/mimetype/mimedatabase.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/base/standardpaths.h"

#include <QFileInfo>
#include <QRegularExpression>
#include <QMimeDatabase>
#include <QString>
#include <QHash>
#include <QtConcurrent>

using namespace dfmbase;

Q_GLOBAL_STATIC(QMimeDatabase, mimedb)

// global mimetypes define list cache
Q_GLOBAL_STATIC(QStringList, mimeArchiveTypes);
Q_GLOBAL_STATIC(QStringList, mimeTextTypes)
Q_GLOBAL_STATIC(QStringList, mimeVideoTypes)
Q_GLOBAL_STATIC(QStringList, mimeAudioTypes)
Q_GLOBAL_STATIC(QStringList, mimeImageTypes)
Q_GLOBAL_STATIC(QStringList, mimeExecutableTypes)
Q_GLOBAL_STATIC(QStringList, mimeBackupTypes)

typedef QHash<int, QString> kiVsHash;
Q_GLOBAL_STATIC(kiVsHash, mimeDisplayNames)
Q_GLOBAL_STATIC(kiVsHash, mimeStdIconNames)

/*!
 * \brief readlines 读取文件的所有行数据
 *
 * \param path 文件路径
 *
 * \return QStringList 文件所有内容
 */
QStringList readlines(const QString &path)
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
/*!
 * \brief loadSupportMimeTypes 加载支持的Mimetype
 *
 * \return bool 是否加载成功
 */
bool loadSupportMimeTypes()
{
    QString textPath = QString("%1/%2").arg(StandardPaths::location(StandardPaths::kMimeTypePath), "text.mimetype");
    QString archivePath = QString("%1/%2").arg(StandardPaths::location(StandardPaths::kMimeTypePath), "archive.mimetype");
    QString videoPath = QString("%1/%2").arg(StandardPaths::location(StandardPaths::kMimeTypePath), "video.mimetype");
    QString audioPath = QString("%1/%2").arg(StandardPaths::location(StandardPaths::kMimeTypePath), "audio.mimetype");
    QString imagePath = QString("%1/%2").arg(StandardPaths::location(StandardPaths::kMimeTypePath), "image.mimetype");
    QString executablePath = QString("%1/%2").arg(StandardPaths::location(StandardPaths::kMimeTypePath), "executable.mimetype");
    QString backupPath = QString("%1/%2").arg(StandardPaths::location(StandardPaths::kMimeTypePath), "backup.mimetype");
    *mimeTextTypes = readlines(textPath);
    *mimeArchiveTypes = readlines(archivePath);
    *mimeVideoTypes = readlines(videoPath);
    *mimeAudioTypes = readlines(audioPath);
    *mimeImageTypes = readlines(imagePath);
    *mimeExecutableTypes = readlines(executablePath);
    *mimeBackupTypes = readlines(backupPath);
    return true;
}
/*!
 * \brief loadFileTypeName 加载文件的基本类型格式
 *
 * \return bool 是否加载成功
 */
bool loadFileTypeName()
{
    (*mimeDisplayNames)[static_cast<int>(MimeDatabase::FileType::kDirectory)] = QObject::tr("Directory");
    (*mimeDisplayNames)[static_cast<int>(MimeDatabase::FileType::kDesktopApplication)] = QObject::tr("Application");
    (*mimeDisplayNames)[static_cast<int>(MimeDatabase::FileType::kVideos)] = QObject::tr("Video");
    (*mimeDisplayNames)[static_cast<int>(MimeDatabase::FileType::kAudios)] = QObject::tr("Audio");
    (*mimeDisplayNames)[static_cast<int>(MimeDatabase::FileType::kImages)] = QObject::tr("Image");
    (*mimeDisplayNames)[static_cast<int>(MimeDatabase::FileType::kArchives)] = QObject::tr("Archive");
    (*mimeDisplayNames)[static_cast<int>(MimeDatabase::FileType::kDocuments)] = QObject::tr("Text");
    (*mimeDisplayNames)[static_cast<int>(MimeDatabase::FileType::kExecutable)] = QObject::tr("Executable");
    (*mimeDisplayNames)[static_cast<int>(MimeDatabase::FileType::kBackups)] = QObject::tr("Backup file");
    (*mimeDisplayNames)[static_cast<int>(MimeDatabase::FileType::kUnknown)] = QObject::tr("Unknown");
    return true;
}
/*!
 * \brief loadMimeStdIcon 加载标准的文件格式对应的icon
 *
 * \return bool 是否加载成功
 */
bool loadMimeStdIcon()
{
    (*mimeStdIconNames)[static_cast<int>(MimeDatabase::FileType::kDirectory)] = "folder";
    (*mimeStdIconNames)[static_cast<int>(MimeDatabase::FileType::kDesktopApplication)] = "application-default-icon";
    (*mimeStdIconNames)[static_cast<int>(MimeDatabase::FileType::kVideos)] = "video";
    (*mimeStdIconNames)[static_cast<int>(MimeDatabase::FileType::kAudios)] = "music";
    (*mimeStdIconNames)[static_cast<int>(MimeDatabase::FileType::kImages)] = "image";
    (*mimeStdIconNames)[static_cast<int>(MimeDatabase::FileType::kArchives)] = "application-x-archive";
    (*mimeStdIconNames)[static_cast<int>(MimeDatabase::FileType::kDocuments)] = "text-plain";
    (*mimeStdIconNames)[static_cast<int>(MimeDatabase::FileType::kExecutable)] = "application-x-executable";
    (*mimeStdIconNames)[static_cast<int>(MimeDatabase::FileType::kBackups)] = "application-x-archive";   // generic backup file icon?
    (*mimeStdIconNames)[static_cast<int>(MimeDatabase::FileType::kUnknown)] = "application-default-icon";

    return true;
}

static auto kThreadLoadMimeTypes = QtConcurrent::run(loadSupportMimeTypes);
static auto kThreadLoadStdIcons = QtConcurrent::run(loadMimeStdIcon);
static auto kThreadLoadMimeNames = QtConcurrent::run(loadFileTypeName);

MimeDatabase::MimeDatabase()
{
    Q_UNUSED(kThreadLoadMimeTypes);
    Q_UNUSED(kThreadLoadStdIcons);
    Q_UNUSED(kThreadLoadMimeNames);
}

MimeDatabase::~MimeDatabase()
{
}
/*!
 * \brief MimeDatabase::mimeFileType 获取文件的mimetype
 *
 * \param mimeType
 *
 * \return QString 返回文件的mimetype
 */
QString MimeDatabase::mimeFileType(const QString &mimeType)
{
    Q_UNUSED(mimeType)
    //todo:
    //#ifdef QT_DEBUG
    //    return mimeDisplayNames->value(mimeFileTypeNameToEnum(mimeType)) + " (" + mimeType + ")";
    //#else // Q_DEBUG
    //    return mimeDisplayNames->value(mimeFileTypeToEnum(mimeType));
    //#endif
    return "";
}
/*!
 * \brief MimeDatabase::mimeFileTypeNameToEnum 获取文件的filetype
 *
 * \param mimeFileTypeName 文件的mimetype
 *
 * \return FileType 文件filetype
 */
MimeDatabase::FileType MimeDatabase::mimeFileTypeNameToEnum(const QString &mimeFileTypeName)
{
    if (mimeFileTypeName == "application/x-desktop") {
        return FileType::kDesktopApplication;
    } else if (mimeFileTypeName == "inode/directory") {
        return FileType::kDirectory;
    } else if (mimeFileTypeName == "application/x-executable"
               || mimeExecutableTypes->contains(mimeFileTypeName)) {
        return FileType::kExecutable;
    } else if (mimeFileTypeName.startsWith("video/")
               || mimeVideoTypes->contains(mimeFileTypeName)) {
        return FileType::kVideos;
    } else if (mimeFileTypeName.startsWith("audio/")
               || mimeAudioTypes->contains(mimeFileTypeName)) {
        return FileType::kAudios;
    } else if (mimeFileTypeName.startsWith("image/")
               || mimeImageTypes->contains(mimeFileTypeName)) {
        return FileType::kImages;
    } else if (mimeFileTypeName.startsWith("text/")
               || mimeTextTypes->contains(mimeFileTypeName)) {
        return FileType::kDocuments;
    } else if (mimeArchiveTypes->contains(mimeFileTypeName)) {
        return FileType::kArchives;
    } else if (mimeBackupTypes->contains(mimeFileTypeName)) {
        return FileType::kBackups;
    } else {
        return FileType::kUnknown;
    }
}
/*!
 * \brief MimeDatabase::supportMimeFileType 根据文件的filetype获取文件的mimetype
 *
 * \param mimeFileType 文件的filetype
 *
 * \return QStringList 文件的mimetype
 */
QStringList MimeDatabase::supportMimeFileType(MimeDatabase::FileType mimeFileType)
{
    QStringList list;

    switch (mimeFileType) {
    case FileType::kDocuments:
        return list;   //empty
    case FileType::kImages:
        return *mimeImageTypes;
    case FileType::kVideos:
        return *mimeVideoTypes;
    case FileType::kAudios:
        return *mimeAudioTypes;
    case FileType::kArchives:
        return *mimeArchiveTypes;
    case FileType::kDesktopApplication:
        return list;   //empty
    case FileType::kExecutable:
        return *mimeExecutableTypes;
    case FileType::kBackups:
        return *mimeBackupTypes;
    default:
        return list;   //empty
    }
}
/*!
 * \brief MimeDatabase::mimeStdIcon 根据文件的mimetype获取文件的标准icon
 *
 * \param mimeType 文件的mimetype
 *
 * \return QString 标注icon的名称
 */
QString MimeDatabase::mimeStdIcon(const QString &mimeType)
{
    return mimeStdIconNames->value(static_cast<int>(mimeFileTypeNameToEnum(mimeType)));
}
/*!
 * \brief MimeDatabase::mimeTypeForFile 获取一个文件的mimetype，调用的是qt的mimeTypeForFile
 *
 * \param fileName 文件名称
 *
 * \param mode 匹配模式（MatchDefault、MatchExtension、MatchContent）其中MatchExtension获取文件的mimetype最快
 *
 * \return QMimeType 文件的mimetype
 */
QMimeType MimeDatabase::mimeTypeForFile(const QString &fileName, QMimeDatabase::MatchMode mode)
{
    return mimedb->mimeTypeForFile(fileName, mode);
}
/*!
 * \brief MimeDatabase::mimeTypeForFile 获取一个文件的mimetype
 *
 * \param fileInfo 文件的QFileInfo
 *
 * \param mode 匹配模式（MatchDefault、MatchExtension、MatchContent）其中MatchExtension获取文件的mimetype最快
 *
 * \return QMimeType 文件的mimetype
 */
QMimeType MimeDatabase::mimeTypeForFile(const QFileInfo &fileInfo, QMimeDatabase::MatchMode mode)
{
    return mimedb->mimeTypeForFile(fileInfo, mode);
}
/*!
 * \brief MimeDatabase::mimeTypesForFileName 获取一个文件的mimetype，调用qt的接口mimeTypesForFileName
 *
 * \param fileName 文件的名称
 *
 * \return QList<QMimeType> 文件的mimetype的列表
 */
QList<QMimeType> MimeDatabase::mimeTypesForFileName(const QString &fileName)
{
    return mimedb->mimeTypesForFileName(fileName);
}
/*!
 * \brief MimeDatabase::mimeTypeForData 根据data获取文件的mimetype
 *
 *  A valid MIME type is always returned. If data doesn't match any
 *
 *  known MIME type data, the default MIME type (application/octet-stream) is returned
 *
 * \param data QByteArray类型的数据
 *
 * \return QMimeType 文件的mimetype
 */
QMimeType MimeDatabase::mimeTypeForData(const QByteArray &data)
{
    return mimedb->mimeTypeForData(data);
}
/*!
 * \brief MimeDatabase::mimeTypeForData 根据data获取文件的mimetype
 *
 *  A valid MIME type is always returned. If data doesn't match any
 *
 *  known MIME type data, the default MIME type (application/octet-stream) is returned
 *
 * \param device QIODevice类型的数据，可以是QFile
 *
 * \return QMimeType 文件的mimetype
 */
QMimeType MimeDatabase::mimeTypeForData(QIODevice *device)
{
    return mimedb->mimeTypeForData(device);
}
/*!
 * \brief MimeDatabase::mimeTypeForUrl 根据文件的url获取文件的mimetype
 *
 * \param url 文件的url
 *
 * \return QMimeType 文件的mimetype
 */
QMimeType MimeDatabase::mimeTypeForUrl(const QUrl &url)
{
    if (dfmbase::FileUtils::isLocalFile(url))
        return mimedb->mimeTypeForUrl(url);
    else
        return mimedb->mimeTypeForFile(UrlRoute::urlToPath(url));
}
/*!
 * \brief MimeDatabase::mimeTypeForFileNameAndData Returns a MIME type for the given fileName and device data.
 *
 *  This overload can be useful when the file is remote, and we started to download some of its data in a device.
 *
 *  This allows to do full MIME type matching for remote files as well.
 *
 *  If the device is not open, it will be opened by this function, and closed after the MIME type detection is completed.
 *
 *  A valid MIME type is always returned. If device data doesn't match any known MIME type data, the default MIME
 *
 *  type (application/octet-stream) is returned.
 *
 *  This method looks at both the file name and the file contents, if necessary.
 *
 *  The file extension has priority over the contents, but the contents will be used if the file extension is unknown, or matches multiple MIME types.
 *
 * \param fileName 文件名称
 *
 * \param device 文件的QIODevice对象
 *
 * \return QMimeType 文件的mimetype
 */
QMimeType MimeDatabase::mimeTypeForFileNameAndData(const QString &fileName, QIODevice *device)
{
    return mimedb->mimeTypeForFileNameAndData(fileName, device);
}
/*!
 * \brief MimeDatabase::mimeTypeForFileNameAndData Returns a MIME type for the given fileName and device data.
 *
 *  This overload can be useful when the file is remote, and we started to download some of its data.
 *
 *  This allows to do full MIME type matching for remote files as well.
 *
 *  A valid MIME type is always returned. If data doesn't match any known MIME type data,
 *
 *  the default MIME type (application/octet-stream) is returned.
 *
 *  This method looks at both the file name and the file contents, if necessary.
 *
 *  The file extension has priority over the contents, but the contents will be used if the
 *
 *  file extension is unknown, or matches multiple MIME types.
 *
 * \param fileName 文件名称
 *
 * \param data 文件内容QByteArray的data
 *
 * \return QMimeType 文件的mimetype
 */
QMimeType MimeDatabase::mimeTypeForFileNameAndData(const QString &fileName, const QByteArray &data)
{
    return mimedb->mimeTypeForFileNameAndData(fileName, data);
}
/*!
 * \brief MimeDatabase::suffixForFileName Returns the suffix for the file fileName,
 *
 *  as known by the MIME database.
 *
 *  This allows to pre-select "tar.bz2" for foo.tar.bz2, but still only "txt" for my.file.with.dots.txt.
 *
 * \param fileName 文件的名称
 *
 * \return QString
 */
QString MimeDatabase::suffixForFileName(const QString &fileName)
{
    return mimedb->suffixForFileName(fileName);
}
/*!
 * \brief MimeDatabase::allMimeTypes Returns the list of all available MIME types.
 *
 *  This can be useful for showing all MIME types to the user, for instance
 *
 *  in a MIME type editor. Do not use unless really necessary in other cases though,
 *
 *  prefer using the mimeTypeForXxx() methods for performance reasons.
 *
 * \return QList<QMimeType>
 */
QList<QMimeType> MimeDatabase::allMimeTypes()
{
    return mimedb->allMimeTypes();
}
