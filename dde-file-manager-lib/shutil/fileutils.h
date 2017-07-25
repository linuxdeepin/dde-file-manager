#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <QObject>
#include <QIcon>
#include <QFileInfo>

#include <QDir>
#include "desktopfile.h"
#include "properties.h"
#include "durl.h"
#include "dfmglobal.h"

/**
 * @class FileUtils
 * @brief Utility class providing static helper methods for file management
 */
class FileUtils {
public:

    static QString WallpaperKey;
    static QString XDG_RUNTIME_DIR;

    static bool removeRecurse(const QString &path, const QString &name);
    static void recurseFolder(const QString &path, const QString &parent,
                            QStringList *list);
    static int filesCount(const QString& dir);
    static qint64 totalSize(const QString& dir);
    static qint64 totalSize(const DUrlList &files);
    static qint64 totalSize(const DUrlList &files, const qint64& maxLimit, bool &isInLimit);
    static bool isArchive(const QString& path);
    static bool canFastReadArchive(const QString& path);
    static QStringList getApplicationNames();
    static QList<DesktopFile> getApplications();
    static QString getRealSuffix(const QString &name);
    static QIcon searchGenericIcon(const QString &category,
       const QIcon &defaultIcon = QIcon::fromTheme("unknown"));
    static QIcon searchMimeIcon(QString mime,
      const QIcon &defaultIcon = QIcon::fromTheme("unknown"));
    static QIcon searchAppIcon(const DesktopFile &app,
      const QIcon &defaultIcon = QIcon::fromTheme("application-x-executable"));
    static QString formatSize( qint64 num );
    static QString formatSizeToGB( qint64 num, bool isUnitVisible = false);
    static QString newDocmentName(QString targetdir, const QString& baseName, const QString& suffix);
    static bool cpTemplateFileToTargetDir(const QString& targetdir, const QString& baseName, const QString& suffix);

    static bool openFile(const QString& filePath);
    static bool openDesktopFile(const QString& filePath);
    static bool openDesktopFileWithParams(const QString& filePath, const DUrlList &urlList);
    static bool openFileByApp(const QString& filePath, const QString& app);

    static bool setBackground(const QString& pictureFilePath);

    static QString md5(const QString& data);
    static QByteArray md5(QFile *file, const QString &filePath);

    static bool isFileExecutable(const QString& path);
    static bool isFileRunnable(const QString& path);
    static QString getFileMimetype(const QString& path);
    static bool isExecutableScript(const QString& path);
    static bool openExcutableScriptFile(const QString& path, int flag);
    static bool openExcutableFile(const QString& path, int flag);

    static QByteArray imageFormatName(QImage::Format f);

    static QString getFileContent(const QString &file);
    static bool writeTextFile(const QString& filePath, const QString& content);
    static void migrateConfigFileFromCache(const QString& key);
    static void setDefaultFileManager();

    static DFMGlobal::MenuExtension getMenuExtension(const DUrlList& urlList);
    static bool isGvfsMountFile(const QString &filePath);
    static bool isFileExists(const QString& filePath);
    static QString getMimeTypeByGIO(const QString& uri);

    static QJsonObject getJsonObjectFromFile(const QString& filePath);
    static QJsonArray getJsonArrayFromFile(const QString& filePath);
    static bool writeJsonObjectFile(const QString& filePath, const QJsonObject& obj);
    static bool writeJsonnArrayFile(const QString& filePath, const QJsonArray& array);

    static void mountAVFS();
    static void umountAVFS();
};

#endif // FILEUTILS_H
