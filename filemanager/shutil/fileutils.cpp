#include "fileutils.h"
#include <QDirIterator>
#include <sys/vfs.h>
#include <QUrl>
#include <QMimeDatabase>
#include <QProcess>
#include <QGSettings>
#include <QFileDialog>
#include <QCryptographicHash>
#include <QDebug>
#include <QDesktopServices>
#include <QtMath>
#include "../views/windowmanager.h"
#include "standardpath.h"

#undef signals
extern "C" {
    #include <gio/gio.h>
    #include <gio/gdesktopappinfo.h>
}
#define signals public

QString FileUtils::WallpaperKey = "pictureUri";

/**
 * @brief Recursive removes file or directory
 * @param path path to file
 * @param name name of file
 * @return true if file/directory was successfully removed
 */
bool FileUtils::removeRecurse(const QString &path, const QString &name) {

  // File location
  QString url = path + QDir::separator() + name;

  // Check whether file or directory exists
  QFileInfo file(url);
  if (!file.exists()) {
    return false;
  }

  // List of files that will be deleted
  QStringList files;

  // If given file is a directory, collect all children of given directory
  if (file.isDir()) {
    QDirIterator it(url, QDir::AllEntries | QDir::System | QDir::NoDotAndDotDot
                    | QDir::Hidden, QDirIterator::Subdirectories);
    while (it.hasNext()) {
      files.prepend(it.next());
    }
  }

  // Append given file to the list of files and delete all
  files.append(url);
  foreach (QString file, files) {
    QFile(file).remove();
  }
  return true;
}
//---------------------------------------------------------------------------

/**
 * @brief Collects all file names in given path (recursive)
 * @param path path
 * @param parent parent path
 * @param list resulting list of files
 */
void FileUtils::recurseFolder(const QString &path, const QString &parent,
                              QStringList *list) {

  // Get all files in this path
  QDir dir(path);
  QStringList files = dir.entryList(QDir::AllEntries | QDir::Files
                                    | QDir::NoDotAndDotDot | QDir::Hidden);

  // Go through all files in current directory
  for (int i = 0; i < files.count(); i++) {

    // If current file is folder perform this method again. Otherwise add file
    // to list of results
    QString current = parent + QDir::separator() + files.at(i);
    if (QFileInfo(files.at(i)).isDir()) {
      recurseFolder(files.at(i), current, list);
    }
    else list->append(current);
  }
}

qint64 FileUtils::filesCount(const QString &dir)
{
    QDir d(dir);
    QStringList entryList = d.entryList(QDir::AllEntries | QDir::System
                | QDir::NoDotAndDotDot | QDir::Hidden);
    return entryList.size();
}

qint64 FileUtils::totalSize(const QString &targetFile)
{
    qint64 total = 0;
    QFileInfo targetInfo(targetFile);
    if (targetInfo.exists()){
        if (targetInfo.isDir()){
            QDir d(targetFile);
            QFileInfoList entryInfoList = d.entryInfoList(QDir::AllEntries | QDir::System
                        | QDir::NoDotAndDotDot | QDir::NoSymLinks
                        | QDir::Hidden);
            foreach (QFileInfo file, entryInfoList) {
                if (file.isFile()){
                    total += file.size();
                }
                else {
                    QDirIterator it(file.absoluteFilePath(), QDir::AllEntries | QDir::System
                                  | QDir::NoDotAndDotDot | QDir::NoSymLinks
                                  | QDir::Hidden, QDirIterator::Subdirectories);
                    while (it.hasNext()) {
                        it.next();
                        total += it.fileInfo().size();
                    }
                }
            }
        }else{
            total += targetInfo.size();
        }
    }
    return total;
}
//---------------------------------------------------------------------------

/**
 * @brief Returns size of all given files/dirs (including nested files/dirs)
 * @param files
 * @return total size
 */
