/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "fileutils.h"

#include "views/windowmanager.h"

#include "app/define.h"
#include "singleton.h"
#include "mimetypedisplaymanager.h"
#include "dfmstandardpaths.h"
#include "dfileservices.h"
#include "dmimedatabase.h"
#include "mimesappsmanager.h"
#include "interfaces/dfmstandardpaths.h"
#include "controllers/appcontroller.h"
#include "dbusinterface/startmanager_interface.h"

#include <dstorageinfo.h>

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
#include <QSettings>
#include <QX11Info>
#include <dabstractfilewatcher.h>

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
    else if (!file.isSymLink()) {
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

/*!
 * \brief Display human readable file size.
 *
 * by default, will display size unit. like `1.2 GB` or `616 MB`. By using
 * \a forceUnit argument we can also force to display a unit size with the
 * required unit. 0 for bytes, 1 for KiB, 2 for MiB, 3 for GiB, etc.
 *
 * \param num The file size number.
 * \param withUnitVisible Size unit visible or not.
 * \param precision Precision for float number after the dot.
 * \param forceUnit Force to use a unit.
 * \return
 */
QString FileUtils::formatSize(qint64 num, bool withUnitVisible, int precision, int forceUnit, QStringList unitList)
{
    bool isForceUnit = (forceUnit >= 0);
    QStringList list;
    qreal fileSize(num);

    if (unitList.size() == 0) {
        list << " B" << " KB" << " MB" << " GB" << " TB"; // should we use KiB since we use 1024 here?
    } else {
        list = unitList;
    }

    QStringListIterator i(list);
    QString unit = i.hasNext() ? i.next() : QStringLiteral(" B");

    int index = 0;
    while(i.hasNext()) {
        if (fileSize < 1024 && !isForceUnit) {
            break;
        }

        if (isForceUnit && index == forceUnit) {
            break;
        }

        unit = i.next();
        fileSize /= 1024;
        index++;
    }
    QString unitString = withUnitVisible ? unit : QString();
    return QString("%1%2").arg(sizeString(QString::number(fileSize, 'f', precision)), unitString);
}

QString FileUtils::diskUsageString(qint64 usedSize, qint64 totalSize)
{
    const qint64 kb = 1024;
    const qint64 mb = 1024 * kb;
    const qint64 gb = 1024 * mb;
    int forceUnit = (totalSize < gb && totalSize > 0) ? 2 : 3;

    return QString("%1/%2").arg(FileUtils::formatSize(usedSize, false, 0, forceUnit), FileUtils::formatSize(totalSize, true, 0, forceUnit, {"B", "K", "M", "G"}));
}

DUrl FileUtils::newDocumentUrl(const DAbstractFileInfoPointer targetDirInfo, const QString &baseName, const QString &suffix)
{
    if (targetDirInfo->isVirtualEntry()) {
        return DUrl();
    }

    int i = 0;
    QString fileName = suffix.isEmpty() ? QString("%1").arg(baseName) : QString("%1.%2").arg(baseName, suffix);
    DUrl fileUrl = targetDirInfo->getUrlByChildFileName(fileName);
    while (true) {
        DAbstractFileInfoPointer newInfo = DFileService::instance()->createFileInfo(nullptr, fileUrl);
        if (newInfo && newInfo->exists()) {
            ++i;
            fileName = suffix.isEmpty()
                    ? QString("%1 %2").arg(baseName, QString::number(i))
                    : QString("%1 %2.%3").arg(baseName, QString::number(i), suffix);
            fileUrl = targetDirInfo->getUrlByChildFileName(fileName);
        } else {
            return fileUrl;
        }
    }

    return DUrl();
}

QString FileUtils::newDocmentName(QString targetdir, const QString &baseName, const QString &suffix)
{
    if (targetdir.isEmpty())
        return QString();

    if (targetdir.endsWith(QDir::separator()))
        targetdir.chop(1);

    int i = 0;
    QString filePath = suffix.isEmpty() ? QString("%1/%2").arg(targetdir, baseName) : QString("%1/%2.%3").arg(targetdir, baseName, suffix);
    while (true) {
        if (QFile(filePath).exists()) {
            ++i;
            filePath = suffix.isEmpty()
                       ? QString("%1/%2 %3").arg(targetdir, baseName, QString::number(i))
                       : QString("%1/%2 %3.%4").arg(targetdir, baseName, QString::number(i), suffix);
        } else {
            return filePath;
        }
    }

    return QString();
}

bool FileUtils::cpTemplateFileToTargetDir(const QString& targetdir, const QString& baseName, const QString& suffix, WId windowId)
{
    QString templateFile;
    QDirIterator it(DFMStandardPaths::location(DFMStandardPaths::TemplatesPath), QDir::Files);
    while (it.hasNext()) {
      it.next();
      if (it.fileInfo().suffix() == suffix){
          templateFile = it.filePath();
          break;
      }
    }

    if (templateFile.isEmpty())
        return false;

//    QString targetFile = FileUtils::newDocmentName(targetdir, baseName, suffix);

//    if (targetFile.isEmpty())
//        return false;

//    return QFile::copy(templateFile, targetFile);
    return !AppController::createFile(templateFile, targetdir, baseName, windowId).isEmpty();
}

bool FileUtils::openFile(const QString &filePath)
{
    bool result = false;
    if (QFileInfo(filePath).suffix() == "desktop"){
        result = FileUtils::launchApp(filePath);
        return result;
    }

    QString mimetype = getFileMimetype(filePath);
    QString defaultDesktopFile = MimesAppsManager::getDefaultAppDesktopFileByMimeType(mimetype);
    if (isFileManagerSelf(defaultDesktopFile) && mimetype != "inode/directory"){
        QStringList recommendApps = mimeAppsManager->getRecommendedApps(DUrl::fromLocalFile(filePath));
        recommendApps.removeOne(defaultDesktopFile);
        if (recommendApps.count() > 0){
            defaultDesktopFile = recommendApps.first();
        }else{
            qDebug() << "no default application for" << filePath;
            return false;
        }
    }

    result = launchApp(defaultDesktopFile, QStringList() << DUrl::fromLocalFile(filePath).toString());
    if (result){
        // workaround since DTK apps doesn't support the recent file spec.
        // spec: https://www.freedesktop.org/wiki/Specifications/desktop-bookmark-spec/
        // the correct approach: let the app add it to the recent list.
        // addToRecentFile(DUrl::fromLocalFile(filePath), mimetype);
        return result;
    }

    if (mimeAppsManager->getDefaultAppByFileName(filePath) == "org.gnome.font-viewer.desktop"){
        QProcess::startDetached("gio", QStringList() << "open" << filePath);
        QTimer::singleShot(200, [=]{
            QProcess::startDetached("gio", QStringList() << "open" << filePath);
        });
        return true;
    }

    result = QProcess::startDetached("gio", QStringList() << "open" << filePath);

    if (!result)
        return QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
    return result;
}

bool FileUtils::launchApp(const QString &desktopFile, const QStringList &filePaths)
{
    if (isFileManagerSelf(desktopFile) && filePaths.count() > 1){
        foreach(const QString& filePath, filePaths){
            openFile(DUrl(filePath).toLocalFile());
        }
        return true;
    }

    bool ok = launchAppByDBus(desktopFile, filePaths);
    if (!ok){
        ok = launchAppByGio(desktopFile, filePaths);
    }
    return ok;
}

bool FileUtils::launchAppByDBus(const QString &desktopFile, const QStringList &filePaths)
{
    if (appController->hasLaunchAppInterface()){
        qDebug() << "launchApp by dbus:" << desktopFile << filePaths;
        appController->startManagerInterface()->LaunchApp(desktopFile, QX11Info::getTimestamp(), filePaths);
        return true;
    }
    return false;
}

bool FileUtils::launchAppByGio(const QString &desktopFile, const QStringList &filePaths)
{
    qDebug() << "launchApp by gio:" << desktopFile << filePaths;

    std::string stdDesktopFilePath = desktopFile.toStdString();
    const char* cDesktopPath = stdDesktopFilePath.data();

    GDesktopAppInfo* appInfo = g_desktop_app_info_new_from_filename(cDesktopPath);
    if (!appInfo) {
        qDebug() << "Failed to open desktop file with gio: g_desktop_app_info_new_from_filename returns NULL. Check PATH maybe?";
        return false;
    }

    GList* g_files = nullptr;
    foreach (const QString& filePath, filePaths) {
        std::string stdFilePath = filePath.toStdString();
        const char* cFilePath = stdFilePath.data();
        GFile* f = g_file_new_for_uri(cFilePath);
        g_files = g_list_append(g_files, f);
    }

    GError* gError = nullptr;
    gboolean ok = g_app_info_launch(reinterpret_cast<GAppInfo*>(appInfo), g_files, nullptr, &gError);

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

bool FileUtils::openFilesByApp(const QString& desktopFile, const QStringList& filePaths)
{
    bool ok = false;

    if (desktopFile.isEmpty()) {
        qDebug() << "Failed to open desktop file with gio: app file path is empty";
        return ok;
    }

    if (filePaths.isEmpty()) {
        qDebug() << "Failed to open desktop file with gio: file path is empty";
        return ok;
    }

    qDebug() << desktopFile << filePaths;

    GDesktopAppInfo* appInfo = g_desktop_app_info_new_from_filename(desktopFile.toLocal8Bit().constData());
    if (!appInfo) {
        qDebug() << "Failed to open desktop file with gio: g_desktop_app_info_new_from_filename returns NULL. Check PATH maybe?";
        return false;
    }

    QString terminalFlag = QString(g_desktop_app_info_get_string(appInfo, "Terminal"));
    if (terminalFlag == "true"){
        QString exec = QString(g_desktop_app_info_get_string(appInfo, "Exec"));
        QStringList args;
        args << "-e" << exec.split(" ").at(0) << filePaths;
        QString termPath = defaultTerminalPath();
        qDebug() << termPath << args;
        ok = QProcess::startDetached(termPath, args);
    }else{
        ok = launchApp(desktopFile, filePaths);
    }
    g_object_unref(appInfo);

    return ok;
}

bool FileUtils::isFileManagerSelf(const QString &desktopFile)
{
    /*
     *  return true if exec field contains dde-file-manager/file-manager.sh of dde-file-manager desktopFile
    */
    DesktopFile d(desktopFile);
    return d.getExec().contains("dde-file-manager") || d.getExec().contains("file-manager.sh");
}

QString FileUtils::defaultTerminalPath()
{
    const static QString dde_daemon_default_term = QStringLiteral("/usr/lib/deepin-daemon/default-terminal");
    const static QString debian_x_term_emu = QStringLiteral("/usr/bin/x-terminal-emulator");

    if (QFileInfo::exists(dde_daemon_default_term)) {
        return dde_daemon_default_term;
    } else if (QFileInfo::exists(debian_x_term_emu)) {
        return debian_x_term_emu;
    }

    return QStandardPaths::findExecutable("xterm");
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

// sort of.. duplicate with FileUtils::isFileRunnable
bool FileUtils::shouldAskUserToAddExecutableFlag(const QString &path)
{
    QString _path = path;
    QFileInfo info(path);
    QString mimetype = getFileMimetype(path);
    if (info.isSymLink()){
        _path = QFile(path).symLinkTarget();
        mimetype = getFileMimetype(path);
    }

    if (mimetype == "application/x-executable"
        || mimetype == "application/x-sharedlib"
        || mimetype == "application/x-iso9660-appimage"
        || mimetype == "application/vnd.appimage") {
        return !isFileExecutable(_path);
    }

    return false;
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

    // blumia: about AppImage mime type, please refer to:
    //         https://cgit.freedesktop.org/xdg/shared-mime-info/tree/freedesktop.org.xml.in
    //         btw, consider using MimeTypeDisplayManager::ExecutableMimeTypes(private) ?
    if (mimetype == "application/x-executable"
        || mimetype == "application/x-sharedlib"
        || mimetype == "application/x-iso9660-appimage"
        || mimetype == "application/vnd.appimage") {
        return isFileExecutable(_path);
    }

    return false;
}


bool FileUtils::isFileWindowsUrlShortcut(const QString &path)
{
    QString mimetype = getFileMimetype(path);
    qDebug() << mimetype;
    if (mimetype == "application/x-mswinurl")
        return true;
    return false;
}

QString FileUtils::getInternetShortcutUrl(const QString &path)
{
    QSettings settings(path, QSettings::IniFormat);
    settings.beginGroup("InternetShortcut");
    QString url = settings.value("URL").toString();
    settings.endGroup();
    return url;
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

    if (info.size() == 0) {
        return false;
    }

    if (info.isSymLink()){
        _path = QFile(path).symLinkTarget();
        mimetype = getFileMimetype(path);
    }

    // blumia: it's not a good idea to check if it is a executable script by just checking
    //         mimetype.startsWith("text/"), should be fixed later.
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
        result = runCommand(path, QStringList(), QUrl(path).adjusted(QUrl::RemoveFilename).toString());
        break;
    case 2:{
        QStringList args;
        args << "-e" << path;
        result = runCommand(FileUtils::defaultTerminalPath(), args, QUrl(path).adjusted(QUrl::RemoveFilename).toString());
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

bool FileUtils::addExecutableFlagAndExecuse(const QString &path, int flag)
{
    bool result = false;
    QFile file(path);
    switch (flag) {
    case 0:
        break;
    case 1:
        file.setPermissions(file.permissions() | QFile::ExeOwner | QFile::ExeUser | QFile::ExeGroup | QFile::ExeOther);
        result = runCommand(path, QStringList());
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
        result = runCommand(FileUtils::defaultTerminalPath(), args, QUrl(path).adjusted(QUrl::RemoveFilename).toString());
        break;
    }
    case 2:
        result = runCommand(path, QStringList(), QUrl(path).adjusted(QUrl::RemoveFilename).toString());
        break;
    default:
        break;
    }

    return result;
}

bool FileUtils::runCommand(const QString &cmd, const QStringList &args, const QString& wd)
{
    bool result = false;
    if (appController->hasLaunchAppInterface()){
        qDebug() << "launch cmd by dbus:" << cmd << args;
        if(wd.length()) {
            QVariantMap opt = {{"dir", wd}};
            appController->startManagerInterface()->RunCommandWithOptions(cmd, args, opt);
        }
        else appController->startManagerInterface()->RunCommand(cmd, args);
        result = true;
    }else{
        qDebug() << "launch cmd by qt:" << cmd << args;
        result = QProcess::startDetached(cmd, args, wd);
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
    QString newPath = QString("%1/%2.%3").arg(DFMStandardPaths::location(DFMStandardPaths::ApplicationConfigPath),key.toLower(), "json");
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

QMap<QString, QString> FileUtils::getKernelParameters()
{
    QFile cmdline("/proc/cmdline");
    cmdline.open(QIODevice::ReadOnly);
    QByteArray content = cmdline.readAll();

    QByteArrayList paraList(content.split(' '));

    QMap<QString, QString> result;
    result.insert("_ori_proc_cmdline", content);

    for (const QByteArray& onePara : paraList) {
        int equalsIdx = onePara.indexOf('=');
        QString key = equalsIdx == -1 ? onePara.trimmed() : onePara.left(equalsIdx).trimmed();
        QString value = equalsIdx == -1 ? QString() : onePara.right(equalsIdx).trimmed();
        result.insert(key, value);
    }

    return result;
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
    if (filePath.isEmpty())
        return false;

    return !DStorageInfo::isLocalDevice(filePath);
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
