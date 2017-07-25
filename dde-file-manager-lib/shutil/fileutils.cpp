#include "fileutils.h"

#include "views/windowmanager.h"

#include "app/define.h"
#include "singleton.h"
#include "mimetypedisplaymanager.h"
#include "dfmstandardpaths.h"
#include "dfileservices.h"
#include "simpleini/SimpleIni.h"
#include "dmimedatabase.h"
#include "mimesappsmanager.h"
#include "interfaces/dfmstandardpaths.h"

#include <QDirIterator>
#include <QUrl>
#include <QProcess>
#include <QGSettings>
#include <QFileDialog>
#include <QCryptographicHash>
#include <QDebug>
#include <QDesktopServices>
#include <QtMath>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

#include <sys/vfs.h>

#undef signals
extern "C" {
    #include <gio/gio.h>
    #include <gio/gdesktopappinfo.h>
}
#define signals public

DFM_USE_NAMESPACE

QString FileUtils::WallpaperKey = "pictureUri";
QString FileUtils::XDG_RUNTIME_DIR = "";

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

int FileUtils::filesCount(const QString &dir)
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
qint64 FileUtils::totalSize(const DUrlList &files) {
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

qint64 FileUtils::totalSize(const DUrlList &files, const qint64 &maxLimit, bool &isInLimit)
{
    qint64 total = 1;
    foreach (QUrl url, files) {
      QFileInfo file = url.path();
      if (file.isFile()) total += file.size();
      if(total > maxLimit){
          isInLimit = false;
          return total;
      }
      else {
        QDirIterator it(url.path(), QDir::AllEntries | QDir::System
                        | QDir::NoDotAndDotDot | QDir::NoSymLinks
                        | QDir::Hidden, QDirIterator::Subdirectories);
        while (it.hasNext()) {
          it.next();
          total += it.fileInfo().size();
          if(total > maxLimit){
              isInLimit = false;
              return total;
          }
        }
      }
    }
    return total;
}

bool FileUtils::isArchive(const QString &path)
{
    QFileInfo f(path);
    if (f.exists()){
        return mimeTypeDisplayManager->supportArchiveMimetypes().contains(DMimeDatabase().mimeTypeForFile(f).name());
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
    QString ret;
    const qint64 kb = 1024;
    const qint64 mb = 1024 * kb;
    const qint64 gb = 1024 * mb;
    const qint64 tb = 1024 * gb;

    if ( num >= tb ) {
        ret = QString( "%1 TB" ).arg( sizeString(QString::number( dRound64(qreal( num ) / tb), 'f', 1 )) );
    } else if( num >= gb ) {
        ret = QString( "%1 GB" ).arg( sizeString(QString::number( dRound64(qreal( num ) / gb), 'f', 1 )) );
    } else if( num >= mb ) {
        ret = QString( "%1 MB" ).arg( sizeString(QString::number( dRound64(qreal( num ) / mb), 'f', 1 )) );
    } else if( num >= kb ) {
        ret = QString( "%1 KB" ).arg( sizeString(QString::number( dRound64(qreal( num ) / kb),'f',1 )) );
    } else {
        ret = QString( "%1 B" ).arg( num );
    }

    return ret;
}

QString FileUtils::formatSizeToGB(qint64 num, bool isUnitVisible)
{
    QString ret;
    const qint64 kb = 1024;
    const qint64 mb = 1024 * kb;
    const qint64 gb = 1024 * mb;
    if (isUnitVisible){
        ret = QString( "%1G" ).arg( sizeString(QString::number(dRound64(qreal( num ) / gb), 'f', 0 )) );
    }else{
        ret = QString( "%1" ).arg( sizeString(QString::number(dRound64(qreal( num ) / gb), 'f', 0 )) );
    }

    return ret;
}

QString FileUtils::newDocmentName(QString targetdir, const QString &baseName, const QString &suffix)
{
    if (targetdir.isEmpty())
        return QString();

    if (targetdir.endsWith(QDir::separator()))
        targetdir.chop(1);

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

    return QString();
}

bool FileUtils::cpTemplateFileToTargetDir(const QString& targetdir, const QString& baseName, const QString& suffix)
{
    QString templateFile;
    QDirIterator it(DFMStandardPaths::standardLocation(DFMStandardPaths::TemplatesPath), QDir::Files);
    while (it.hasNext()) {
      it.next();
      if (it.fileInfo().suffix() == suffix){
          templateFile = it.filePath();
          break;
      }
    }

    if (templateFile.isEmpty())
        return false;

    QString targetFile = FileUtils::newDocmentName(targetdir, baseName, suffix);

    if (targetFile.isEmpty())
        return false;

    return QFile::copy(templateFile, targetFile);
}

bool FileUtils::openFile(const QString &filePath)
{
    if (QFileInfo(filePath).suffix() == "desktop"){
        return FileUtils::openDesktopFile(filePath);
    }
    qDebug() << mimeAppsManager->getDefaultAppByFileName(filePath);
    if (mimeAppsManager->getDefaultAppByFileName(filePath) == "org.gnome.font-viewer.desktop"){
        QProcess::startDetached("gvfs-open", QStringList() << filePath);
        QTimer::singleShot(200, [=]{
            QProcess::startDetached("gvfs-open", QStringList() << filePath);
        });
        return true;
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

bool FileUtils::openDesktopFileWithParams(const QString &filePath, const DUrlList& urlList)
{
    if (filePath.isEmpty()) {
        qDebug() << "Failed to open desktop file with gio: desktop file path is empty";
        return false;
    }

    std::string stdFilePath = filePath.toStdString();

    const char* cPath = stdFilePath.data();

    GDesktopAppInfo* appInfo = g_desktop_app_info_new_from_filename(cPath);
    if (!appInfo) {
        qDebug() << "Failed to open desktop file with gio: g_desktop_app_info_new_from_filename returns NULL. Check PATH maybe?";
        return false;
    }

    GList* g_files = NULL;
    GList* result_g_files = g_list_prepend(g_files, const_cast<char*>(cPath));
    Q_UNUSED(result_g_files)
    foreach (const DUrl& url, urlList) {
        GFile* f = g_file_new_for_uri(url.toString().toUtf8());
        g_files = g_list_append(g_files, f);
    }

    GError* gError = nullptr;

    const auto ok = g_app_info_launch(reinterpret_cast<GAppInfo*>(appInfo), g_files, NULL, &gError);

    if (gError) {
        qWarning() << "Error when trying to open desktop file with gio:" << gError->message;
        g_error_free(gError);
    }

    if (!ok) {
        qWarning() << "Failed to open desktop file with gio: g_app_info_launch returns false";
    }
    g_object_unref(appInfo);

    g_list_free(g_files);

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

    GDesktopAppInfo* appInfo = g_desktop_app_info_new_from_filename(app.toLocal8Bit().constData());
    if (!appInfo) {
        qDebug() << "Failed to open desktop file with gio: g_desktop_app_info_new_from_filename returns NULL. Check PATH maybe?";
        return false;
    }

    const auto stdFilePath = filePath.toStdString();
    const char* cFilePath = stdFilePath.c_str();

    GList files;
    GFile* file = g_file_new_for_commandline_arg((gchar *)cFilePath);
    files.data = file;
    files.prev = files.next = NULL;

    GError* gError = nullptr;
    const auto ok = g_app_info_launch(reinterpret_cast<GAppInfo*>(appInfo), &files, NULL, &gError);
    if (gError) {
        qWarning() << "Error when trying to open desktop file with gio:" << gError->message;
        g_error_free(gError);
    }

    if (!ok) {
        qWarning() << "Failed to open desktop file with gio: g_app_info_launch_uris returns false";
    }
    g_object_unref(appInfo);
    g_object_unref(file);
    return ok;
}

bool FileUtils::setBackground(const QString &pictureFilePath)
{
    QGSettings gsettings("com.deepin.wrap.gnome.desktop.background", "/com/deepin/wrap/gnome/desktop/background/");
    gsettings.set(WallpaperKey, pictureFilePath);

    return true;
}

QString FileUtils::md5(const QString &fpath)
{
    QFile file(fpath);

    if (file.open(QIODevice::ReadOnly)) {
        return QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5).toHex();
    }

    return QCryptographicHash::hash(fpath.toLocal8Bit(), QCryptographicHash::Md5).toHex();
}

QByteArray FileUtils::md5(QFile *file, const QString &filePath)
{
    QByteArray md5 = filePath.toLocal8Bit() + QByteArray::number(file->size());

    if (file->open(QIODevice::ReadOnly)) {
        if (file->size() < 8192) {
            md5 += file->readAll();
        } else {
            char data[4097] = {};

            file->read(data, 4096);

            md5.append(data);

            file->seek(file->size() - 4096);

            file->read(data, 4096);

            md5.append(data);
        }

        file->close();
    }

    return QCryptographicHash::hash(md5, QCryptographicHash::Md5).toHex();;
}

bool FileUtils::isFileExecutable(const QString &path)
{
    QFile file(path);
    return (file.permissions() & QFile::ReadUser) && (file.permissions() & QFile::ExeUser);
}

bool FileUtils::isFileRunnable(const QString &path)
{
    QString _path = path;
    QFileInfo info(path);
    QString mimetype = getFileMimetype(path);
    qDebug() << info.isSymLink() << mimetype;
    if (info.isSymLink()){
        _path = QFile(path).symLinkTarget();
        mimetype = getFileMimetype(path);
    }

    if (mimetype == "application/x-executable" || mimetype == "application/x-sharedlib") {
        return isFileExecutable(_path);
    }

    return false;
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
            (mimetype == "application/x-shellscript")) {
        return isFileExecutable(_path);
    }

    return false;
}

bool FileUtils::openExcutableScriptFile(const QString &path, int flag)
{
    bool result = false;
    switch (flag) {
    case 0:

        break;
    case 1:
        result = QProcess::startDetached(path, QStringList());
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

bool FileUtils::openExcutableFile(const QString &path, int flag)
{
    bool result = false;
    switch (flag) {
    case 0:
        break;
    case 1:{
        QStringList args;
        args << "-e" << path;
        result = QProcess::startDetached("x-terminal-emulator", args);
        break;
    }
    case 2:
        result = QProcess::startDetached(path,QStringList());
        break;
    default:
        break;
    }

    return result;
}

QByteArray FileUtils::imageFormatName(QImage::Format f)
{
    switch (f) {
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
    case QImage::Format_ARGB8565_Premultiplied:
    case QImage::Format_ARGB6666_Premultiplied:
    case QImage::Format_ARGB8555_Premultiplied:
    case QImage::Format_ARGB4444_Premultiplied:
    case QImage::Format_RGBA8888:
    case QImage::Format_RGBA8888_Premultiplied:
    case QImage::Format_A2BGR30_Premultiplied:
    case QImage::Format_A2RGB30_Premultiplied:
    case QImage::Format_Alpha8:
        return "png";
    default:
        break;
    }

    return "jpeg";
}


QString FileUtils::getFileContent(const QString &file)
{
    QFile f(file);
    QString fileContent = "";
    if (f.open(QFile::ReadOnly))
    {
        fileContent = QString(f.readAll());
        f.close();
    } else {
        qDebug () << "Could not read file " << file << ":" << f.errorString();
    }
    return fileContent;
}

bool FileUtils::writeTextFile(const QString &filePath, const QString &content)
{
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly|QIODevice::Text)) {
        QTextStream in(&file);
        in << content << endl;
        file.close();
        return true;
    } else {
        qDebug () << "Failed to write content to file " << filePath << ":" << file.errorString();
    }
    return false;
}

void FileUtils::migrateConfigFileFromCache(const QString& key)
{
    bool ret = false;
    QString oldPath = QString("%1/%2/%3.%4").arg(QDir().homePath(), ".cache/dde-file-manager",key,"json");
    QString newPath = QString("%1/%2.%3").arg(DFMStandardPaths::standardLocation(DFMStandardPaths::ApplicationConfigPath),key.toLower(), "json");
    QFile srcFile(oldPath);
    ret = srcFile.open(QIODevice::ReadOnly);
    if(ret){
        QByteArray data = srcFile.readAll();
        srcFile.close();

        QFile desFile(newPath);
        ret = desFile.open(QIODevice::WriteOnly);
        if(ret){
            int code = desFile.write(data);
            if(code < 0){
                qDebug () << "Error occurred when writing data";
                ret = false;
            } else {
                ret = srcFile.remove();
                if(!ret){
                    qDebug () << "Failed to remove source file " << oldPath;
                }
            }
            desFile.close();

        } else {
            qDebug () << "Failed to write data :" << desFile.errorString();
        }

    } else {
        qDebug () << "Could not read source file " << oldPath << ":" << srcFile.errorString();
    }

    if(!ret){
        qDebug () << "Failed to migrate config file from cache";
    }
}

void FileUtils::setDefaultFileManager()
{
    QString configPath = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).at(0);
    QString mimeAppsListPath = QString("%1/%2").arg(configPath, "mimeapps.list");
    QString mimeKey("inode/directory");
    QString appDesktopFile("dde-file-manager.desktop");
    const char defaultApplicationsSection[] = "Default Applications";

    CSimpleIniA settings;
    settings.SetUnicode(true);
    QString content(getFileContent(mimeAppsListPath));
    settings.LoadData(content.toStdString().c_str(), content.length());


    const char* mime_cstr = mimeKey.toStdString().c_str();
    if (QString(settings.GetValue(defaultApplicationsSection, mime_cstr)) == appDesktopFile){
        return;
    }else{
        settings.SetValue(defaultApplicationsSection, mime_cstr,
                          appDesktopFile.toStdString().c_str());

        std::string strData;
        settings.Save(strData);
        qDebug() << QString::fromStdString(strData);
        writeTextFile(mimeAppsListPath, QString::fromStdString(strData));
    }
}

DFMGlobal::MenuExtension FileUtils::getMenuExtension(const DUrlList &urlList)
{
    int fileCount = 0;
    int dirCount = 0;
    foreach (DUrl url, urlList) {
        QFileInfo info(url.toLocalFile());
        if (info.isDir()){
            dirCount += 1;
        }else if (info.isFile()){
            fileCount += 1;
        }
    }
    if (urlList.count() == 0){
            return DFMGlobal::MenuExtension::EmptyArea;
    }else if (fileCount == 1 && dirCount == 0 && fileCount == urlList.count()){
        return DFMGlobal::MenuExtension::SingleFile;
    }else if (fileCount > 1 && dirCount == 0 && fileCount == urlList.count()){
        return DFMGlobal::MenuExtension::MultiFiles;
    }else if (fileCount == 0 && dirCount == 1 && dirCount == urlList.count()){
        return DFMGlobal::MenuExtension::SingleDir;
    }else if (fileCount ==0 && dirCount > 1 && dirCount == urlList.count()){
        return DFMGlobal::MenuExtension::MultiDirs;
    }else if (urlList.count() > 1){
        return DFMGlobal::MenuExtension::MultiFileDirs;
    }else{
        return DFMGlobal::MenuExtension::UnknowMenuExtension;
    }
}

bool FileUtils::isGvfsMountFile(const QString &filePath)
{
    if (XDG_RUNTIME_DIR.isEmpty()){
        QStringList runtimes = QStandardPaths::standardLocations(QStandardPaths::RuntimeLocation);
        if (runtimes.count() >=1){
            XDG_RUNTIME_DIR = runtimes.at(0);
        }
    }

    QString gvfsDir = QString("%1/gvfs").arg(XDG_RUNTIME_DIR);
    if (filePath.startsWith(gvfsDir) && DUrl(filePath) != DUrl(gvfsDir)) {
        return true;
    }

    return false;
}

bool FileUtils::isFileExists(const QString &filePath)
{
    GFile *file;
    std::string fstdPath = filePath.toStdString();
    file = g_file_new_for_path(fstdPath.data());
    bool isExists = g_file_query_exists(file, NULL);
    g_object_unref(file);
    return isExists;
}

QString FileUtils::getMimeTypeByGIO(const QString &uri)
{
    GFile* file = NULL;
    GError* error = NULL;
    GFileInfo* fileInfo = NULL;
    QString mimeType;

    file = g_file_new_for_uri(uri.toLocal8Bit().constData());
    if(!file)
        return "";

    fileInfo = g_file_query_info(file,
                                 G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE,
                                 G_FILE_QUERY_INFO_NONE,
                                 NULL,
                                 &error);
    if(error || !fileInfo){
        qDebug () << "read file " << uri     << " mime type error:" << (error?error->message:"unknown error");
        g_error_free(error);
        g_object_unref(file);
        return "";
    }

    mimeType = g_file_info_get_content_type(fileInfo);
    g_object_unref(file);
    g_object_unref(fileInfo);
    return mimeType;
}

QJsonObject FileUtils::getJsonObjectFromFile(const QString &filePath)
{
    QJsonObject obj;
    QJsonDocument doc;
    if(!QFile::exists(filePath))
        return obj;

    QFile file(filePath);

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug () << "cannot read file " << filePath << ":" << file.errorString();
        file.close();
        return obj;
    }
    QByteArray data = file.readAll();
    file.close();

    QJsonParseError *jsError = NULL;
    doc = QJsonDocument::fromJson(data, jsError);

    if(jsError){
        qDebug () << "cache data pase error:" << jsError->errorString();
        return obj;
    }

    obj = doc.object();
    return obj;
}