qint64 FileUtils::totalSize(const QList<QUrl> &files) {
  qint64 total = 1;
  foreach (QUrl url, files) {
    QFileInfo file = url.path();
    if (file.isFile()) total += file.size();
    else {
      QDirIterator it(url.path(), QDir::AllEntries | QDir::System
                      | QDir::NoDotAndDotDot | QDir::NoSymLinks
                      | QDir::Hidden, QDirIterator::Subdirectories);
      while (it.hasNext()) {
        it.next();
        total += it.fileInfo().size();
      }
    }
  }
  return total;
}

bool FileUtils::isArchive(const QString &path)
{
    QFileInfo f(path);
    if (f.exists()){
        QStringList archiveMimes;
        archiveMimes << "application/x-7z-compressed" << "application/x-ace" << "application/x-alz"
                    << "application/x-ar" << "application/x-arj" << "application/x-stuffit"
                    << "application/x-bzip" << "application/x-bzip2" << "application/vnd.ms-cab-compressed"
                    << "application/x-cbr" << "application/x-cbz" << "application/x-cpio"
                    << "application/x-deb" << "application/x-ear" << "application/x-ms-dos-executable"
                    << "application/x-gzip" << "application/x-cd-image" << "application/x-java-archive"
                    << "application/x-lha" << "application/x-lha" << "application/x-lzip"
                    << "application/x-lzma" << "application/x-lzop" << "application/x-rar"
                    << "application/x-rpm" << "application/x-rzip" << "application/x-stuffit"
                    << "application/x-tar" << "application/x-bzip-compressed-tar"
                    << "application/x-compressed-tar" << "application/x-lzip-compressed-tar"
                    << "application/x-lzma-compressed-tar" << "application/x-lzop-compressed-tar"
                    << "application/x-7z-compressed-tar" << "application/x-xz-compressed-tar"
                    << "application/x-tarz" << "application/x-tarz" << "application/x-compressed-tar"
                    << "application/x-xz-compressed-tar" << "application/x-lzip-compressed-tar"
                    << "application/x-lzma-compressed-tar" << "application/x-lzop-compressed-tar"
                    << "application/x-war" << "application/x-xz" << "application/x-gzip"
                    << "application/x-compress" << "application/zip" << "application/x-zoo";
        return archiveMimes.contains(QMimeDatabase().mimeTypeForFile(f).name());
    }else{
        return false;
    }
}
//---------------------------------------------------------------------------

/**
 * @brief Returns names of available applications
 * @return application name list
 */
QStringList FileUtils::getApplicationNames() {
  QStringList appNames;
  QDirIterator it("/usr/share/applications", QStringList("*.desktop"),
                  QDir::Files | QDir::NoDotAndDotDot,
                  QDirIterator::Subdirectories);
  while (it.hasNext()) {
    it.next();
    appNames.append(it.fileName());
  }
  return appNames;
}
//---------------------------------------------------------------------------

/**
 * @brief Returns list of available applications
 * @return application list
 */
QList<DesktopFile> FileUtils::getApplications() {
  QList<DesktopFile> apps;
  QDirIterator it("/usr/share/applications", QStringList("*.desktop"),
                  QDir::Files | QDir::NoDotAndDotDot,
                  QDirIterator::Subdirectories);
  while (it.hasNext()) {
    it.next();
    apps.append(DesktopFile(it.filePath()));
  }
  return apps;
}
//---------------------------------------------------------------------------

/**
 * @brief Returns real suffix for given file
 * @param name
 * @return suffix
 */
QString FileUtils::getRealSuffix(const QString &name) {

  // Strip version suffix
  QStringList tmp = name.split(".");
  bool ok;
  while (tmp.size() > 1) {
    tmp.last().toInt(&ok);
    if (!ok) {
      return tmp.last();
    }
    tmp.removeLast();
  }
  return "";
}
//---------------------------------------------------------------------------

/**
 * @brief Returns mime icon
 * @param mime
 * @return icon
 */
QIcon FileUtils::searchMimeIcon(QString mime, const QIcon &defaultIcon) {
  QIcon icon = QIcon::fromTheme(mime.replace("/", "-"), defaultIcon);
  return icon;
}
//---------------------------------------------------------------------------

