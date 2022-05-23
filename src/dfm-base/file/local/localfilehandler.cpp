/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "localfilehandler.h"

#include "dfm-base/interfaces/abstractfileinfo.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/mimetype/mimedatabase.h"
#include "dfm-base/mimetype/mimesappsmanager.h"
#include "dfm-base/mimetype/mimetypedisplaymanager.h"
#include "dfm-base/utils/desktopfile.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/utils/decorator/decoratorfile.h"
#include "dfm-base/utils/decorator/decoratorfileinfo.h"
#include "dfm-base/utils/decorator/decoratorfileoperator.h"
#include "dfm-base/utils/decorator/decoratorfileenumerator.h"
#include "utils/universalutils.h"
#include "dfm_event_defines.h"

#include <dfm-io/dfmio_register.h>
#include <dfm-io/core/doperator.h>
#include <dfm-io/core/diofactory.h>
#include <dfm-io/core/dfile.h>

#include <DRecentManager>

#include <QString>
#include <QUrl>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QDebug>
#include <QRegularExpression>
#include <QProcess>
#include <QDesktopServices>
#include <QX11Info>

#include <unistd.h>
#include <utime.h>
#include <cstdio>

#undef signals
extern "C" {
#include <gio/gio.h>
#include <gio/gappinfo.h>
#include <gio-unix-2.0/gio/gdesktopappinfo.h>
}
#define signals public

DFMBASE_USE_NAMESPACE

LocalFileHandler::LocalFileHandler()
{
    // TODO(lanxs)
    // 注册暂时放在这里
    dfmio::dfmio_init();
}

LocalFileHandler::~LocalFileHandler()
{
}
/*!
 * \brief LocalFileHandler::touchFile 创建文件，创建文件时会使用Truncate标志，清理掉文件的内容
 * 如果文件存在就会造成文件内容丢失
 * \param url 新文件的url
 * \return bool 创建文件是否成功
 */
bool LocalFileHandler::touchFile(const QUrl &url)
{
    DecoratorFileOperator doperator(url);

    QSharedPointer<DFMIO::DOperator> oper = doperator.operatorPtr();
    if (!oper) {
        qWarning() << "create operator failed, url: " << url;
        return false;
    }

    bool success = oper->touchFile();
    if (!success) {
        qWarning() << "touch file failed, url: " << url;

        setError(oper->lastError());

        return false;
    }

    DecoratorFileInfo targetFileInfo(url);
    const QString &suffix = targetFileInfo.suffix();

    QString templateFile;
    DecoratorFileEnumerator enumerator(StandardPaths::location(StandardPaths::kTemplatesPath), {}, static_cast<DFMIO::DEnumerator::DirFilter>(static_cast<int32_t>(QDir::Files)));
    while (enumerator.hasNext()) {
        if (enumerator.fileInfo()->attribute(DFMIO::DFileInfo::AttributeID::kStandardSuffix) == suffix) {
            templateFile = enumerator.next();
            break;
        }
    }

    if (!templateFile.isEmpty()) {
        QByteArray arr = DecoratorFile(templateFile).readAll();
        if (!arr.isEmpty()) {
            qint64 writeCount = DecoratorFile(url).writeAll(arr);
            if (writeCount <= 0)
                qWarning() << "file touch succ, but write template failed";
        }
    }

    AbstractFileInfoPointer fileInfo = InfoFactory::create<AbstractFileInfo>(url);
    fileInfo->refresh();

    FileUtils::notifyFileChangeManual(DFMBASE_NAMESPACE::FileNotifyType::kFileAdded, url);

    return true;
}
/*!
 * \brief LocalFileHandler::mkdir 创建目录
 * \param dir 创建目录的url
 * \return bool 创建文件是否成功
 */
bool LocalFileHandler::mkdir(const QUrl &dir)
{
    QSharedPointer<DFMIO::DIOFactory> factory = produceQSharedIOFactory(dir.scheme(), static_cast<QUrl>(dir));
    if (!factory) {
        qWarning() << "create factory failed, url: " << dir;
        return false;
    }

    QSharedPointer<DFMIO::DOperator> oper = factory->createOperator();
    if (!oper) {
        qWarning() << "create operator failed, url: " << dir;
        return false;
    }

    bool success = oper->makeDirectory();
    if (!success) {
        qWarning() << "make directory failed, url: " << dir;

        setError(oper->lastError());

        return false;
    }

    AbstractFileInfoPointer fileInfo = InfoFactory::create<AbstractFileInfo>(dir);
    fileInfo->refresh();

    FileUtils::notifyFileChangeManual(DFMBASE_NAMESPACE::FileNotifyType::kFileAdded, dir);

    return true;
}
/*!
 * \brief LocalFileHandler::rmdir 删除目录
 * \param url 删除目录的url
 * \return bool 是否删除目录成功
 */
