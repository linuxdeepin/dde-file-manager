#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <QObject>
#include <QIcon>
#include <QFileInfo>

#include <QDir>
#include "desktopfile.h"
#include "properties.h"
#include "../models/durl.h"

/**
 * @class FileUtils
 * @brief Utility class providing static helper methods for file management
 */
class FileUtils {
public:

    static QString WallpaperKey;

    static bool removeRecurse(const QString &path, const QString &name);
    static void recurseFolder(const QString &path, const QString &parent,
                            QStringList *list);
    static qint64 filesCount(const QString& dir);
    static qint64 totalSize(const QString& dir);
    static qint64 totalSize(const DUrlList &files);
    static bool isArchive(const QString& path);
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
    static QString newDocmentName(const QString& targetdir, const QString& baseName, const QString& suffix);
    static void cpTemplateFileToTargetDir(const QString& targetdir, const QString& baseName, const QString& suffix);

    static bool openFile(const QString& filePath);
    static bool openDesktopFile(const QString& filePath);
    static bool openFileByApp(const QString& filePath, const QString& app);

    static bool setBackground(const QString& pictureFilePath);

    static QString getSoftLinkFileName(const QString& file, const QString& targetDir = QDir::homePath());
    static void createSoftLink(const QString& file, const QString& targetDir = QDir::homePath());
    static void createSoftLink(int windowId, const QString& file);
    static void sendToDesktop(const DUrlList& urls);
    static void sendToDesktop(const QString& file);

    static QString md5(const QString& data);
    static QByteArray md5(QFile *file, const QString &filePath);

    static bool isFileExecutable(const QString& path);
    static QString getFileMimetype(const QString& path);
    static bool isExecutableScript(const QString& path);
    static bool openExcutableFile(const QString& path, int flag);

    static QByteArray imageFormatName(QImage::Format f);
};

#endif // FILEUTILS_H