/**
 * @brief Searches for generic icon
 * @param category
 * @return icon
 */
QIcon FileUtils::searchGenericIcon(const QString &category,
                                   const QIcon &defaultIcon) {
  QIcon icon = QIcon::fromTheme(category + "-generic");
  if (!icon.isNull()) {
    return icon;
  }
  icon = QIcon::fromTheme(category + "-x-generic");
  return icon.isNull() ? defaultIcon : icon;
}
//---------------------------------------------------------------------------

/**
 * @brief Searches for application icon in the filesystem
 * @param app
 * @param defaultIcon
 * @return icon
 */
QIcon FileUtils::searchAppIcon(const DesktopFile &app,
                               const QIcon &defaultIcon) {

  // Resulting icon
  QIcon icon;

  // First attempt, check whether icon is a valid file
  if (QFile(app.getIcon()).exists()) {
    icon = QIcon(app.getIcon());
    if (!icon.isNull()) {
      return icon;
    }
  }

  // Second attempt, try load icon from theme
  icon = QIcon::fromTheme(app.getIcon());
  if (!icon.isNull()) {
    return icon;
  }

  // Next, try luck with application name
  QString name = app.getFileName().remove(".desktop").split("/").last();
  icon = QIcon::fromTheme(name);
  if (!icon.isNull()) {
    return icon;
  }

  // Last chance
  QDir appIcons("/usr/share/pixmaps","", 0, QDir::Files | QDir::NoDotAndDotDot);
  QStringList iconFiles = appIcons.entryList();
  QStringList searchIcons = iconFiles.filter(name);
  if (searchIcons.count() > 0) {
    return QIcon("/usr/share/pixmaps/" + searchIcons.at(0));
  }

  // Default icon
  return defaultIcon;
}
//---------------------------------------------------------------------------

QString sizeString(const QString &str)
{
    int begin_pos = str.indexOf('.');

    if (begin_pos < 0)
        return str;

    QString size = str;

    while (size.count() - 1 > begin_pos) {
        if (!size.endsWith('0'))
            return size;

        size = size.left(size.count() - 1);
    }

    return size.left(size.count() - 1);
}

qreal dRound64(qreal num, int count = 1)
{
    if (count <= 0)
        return qRound64(num);

    qreal base = qPow(10, count);

    return qRound64(num * base) / base;
}

QString FileUtils::formatSize( qint64 num )
{
    QString total;
    const qint64 kb = 1024;
    const qint64 mb = 1024 * kb;
    const qint64 gb = 1024 * mb;
    const qint64 tb = 1024 * gb;

    if ( num >= tb ) {
        total = QString( "%1 TB" ).arg( sizeString(QString::number( dRound64(qreal( num ) / tb), 'f', 1 )) );
    } else if( num >= gb ) {
        total = QString( "%1 GB" ).arg( sizeString(QString::number( dRound64(qreal( num ) / gb), 'f', 1 )) );
    } else if( num >= mb ) {
        total = QString( "%1 MB" ).arg( sizeString(QString::number( dRound64(qreal( num ) / mb), 'f', 1 )) );
    } else if( num >= kb ) {
        total = QString( "%1 KB" ).arg( sizeString(QString::number( dRound64(qreal( num ) / kb),'f',1 )) );
    } else {
        total = QString( "%1 B" ).arg( num );
    }

    return total;
}

QString FileUtils::newDocmentName(const QString &targetdir, const QString &baseName, const QString &suffix)
{
    int i = 0;
    QString filePath = QString("%1/%2.%4").arg(targetdir, baseName, suffix);
    while (true) {
        if (QFile(filePath).exists()){
            i++;
            filePath = QString("%1/%2 %3.%4").arg(targetdir, baseName, QString::number(i), suffix);
        }
        else{
            return filePath;
        }
    }
}