bool LocalFileHandler::rmdir(const QUrl &url)
{
    QSharedPointer<DFMIO::DIOFactory> factory = produceQSharedIOFactory(url.scheme(), static_cast<QUrl>(url));
    if (!factory) {
        qWarning() << "create factory failed, url: " << url;
        return false;
    }

    QSharedPointer<DFMIO::DOperator> oper = factory->createOperator();
    if (!oper) {
        qWarning() << "create operator failed, url: " << url;
        return false;
    }

    bool success = oper->trashFile();
    if (!success) {
        qWarning() << "trash file failed, url: " << url;

        setError(oper->lastError());

        return false;
    }

    FileUtils::notifyFileChangeManual(DFMBASE_NAMESPACE::FileNotifyType::kFileDeleted, url);

    return true;
}
/*!
 * \brief LocalFileHandler::rename 重命名文件
 * \param url 源文件的url
 * \param newUrl 新文件的url
 * \return bool 重命名文件是否成功
 */
bool LocalFileHandler::renameFile(const QUrl &url, const QUrl &newUrl)
{
    if (!url.isLocalFile() || !newUrl.isLocalFile())
        return false;

    if (url.scheme() != newUrl.scheme())
        return false;

    const QByteArray &sourceFile = url.toLocalFile().toLocal8Bit();
    const QByteArray &targetFile = newUrl.toLocalFile().toLocal8Bit();

    if (::rename(sourceFile.constData(), targetFile.constData()) == 0) {
        FileUtils::notifyFileChangeManual(DFMBASE_NAMESPACE::FileNotifyType::kFileDeleted, url);
        FileUtils::notifyFileChangeManual(DFMBASE_NAMESPACE::FileNotifyType::kFileAdded, newUrl);

        AbstractFileInfoPointer fileInfo = InfoFactory::create<AbstractFileInfo>(newUrl);
        fileInfo->refresh();
        return true;
    }

    QSharedPointer<DFMIO::DIOFactory> factory = produceQSharedIOFactory(url.scheme(), static_cast<QUrl>(url));
    if (!factory) {
        qWarning() << "create factory failed, url: " << url;
        return false;
    }
    QSharedPointer<DFMIO::DOperator> oper = factory->createOperator();
    if (!oper) {
        qWarning() << "create operator failed, url: " << url;
        return false;
    }

    bool success = oper->renameFile(newUrl);
    if (!success) {
        qWarning() << "rename file failed, url: " << url;

        setError(oper->lastError());

        return false;
    }

    AbstractFileInfoPointer fileInfo = InfoFactory::create<AbstractFileInfo>(newUrl);
    fileInfo->refresh();

    FileUtils::notifyFileChangeManual(DFMBASE_NAMESPACE::FileNotifyType::kFileDeleted, url);
    FileUtils::notifyFileChangeManual(DFMBASE_NAMESPACE::FileNotifyType::kFileAdded, newUrl);

    return true;
}

bool LocalFileHandler::renameFileBatchReplace(const QList<QUrl> &urls, const QPair<QString, QString> &pair, QMap<QUrl, QUrl> &successUrls)
{
    QMap<QUrl, QUrl> needDealUrls = FileUtils::fileBatchReplaceText(urls, pair);
    return renameFilesBatch(needDealUrls, successUrls);
}

bool LocalFileHandler::renameFileBatchAppend(const QList<QUrl> &urls, const QPair<QString, AbstractJobHandler::FileNameAddFlag> &pair, QMap<QUrl, QUrl> &successUrls)
{
    QMap<QUrl, QUrl> needDealUrls = FileUtils::fileBatchAddText(urls, pair);
    return renameFilesBatch(needDealUrls, successUrls);
}

