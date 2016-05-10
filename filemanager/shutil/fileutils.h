#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <QObject>
#include <QIcon>
#include <QFileInfo>
#include "desktopfile.h"
#include "properties.h"

class ProgressWatcher;

/**
 * @class FileUtils
 * @brief Utility class providing static helper methods for file management
 */
class FileUtils {
public:
  static bool removeRecurse(const QString &path, const QString &name);
  static void recurseFolder(const QString &path, const QString &parent,
                            QStringList *list);
  static qint64 totalSize(const QList<QUrl> &files);
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

  static bool openDesktopFile(const QString& filePath);
  static bool openFileByApp(const QString& filePath, const QString& app);
};

#endif // FILEUTILS_H