void FileUtils::cpTemplateFileToTargetDir(const QString& targetdir, const QString& baseName, const QString& suffix)
{
    QString templateFile;
    QDirIterator it("/usr/share/dde-file-manager/templates", QDir::Files);
    while (it.hasNext()) {
      it.next();
      if (it.filePath().endsWith(suffix)){
          templateFile = it.filePath();
          break;
      }
    }

    QString targetFile = FileUtils::newDocmentName(targetdir, baseName, suffix);
    QStringList args;
    args << templateFile;
    args << targetFile;
    QProcess::startDetached("cp", args);
}

bool FileUtils::openFile(const QString &filePath)
{
    if (QFileInfo(filePath).suffix() == "desktop"){
        return FileUtils::openDesktopFile(filePath);
    }

    bool result = QProcess::startDetached("gvfs-open", QStringList() << filePath);
    if (!result)
        return QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
    return result;
}

bool FileUtils::openDesktopFile(const QString &filePath)
{

    if (filePath.isEmpty()) {
        qDebug() << "Failed to open desktop file with gio: desktop file path is empty";
        return false;
    }

    const auto stdPath = filePath.toStdString();
    const char* cPath = stdPath.c_str();

    GDesktopAppInfo* appInfo = g_desktop_app_info_new_from_filename(cPath);
    if (!appInfo) {
        qDebug() << "Failed to open desktop file with gio: g_desktop_app_info_new_from_filename returns NULL. Check PATH maybe?";
        return false;
    }
    GError* gError = nullptr;
    const auto ok = g_app_info_launch_uris(reinterpret_cast<GAppInfo*>(appInfo), NULL, NULL, &gError);

    if (gError) {
        qWarning() << "Error when trying to open desktop file with gio:" << gError->message;
        g_error_free(gError);
    }

    if (!ok) {
        qWarning() << "Failed to open desktop file with gio: g_app_info_launch_uris returns false";
    }
    g_object_unref(appInfo);

    return ok;
}

bool FileUtils::openFileByApp(const QString &filePath, const QString &app)
{
    if (filePath.isEmpty()) {
        qDebug() << "Failed to open desktop file with gio: file path is empty";
        return false;
    }
    if (app.isEmpty()) {
        qDebug() << "Failed to open desktop file with gio: app file path is empty";
        return false;
    }

    qDebug() << filePath << app;

    const auto stdAppPath = app.toStdString();
    const char* cAppPath = stdAppPath.c_str();

    GDesktopAppInfo* appInfo = g_desktop_app_info_new_from_filename(cAppPath);
    if (!appInfo) {
        qDebug() << "Failed to open desktop file with gio: g_desktop_app_info_new_from_filename returns NULL. Check PATH maybe?";
        return false;
    }

    const auto isSupportLaunchUris = g_app_info_supports_uris(reinterpret_cast<GAppInfo*>(appInfo));

    if (!isSupportLaunchUris){
        qWarning() << "Failed to open desktop file with gio: g_app_info_supports_uris returns false";
        openFile(filePath);
        return false;
    }

    const auto stdFilePath = filePath.toStdString();
    const char* cFilePath = stdFilePath.c_str();

    GList uris;
    uris.data = (gchar *)cFilePath;
    uris.prev = uris.next = NULL;


    GError* gError = nullptr;
    const auto ok = g_app_info_launch_uris(reinterpret_cast<GAppInfo*>(appInfo), &uris, NULL, &gError);
    if (gError) {
        qWarning() << "Error when trying to open desktop file with gio:" << gError->message;
        g_error_free(gError);
    }

    if (!ok) {
        qWarning() << "Failed to open desktop file with gio: g_app_info_launch_uris returns false";
    }
    g_object_unref(appInfo);

    return ok;
}

bool FileUtils::setBackground(const QString &pictureFilePath)
{
    QGSettings* gsettings = new QGSettings("com.deepin.wrap.gnome.desktop.background",
                                             "/com/deepin/wrap/gnome/desktop/background/");
    gsettings->set(WallpaperKey, pictureFilePath);
    gsettings->deleteLater();
}