bool LocalFileHandler::renameFileBatchCustom(const QList<QUrl> &urls, const QPair<QString, QString> &pair, QMap<QUrl, QUrl> &successUrls)
{
    QMap<QUrl, QUrl> needDealUrls = FileUtils::fileBatchCustomText(urls, pair);
    return renameFilesBatch(needDealUrls, successUrls);
}
/*!
 * \brief LocalFileHandler::openFile 打开文件
 * \param file 打开文件的url
 * \return bool 打开文件是否成功
 */
bool LocalFileHandler::openFile(const QUrl &url)
{
    return openFiles({ url });
}
/*!
 * \brief LocalFileHandler::openFiles 打开多个文件
 * \param files 打开文件的url列表
 * \return bool 打开文件是否成功
 */
bool LocalFileHandler::openFiles(const QList<QUrl> &urls)
{
    QList<QUrl> resourceUrls = urls;

    bool ret = false;
    for (const QUrl &url : urls) {
        AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url);
        if (!info)
            continue;
        if (info->suffix() == Global::kDesktop) {
            ret = launchApp(url.path()) || ret;   //有一个成功就成功
            resourceUrls.removeOne(url);
            continue;
        }
    }
    if (resourceUrls.isEmpty())
        return ret;

    const QUrl firstUrl = resourceUrls.first();

    AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(firstUrl);
    QString mimetype;
    if (info && info->size() == 0 && info->exists()) {
        mimetype = info->fileMimeType().name();
    } else {
        mimetype = getFileMimetypeFromGio(firstUrl);
    }

    bool isOpenNow = false;
    QString defaultDesktopFile = MimesAppsManager::getDefaultAppDesktopFileByMimeType(mimetype);
    if (defaultDesktopFile.isEmpty()) {
        if (isSmbUnmountedFile(firstUrl)) {
            mimetype = QString("inode/directory");
            defaultDesktopFile = MimesAppsManager::getDefaultAppDesktopFileByMimeType(mimetype);
            isOpenNow = true;
            mimetype = QString();
        } else {
            qDebug() << "no default application for" << firstUrl;
            return false;
        }
    }

    if (!isOpenNow && isFileManagerSelf(defaultDesktopFile) && mimetype != "inode/directory") {
        QStringList recommendApps = MimesAppsManager::getRecommendedApps(firstUrl);
        recommendApps.removeOne(defaultDesktopFile);
        if (recommendApps.count() > 0) {
            defaultDesktopFile = recommendApps.first();
        } else {
            qDebug() << "no default application for" << firstUrl;
            return false;
        }
    }

    QStringList appAgrs;
    for (const QUrl &tmp : resourceUrls)
        appAgrs << tmp.toString();
    bool result = launchApp(defaultDesktopFile, appAgrs);
    if (result) {
        // workaround since DTK apps doesn't support the recent file spec.
        // spec: https://www.freedesktop.org/wiki/Specifications/desktop-bookmark-spec/
        // the correct approach: let the app add it to the recent list.
        // addToRecentFile(DUrl::fromLocalFile(filePath), mimetype);
        for (const QUrl &tmp : resourceUrls) {
            QString file_path = tmp.path();
            DesktopFile df(defaultDesktopFile);
            addRecentFile(file_path, df, mimetype);
        }
        return result;
    } else if (isSmbUnmountedFile(firstUrl)) {
        return false;
    }

    if (MimesAppsManager::getDefaultAppByFileName(firstUrl.path()) == "org.gnome.font-viewer.desktop") {
        QProcess::startDetached("gio", QStringList() << "open" << firstUrl.path());
        QTimer::singleShot(200, [=] {
            QProcess::startDetached("gio", QStringList() << "open" << firstUrl.path());
        });
        return true;
    }

    result = QProcess::startDetached("gio", QStringList() << "open" << firstUrl.path());

    if (!result) {
        result = false;
        for (const QUrl &tmp : resourceUrls)
            result = QDesktopServices::openUrl(tmp) || result;   //有一个成功就成功
    }
    return result;
}
/*!
 * \brief LocalFileHandler::openFileByApp 指定的app打开文件
 * \param file 文件的url
 * \param appDesktop app的desktop路径
 * \return bool 是否打开成功
 */