QJsonArray FileUtils::getJsonArrayFromFile(const QString &filePath)
{
    QJsonArray array;
    QJsonDocument doc;
    if(!QFile::exists(filePath))
        return array;

    QFile file(filePath);

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug () << "cannot read file " << filePath << ":" << file.errorString();
        file.close();
        return array;
    }
    QByteArray data = file.readAll();
    file.close();

    QJsonParseError *jsError = NULL;
    doc = QJsonDocument::fromJson(data, jsError);

    if(jsError){
        qDebug () << "cache data pase error:" << jsError->errorString();
        return array;
    }

    array = doc.array();
    return array;
}

bool FileUtils::writeJsonObjectFile(const QString &filePath, const QJsonObject &obj)
{
    QJsonDocument doc;
    doc.setObject(obj);
    return writeTextFile(filePath, doc.toJson().data());
}

bool FileUtils::writeJsonnArrayFile(const QString &filePath, const QJsonArray &array)
{
    QJsonDocument doc;
    doc.setArray(array);
    return writeTextFile(filePath, doc.toJson().data());
}

void FileUtils::mountAVFS()
{
    QProcess p;
    p.start("/usr/bin/umountavfs");
    p.waitForFinished();
    QProcess::startDetached("/usr/bin/mountavfs");
}

void FileUtils::umountAVFS()
{
    QProcess::startDetached("/usr/bin/umountavfs");
}