QString FileUtils::getSoftLinkFileName(const QString &file, const QString &targetDir)
{
    QFileInfo  info(file);

    if (info.exists()){
        QString baseName = info.baseName();
        QString shortcut = QObject::tr("Shortcut");
        QString linkBaseName;
        int number = 1;
        while (true) {
            if (info.isFile()){
                if (number == 1){
                    linkBaseName = QString("%1 %2.%3").arg(baseName, shortcut, info.suffix());
                }else{
                    linkBaseName = QString("%1 %2%3.%4").arg(baseName, shortcut, QString::number(number), info.suffix());
                }
            }else if (info.isDir()){
                if (number == 1){
                    linkBaseName = QString("%1 %2").arg(baseName, shortcut);
                }else{
                    linkBaseName = QString("%1 %2%3").arg(baseName, shortcut, QString::number(number));
                }
            }else if (info.isSymLink()){
                return QString();
            }
            QString linkFile = QString("%1/%2").arg(targetDir, linkBaseName);
            if (QFile(linkFile).exists()){
                number += 1;
                continue;
            }else{
                return linkFile;
            }
        }
    }else{
        return QString();
    }
}

void FileUtils::createSoftLink(const QString &file, const QString &targetDir)
{
    QString linkFile = getSoftLinkFileName(file, targetDir);
    if (!linkFile.isEmpty()){
        QFile f(file);
        bool successed = f.link(linkFile);
        if (!successed){
            qDebug() << "create link file" << linkFile << f.errorString();
        }
    }

}

void FileUtils::createSoftLink(int windowId, const QString &file)
{
    QString linkFile = getSoftLinkFileName(file, QFileInfo(file).dir().absolutePath());
    if (!linkFile.isEmpty()){
        QString fileName = QFileDialog::getSaveFileName(WindowManager::getWindowById(windowId), QObject::tr("Create symlink"),
                               linkFile);
        QFile f(file);
        bool successed = f.link(fileName);
        if (!successed){
            qDebug() << "create link file" << fileName << f.errorString();
        }
    }
}

void FileUtils::sendToDesktop(const DUrlList &urls)
{
    foreach (DUrl url, urls) {
        sendToDesktop(url.toLocalFile());
    }
}

void FileUtils::sendToDesktop(const QString &file)
{
    createSoftLink(file, StandardPath::getDesktopPath());
}

QString FileUtils::md5(const QString &fpath)
{
    QFile file(fpath);

    if (file.open(QIODevice::ReadOnly)) {
        return QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5).toHex();
    }

    return QCryptographicHash::hash(fpath.toLocal8Bit(), QCryptographicHash::Md5).toHex();
}

bool FileUtils::isFileExecutable(const QString &path)
{
    QFile file(path);
    return (file.permissions() & QFile::ReadUser) && (file.permissions() & QFile::ExeUser);
}

QString FileUtils::getFileMimetype(const QString &path)
{
    GFile *file;
    GFileInfo *info;
    QString result;

    file = g_file_new_for_path(path.toUtf8());
    info = g_file_query_info(file, "standard::content-type", G_FILE_QUERY_INFO_NONE, NULL, NULL);
    result = g_file_info_get_content_type(info);

    g_object_unref(file);

    return result;
}

bool FileUtils::isExecutableScript(const QString &path)
{
    QString _path = path;
    QFileInfo info(path);
    QString mimetype = getFileMimetype(path);
    qDebug() << info.isSymLink() << mimetype;
    if (info.isSymLink()){
        _path = QFile(path).symLinkTarget();
        mimetype = getFileMimetype(path);
    }

    if (mimetype.startsWith("text/") ||
            (mimetype == "application/x-shellscript") ||
            (mimetype == "application/x-executable")) {
        return isFileExecutable(_path);
    }

    return false;
}

bool FileUtils::openExcutableFile(const QString &path, int flag)
{
    bool result = false;
    switch (flag) {
    case 0:

        break;
    case 1:
        result = QProcess::startDetached(path);
        break;
    case 2:{
        QStringList args;
        args << "-e" << path;
        result = QProcess::startDetached("x-terminal-emulator", args);
        qDebug() << result;
        break;
    }
    case 3:
        result = openFile(path);
        break;
    default:
        break;
    }

    return result;
}