bool LocalFileHandler::openFileByApp(const QUrl &file, const QString &desktopFile)
{
    return openFilesByApp({ file }, desktopFile);
}
/*!
 * \brief LocalFileHandler::openFilesByApp 指定的app打开多个文件
 * \param files 文件的url列表
 * \param appDesktop app的desktop路径
 * \return bool 是否打开成功
 */
bool LocalFileHandler::openFilesByApp(const QList<QUrl> &filePaths, const QString &desktopFile)
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

    GDesktopAppInfo *appInfo = g_desktop_app_info_new_from_filename(desktopFile.toLocal8Bit().constData());
    if (!appInfo) {
        qDebug() << "Failed to open desktop file with gio: g_desktop_app_info_new_from_filename returns NULL. Check PATH maybe?";
        return false;
    }

    QStringList filePathsStr;
    for (const auto &url : filePaths) {
        filePathsStr << url.toString();
    }

    QString terminalFlag = QString(g_desktop_app_info_get_string(appInfo, "Terminal"));
    if (terminalFlag == "true") {
        QString exec = QString(g_desktop_app_info_get_string(appInfo, "Exec"));
        QStringList args;
        args << "-e" << exec.split(" ").at(0) << filePathsStr;
        QString termPath = defaultTerminalPath();
        qDebug() << termPath << args;
        ok = QProcess::startDetached(termPath, args);
    } else {
        ok = launchApp(desktopFile, filePathsStr);
    }
    g_object_unref(appInfo);

    if (ok) {
        // workaround since DTK apps doesn't support the recent file spec.
        // spec: https://www.freedesktop.org/wiki/Specifications/desktop-bookmark-spec/
        // the correct approach: let the app add it to the recent list.
        // addToRecentFile(DUrl::fromLocalFile(filePath), mimetype);
        QString filePath = filePaths.first().toString();
        filePath = QUrl::fromUserInput(filePath).path();
        QString mimetype = getFileMimetype(filePath);
        for (const QUrl &tmp : filePaths) {
            QString temFilePath = tmp.path();
            DesktopFile df(desktopFile);
            addRecentFile(temFilePath, df, mimetype);
        }
    }

    return ok;
}

/*!
 * \brief LocalFileHandler::createSystemLink 创建文件的连接文件使用系统c库
 * \param sourcfile 源文件的url
 * \param link 链接文件的url
 * \return bool 创建链接文件是否成功
 */
bool LocalFileHandler::createSystemLink(const QUrl &sourcefile, const QUrl &link)
{
    QSharedPointer<DFMIO::DIOFactory> factory = produceQSharedIOFactory(sourcefile.scheme(), static_cast<QUrl>(sourcefile));
    if (!factory) {
        qWarning() << "create factory failed, url: " << sourcefile;
        return false;
    }

    QSharedPointer<DFMIO::DOperator> oper = factory->createOperator();
    if (!oper) {
        qWarning() << "create operator failed, url: " << sourcefile;
        return false;
    }

    bool success = oper->createLink(link);
    if (!success) {
        qWarning() << "create link failed, url: " << sourcefile << " link url: " << link;

        setError(oper->lastError());

        return false;
    }

    FileUtils::notifyFileChangeManual(DFMBASE_NAMESPACE::FileNotifyType::kFileAdded, link);

    return true;
}
/*!
 * \brief LocalFileHandler::setPermissions 设置文件的权限
 * \param url 需要设置文件的url
 * \param permissions 文件的权限
 * \return bool 设置文件的权限是否成功
 */
bool LocalFileHandler::setPermissions(const QUrl &url, QFileDevice::Permissions permissions)
{
    QSharedPointer<DFMIO::DIOFactory> factory = produceQSharedIOFactory(url.scheme(), static_cast<QUrl>(url));
    if (!factory) {
        qWarning() << "create factory failed, url: " << url;
        return false;
    }
    QSharedPointer<DFMIO::DFile> dfile = factory->createFile();
    if (!dfile) {
        qWarning() << "create file failed, url: " << url;
        return false;
    }

    bool success = dfile->setPermissions(DFMIO::DFile::Permissions(uint16_t(permissions)));
    if (!success) {
        qWarning() << "set permissions failed, url: " << url;

        setError(dfile->lastError());

        return false;
    }

    return true;
}
/*!
 * \brief LocalFileHandler::deleteFile 删除文件使用系统c库
 * \param file 文件的url
 * \return bool 删除文件是否失败
 */
bool LocalFileHandler::deleteFile(const QUrl &file)
{
    int ret = 0;
    if ((ret = ::remove(file.toLocalFile().toLocal8Bit())) == 0) {
        FileUtils::notifyFileChangeManual(DFMBASE_NAMESPACE::FileNotifyType::kFileDeleted, file);
        qDebug() << "delete file success: " << file;
        return true;
    }
    qDebug() << "try delete file, but failed url: " << file << " ret: " << ret;

    setError(DFMIOError(DFM_IO_ERROR_NOT_SUPPORTED));

    return false;
}
/*!
 * \brief LocalFileHandler::setFileTime 设置文件的读取和最后修改时间
 * \param url 文件的url
 * \param accessDateTime 文件的读取时间
 * \param lastModifiedTime 文件最后修改时间
 * \return bool 设置是否成功
 */
bool LocalFileHandler::setFileTime(const QUrl &url, const QDateTime &accessDateTime,
                                   const QDateTime &lastModifiedTime)
{
    utimbuf buf = { accessDateTime.toTime_t(), lastModifiedTime.toTime_t() };

    if (::utime(url.toLocalFile().toLocal8Bit(), &buf) == 0) {
        return true;
    }

    setError(DFMIOError(DFM_IO_ERROR_NOT_SUPPORTED));

    return false;
}

bool LocalFileHandler::launchApp(const QString &desktopFilePath, const QStringList &fileUrls)
{
    QStringList newFileUrls(fileUrls);

    if (isFileManagerSelf(desktopFilePath) && fileUrls.count() > 1) {
        for (const QString &url : fileUrls) {
            openFile(url);
        }
        return true;
    }

    if (isFileManagerSelf(desktopFilePath) && fileUrls.count() == 1) {
        QUrl fileUrl(fileUrls[0]);
        if (isSmbUnmountedFile(fileUrl)) {
            newFileUrls.clear();
            newFileUrls << smbFileUrl(fileUrls[0]).toLocalFile();
        }
    }

    bool ok = launchAppByDBus(desktopFilePath, newFileUrls);
    if (!ok) {
        ok = launchAppByGio(desktopFilePath, newFileUrls);
    }
    return ok;
}

bool LocalFileHandler::launchAppByDBus(const QString &desktopFile, const QStringList &filePaths)
{
    if (UniversalUtils::checkLaunchAppInterface())
        return UniversalUtils::launchAppByDBus(desktopFile, filePaths);
    return false;
}

bool LocalFileHandler::launchAppByGio(const QString &desktopFilePath, const QStringList &fileUrls)
{
    qDebug() << "launchApp by gio:" << desktopFilePath << fileUrls;

    const char *cDesktopFilePath = desktopFilePath.toLocal8Bit().data();

    g_autoptr(GDesktopAppInfo) appInfo = g_desktop_app_info_new_from_filename(cDesktopFilePath);
    if (!appInfo) {
        qDebug() << "Failed to open desktop file with gio: g_desktop_app_info_new_from_filename returns NULL. Check PATH maybe?";
        return false;
    }

    GList *gfiles = nullptr;
    foreach (const QString &url, fileUrls) {
        const char *cFilePath = url.toLocal8Bit().data();
        GFile *gfile = g_file_new_for_uri(cFilePath);
        gfiles = g_list_append(gfiles, gfile);
    }

    g_autoptr(GError) gerror = nullptr;
    gboolean ok = g_app_info_launch(reinterpret_cast<GAppInfo *>(appInfo), gfiles, nullptr, &gerror);

    if (gerror) {
        qWarning() << "Error when trying to open desktop file with gio:" << gerror->message;
    }

    if (!ok) {
        qWarning() << "Failed to open desktop file with gio: g_app_info_launch returns false";
    }
    if (gfiles)
        g_list_free(gfiles);

    return ok;
}

bool LocalFileHandler::isFileManagerSelf(const QString &desktopFile)
{
    /*
     *  return true if exec field contains dde-file-manager/file-manager.sh of dde-file-manager desktopFile
    */
    DesktopFile d(desktopFile);
    return d.desktopExec().contains("dde-file-manager") || d.desktopExec().contains("file-manager.sh");
}

bool LocalFileHandler::isSmbUnmountedFile(const QUrl &url)
{
    return url.path().startsWith("/run/user/")
            && url.path().contains("/gvfs/smb-share:server=");
    // TODO(lanxs)
    /*return url.path().startsWith("/run/user/")
            && url.path().contains("/gvfs/smb-share:server=")
            && DFileService::instance()->checkGvfsMountfileBusy(url, false);*/
}

QUrl LocalFileHandler::smbFileUrl(const QString &filePath)
{
    static QRegularExpression regExp("file:///run/user/\\d+/gvfs/smb-share:server=(?<host>.*),share=(?<path>.*)",
                                     QRegularExpression::DotMatchesEverythingOption
                                             | QRegularExpression::DontCaptureOption
                                             | QRegularExpression::OptimizeOnFirstUsageOption);

    const QRegularExpressionMatch &match = regExp.match(filePath, 0, QRegularExpression::NormalMatch,
                                                        QRegularExpression::DontCheckSubjectStringMatchOption);

    if (!match.hasMatch())
        return QUrl::fromLocalFile(filePath);

    const QString &host = match.captured("host");
    const QString &path = match.captured("path");

    QUrl newUrl;
    newUrl.setScheme(Global::kSmb);
    newUrl.setHost(host);
    newUrl.setPath("/" + path.mid(0, path.lastIndexOf("/")));
    return newUrl;
}

QString LocalFileHandler::getFileMimetypeFromGio(const QUrl &url)
{
    QSharedPointer<DFMIO::DIOFactory> factory = produceQSharedIOFactory(url.scheme(), static_cast<QUrl>(url));
    if (!factory) {
        qWarning() << "create factory failed, url: " << url;
        return QString();
    }
    QSharedPointer<DFMIO::DFileInfo> dfileinfo = factory->createFileInfo();
    if (!dfileinfo) {
        qWarning() << "create fileinfo failed, url: " << url;
        return QString();
    }

    bool succ = false;
    auto mimeType = dfileinfo->attribute(DFMIO::DFileInfo::AttributeID::kStandardContentType, &succ);
    if (succ)
        return mimeType.toString();

    return QString();
}

void LocalFileHandler::addRecentFile(const QString &filePath, const DesktopFile &desktopFile, const QString &mimetype)
{
    if (filePath.isEmpty()) {
        return;
    }
    DTK_CORE_NAMESPACE::DRecentData recentData;
    recentData.appName = desktopFile.desktopName();
    recentData.appExec = desktopFile.desktopExec();
    recentData.mimeType = mimetype;
    DTK_CORE_NAMESPACE::DRecentManager::removeItem(filePath);
    DTK_CORE_NAMESPACE::DRecentManager::addItem(filePath, recentData);
}

QString LocalFileHandler::defaultTerminalPath()
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

QString LocalFileHandler::getFileMimetype(const QString &path)
{
    g_autoptr(GFile) file;
    g_autoptr(GFileInfo) info;
    QString result = QString();

    file = g_file_new_for_path(path.toLocal8Bit().data());
    info = g_file_query_info(file, "standard::content-type", G_FILE_QUERY_INFO_NONE, nullptr, nullptr);
    if (info)
        result = g_file_info_get_content_type(info);

    g_object_unref(file);

    return result;
}

bool LocalFileHandler::renameFilesBatch(const QMap<QUrl, QUrl> &urls, QMap<QUrl, QUrl> &successUrls)
{
    successUrls.clear();

    QMap<QUrl, QUrl>::const_iterator beg = urls.constBegin();
    QMap<QUrl, QUrl>::const_iterator end = urls.constEnd();

    for (; beg != end; ++beg) {
        QUrl currentName { beg.key() };
        QUrl expectedName { beg.value() };

        if (currentName == expectedName) {
            continue;
        }

        ///###: just cache files that rename successfully.
        if (renameFile(currentName, expectedName)) {
            successUrls[currentName] = expectedName;
        }
    }

    return successUrls.size() == urls.size();
}
/*!
 * \brief LocalFileHandler::errorString 获取错误信息
 * \return
 */
QString LocalFileHandler::errorString()
{
    return lastError.errorMsg();
}

DFMIOErrorCode LocalFileHandler::errorCode()
{
    return lastError.code();
}
/*!
 * \brief LocalFileHandler::setError 设置当前的错误信息
 * \param error 错误信息
 */
void LocalFileHandler::setError(DFMIOError error)
{
    lastError = error;
}
