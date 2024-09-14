// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "localfilehandler.h"
#include "localfilehandler_p.h"

#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/mimetype/mimesappsmanager.h>
#include <dfm-base/mimetype/mimetypedisplaymanager.h>
#include <dfm-base/utils/desktopfile.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/networkutils.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/dbusservice/global_server_defines.h>

#include <dfm-io/doperator.h>
#include <dfm-io/dfile.h>
#include <dfm-io/denumerator.h>

#include <QString>
#include <QUrl>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QDebug>
#include <QRegularExpression>
#include <QProcess>
#include <QDesktopServices>
#include <QSettings>
#include <QDBusConnection>

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

using namespace dfmbase;
using namespace GlobalServerDefines;

LocalFileHandler::LocalFileHandler()
    : d(new LocalFileHandlerPrivate(this))
{
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
QUrl LocalFileHandler::touchFile(const QUrl &url, const QUrl &tempUrl /*= QUrl()*/)
{
    QSharedPointer<DFMIO::DOperator> oper { new DFMIO::DOperator(url) };
    if (!oper) {
        qCWarning(logDFMBase) << "create operator failed, url: " << url;
        return QUrl();
    }

    bool success = oper->touchFile();
    if (!success) {
        qCWarning(logDFMBase) << "touch file failed, url: " << url;

        d->setError(oper->lastError());

        return QUrl();
    } else {   // fix bug 189699 When the iPhone creates a file, the gio is created successfully, but there is no file
        auto info = InfoFactory::create<FileInfo>(url, Global::CreateFileInfoType::kCreateFileInfoSync);
        if (!info || !info->exists()) {
            d->lastError.setCode(DFMIOErrorCode::DFM_IO_ERROR_NOT_SUPPORTED);
            return QUrl();
        }
    }

    auto templateUrl = d->loadTemplateInfo(url, tempUrl);
    qCInfo(logDFMBase, "touchFile source file : %s, Template file %s, successed by dfmio function touchFile!",
           url.path().toStdString().c_str(), tempUrl.path().toStdString().c_str());
    FileUtils::notifyFileChangeManual(DFMGLOBAL_NAMESPACE::FileNotifyType::kFileAdded, url);

    return templateUrl;
}
/*!
 * \brief LocalFileHandler::mkdir 创建目录
 * \param dir 创建目录的url
 * \return bool 创建文件是否成功
 */
bool LocalFileHandler::mkdir(const QUrl &dir)
{
    QSharedPointer<DFMIO::DOperator> oper { new DFMIO::DOperator(dir) };
    if (!oper) {
        qCWarning(logDFMBase) << "create operator failed, url: " << dir;
        return false;
    }

    bool success = oper->makeDirectory();
    if (!success) {
        qCWarning(logDFMBase) << "make directory failed, url: " << dir;

        d->setError(oper->lastError());

        return false;
    }

    FileInfoPointer fileInfo = InfoFactory::create<FileInfo>(dir);
    fileInfo->refresh();

    qCInfo(logDFMBase, "mkdir source file : %s, successed by dfmio function makeDirectory!", dir.path().toStdString().c_str());
    FileUtils::notifyFileChangeManual(DFMGLOBAL_NAMESPACE::FileNotifyType::kFileAdded, dir);

    return true;
}
/*!
 * \brief LocalFileHandler::rmdir 删除目录
 * \param url 删除目录的url
 * \return bool 是否删除目录成功
 */
bool LocalFileHandler::rmdir(const QUrl &url)
{
    QSharedPointer<DFMIO::DOperator> oper { new DFMIO::DOperator(url) };

    if (!oper) {
        qCWarning(logDFMBase) << "create operator failed, url: " << url;
        return false;
    }

    QString targetTrash = oper->trashFile();
    if (targetTrash.isEmpty()) {
        qCWarning(logDFMBase) << "trash file failed, url: " << url;

        d->setError(oper->lastError());

        return false;
    }

    qCWarning(logDFMBase, "rmdir source file : %s, successed by dfmio function trashFile!", url.path().toStdString().c_str());

    FileUtils::notifyFileChangeManual(DFMGLOBAL_NAMESPACE::FileNotifyType::kFileDeleted, url);

    return true;
}
/*!
 * \brief LocalFileHandler::rename 重命名文件
 * \param url 源文件的url
 * \param newUrl 新文件的url
 * \return bool 重命名文件是否成功
 */
bool LocalFileHandler::renameFile(const QUrl &url, const QUrl &newUrl, const bool needCheck)
{
    if (!dfmbase::FileUtils::isLocalFile(url) || !dfmbase::FileUtils::isLocalFile(newUrl))
        return false;

    if (url.scheme() != newUrl.scheme())
        return false;

    {   // check hidden name
        if (needCheck) {
            const QString &newName = newUrl.fileName();
            if (!doHiddenFileRemind(newName))
                return true;
        }
    }

    // check device, use set displayname if device is mtp
    if (Q_UNLIKELY(FileUtils::isMtpFile(newUrl))) {
        const QUrl &fromParentUrl = UrlRoute::urlParent(url);
        const QUrl &toParentUrl = UrlRoute::urlParent(newUrl);
        if (fromParentUrl == toParentUrl) {
            // if fileinfo or other operation query info in mtp device, file will rename false. mtp device is busy
            const QString &newName = newUrl.fileName();
            QSharedPointer<DFMIO::DOperator> oper { new DFMIO::DOperator(url) };
            if (!oper) {
                qCWarning(logDFMBase) << "create operator failed, url: " << url;
                return false;
            }

            bool success = oper->renameFile(newName);
            qCInfo(logDFMBase, "rename source file : %s , target file :%s , successed : %d  in mtp\
                  by dfmio function rename!",
                   url.path().toStdString().c_str(),
                   newUrl.path().toStdString().c_str(),
                   success);
            if (success)
                return true;
        }
    }

    // use system api
    const QString &sourceFile = url.toLocalFile();
    const QString &targetFile = newUrl.toLocalFile();

    if (DFMIO::DFile(targetFile).exists()) {
        DFMIOError error;
        error.setCode(DFM_IO_ERROR_EXISTS);
        d->setError(error);
        return false;   // TODO(xust/lanxuesong): user interaction?
    }

    if (::rename(sourceFile.toLocal8Bit().constData(), targetFile.toLocal8Bit().constData()) == 0) {
        FileUtils::notifyFileChangeManual(DFMGLOBAL_NAMESPACE::FileNotifyType::kFileDeleted, url);
        FileUtils::notifyFileChangeManual(DFMGLOBAL_NAMESPACE::FileNotifyType::kFileAdded, newUrl);

        FileInfoPointer fileInfo = InfoFactory::create<FileInfo>(newUrl);
        fileInfo->refresh();
        qCInfo(logDFMBase, "rename source file : %s , target file :%s successed by system function rename!",
               sourceFile.toStdString().c_str(),
               targetFile.toStdString().c_str());
        return true;
    }

    QSharedPointer<DFMIO::DOperator> oper { new DFMIO::DOperator(url) };
    if (!oper) {
        qCWarning(logDFMBase) << "create operator failed, url: " << url;
        return false;
    }

    bool success = oper->renameFile(newUrl);
    qCInfo(logDFMBase, "rename source file : %s , target file :%s , successed : %d \
          by dfmio function rename!",
           url.path().toStdString().c_str(),
           newUrl.path().toStdString().c_str(), success);
    if (!success) {
        qCWarning(logDFMBase) << "rename file failed, url: " << url;

        d->setError(oper->lastError());

        return false;
    }

    FileInfoPointer fileInfo = InfoFactory::create<FileInfo>(newUrl);
    fileInfo->refresh();

    FileUtils::notifyFileChangeManual(DFMGLOBAL_NAMESPACE::FileNotifyType::kFileDeleted, url);
    FileUtils::notifyFileChangeManual(DFMGLOBAL_NAMESPACE::FileNotifyType::kFileAdded, newUrl);

    return true;
}
/*!
 * \brief LocalFileHandler::openFile 打开文件
 * \param file 打开文件的url
 * \return bool 打开文件是否成功
 */
bool LocalFileHandler::openFile(const QUrl &fileUrl)
{
    return openFiles({ fileUrl });
}
/*!
 * \brief LocalFileHandler::openFiles 打开多个文件
 * \param files 打开文件的url列表
 * \return bool 打开文件是否成功
 */
bool LocalFileHandler::openFiles(const QList<QUrl> &fileUrls)
{
    if (fileUrls.isEmpty())
        return true;

    QList<QUrl> urls = fileUrls;

    QList<QUrl> pathList;
    bool result = false;
    d->invalidPath.clear();
    for (QUrl &fileUrl : urls) {
        FileInfoPointer fileInfo = InfoFactory::create<FileInfo>(fileUrl);
        QUrl sourceUrl = fileUrl;
        QStringList targetList;
        targetList.append(fileUrl.path());
        FileInfoPointer fileInfoLink = fileInfo;

        while (fileInfoLink->isAttributes(OptInfoType::kIsSymLink)) {
            QString targetLink = fileInfoLink->pathOf(PathInfoType::kSymLinkTarget);
            targetLink = targetLink.endsWith(QDir::separator()) && targetLink != QDir::separator() ? QString(targetLink).left(targetLink.length() - 1)
                                                                                                   : targetLink;
            if (targetList.contains(targetLink))
                break;
            targetList.append(targetLink);
            // 网络文件检查
            if (NetworkUtils::instance()->checkFtpOrSmbBusy(QUrl::fromLocalFile(targetLink))) {
                DialogManager::instance()->showUnableToVistDir(targetLink);
                return true;
            }
            fileInfoLink = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(targetLink));
            if (!fileInfoLink) {
                DialogManagerInstance->showErrorDialog(QObject::tr("Unable to find the original file"), QString());
                return false;
            }
            const_cast<QUrl &>(fileUrl) = fileInfoLink->urlOf(UrlInfoType::kRedirectedFileUrl);
            if (d->isInvalidSymlinkFile(fileUrl)) {
                d->lastEvent = DialogManagerInstance->showBreakSymlinkDialog(fileInfoLink->nameOf(
                                                                                     NameInfoType::kFileName),
                                                                             fileInfo->urlOf(UrlInfoType::kUrl));
                d->invalidPath << sourceUrl;
                return d->lastEvent == DFMBASE_NAMESPACE::GlobalEventType::kUnknowType;
            }
        }

        if (d->isExecutableScript(fileUrl.path())) {
            int code = DialogManagerInstance->showRunExcutableScriptDialog(fileUrl);
            result = d->openExcutableScriptFile(fileUrl.path(), code) || result;
            continue;
        }

        if (d->isFileRunnable(fileUrl.path()) && !FileUtils::isDesktopFile(fileUrl)) {
            int code = DialogManagerInstance->showRunExcutableFileDialog(fileUrl);
            result = d->openExcutableFile(fileUrl.path(), code) || result;
            continue;
        }

        if (d->shouldAskUserToAddExecutableFlag(fileUrl.path()) && !FileUtils::isDesktopFile(fileUrl)) {
            int code = DialogManagerInstance->showAskIfAddExcutableFlagAndRunDialog();
            result = d->addExecutableFlagAndExecuse(fileUrl.path(), code) || result;
            continue;
        }

        QString urlPath = fileUrl.path();
        if (d->isFileWindowsUrlShortcut(urlPath)) {
            urlPath = d->getInternetShortcutUrl(urlPath);
            pathList << QUrl::fromLocalFile(urlPath);
        } else {
            pathList << fileUrl;
        }
    }

    if (!pathList.empty()) {
        result = d->doOpenFiles(pathList);
    } else {
        return true;
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
bool LocalFileHandler::openFilesByApp(const QList<QUrl> &fileUrls, const QString &desktopFile)
{
    bool ok = false;

    if (desktopFile.isEmpty()) {
        qCWarning(logDFMBase) << "Failed to open desktop file with gio: app file path is empty";
        return ok;
    }

    if (fileUrls.isEmpty()) {
        qCWarning(logDFMBase) << "Failed to open desktop file with gio: file path is empty";
        return ok;
    }

    qCDebug(logDFMBase) << desktopFile << fileUrls;

    GDesktopAppInfo *appInfo = g_desktop_app_info_new_from_filename(desktopFile.toLocal8Bit().constData());
    if (!appInfo) {
        qCWarning(logDFMBase) << "Failed to open desktop file with gio: g_desktop_app_info_new_from_filename returns NULL. Check PATH maybe?";
        return false;
    }

    QStringList filePathsStr;
    for (const auto &url : fileUrls) {
        filePathsStr << url.toString();
    }

    QString terminalFlag = QString(g_desktop_app_info_get_string(appInfo, "Terminal"));
    if (terminalFlag == "true") {
        QString exec = QString(g_desktop_app_info_get_string(appInfo, "Exec"));
        QStringList args;
        args << "-e" << exec.split(" ").at(0) << filePathsStr;
        QString termPath = defaultTerminalPath();
        qCDebug(logDFMBase) << termPath << args;
        ok = QProcess::startDetached(termPath, args);
    } else {
        ok = d->launchApp(desktopFile, filePathsStr);
    }
    g_object_unref(appInfo);

    if (ok) {
        // workaround since DTK apps doesn't support the recent file spec.
        // spec: https://www.freedesktop.org/wiki/Specifications/desktop-bookmark-spec/
        // the correct approach: let the app add it to the recent list.
        // addToRecentFile(DUrl::fromLocalFile(filePath), mimetype);
        QString mimetype = d->getFileMimetype(fileUrls.first());
        d->addRecentFile(desktopFile, fileUrls, mimetype);
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
    QSharedPointer<DFMIO::DOperator> oper { new DFMIO::DOperator(sourcefile) };

    if (!oper) {
        qCWarning(logDFMBase) << "create operator failed, url: " << sourcefile;
        return false;
    }

    bool success = oper->createLink(link);
    if (!success) {
        qCWarning(logDFMBase) << "create link failed, url: " << sourcefile << " link url: " << link;

        d->setError(oper->lastError());

        return false;
    }

    qCInfo(logDFMBase, "create system link, source file %s, link file %s successed !",
           sourcefile.path().toStdString().c_str(), link.path().toStdString().c_str());
    FileUtils::notifyFileChangeManual(DFMGLOBAL_NAMESPACE::FileNotifyType::kFileAdded, link);

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
    QSharedPointer<DFMIO::DFile> dfile { new DFMIO::DFile(url) };
    if (!dfile) {
        qCWarning(logDFMBase) << "create file failed, url: " << url;
        return false;
    }

    // if the `permissions` is invalid, do not set permissions
    // eg. bug-199607: Copy MTP folder to local, folder permissions wrong
    // reason: `dfm-io` uses gio to query the `unix::mode` field to get file permissions, but this field is not available in MTP file
    if (0 == permissions)
        return true;

    bool success = dfile->setPermissions(DFMIO::DFile::Permissions(uint16_t(permissions)));
    if (!success) {
        qCWarning(logDFMBase) << "set permissions failed, url: " << url;

        d->setError(dfile->lastError());

        return false;
    }

    return true;
}

bool LocalFileHandler::setPermissionsRecursive(const QUrl &url, QFileDevice::Permissions permissions)
{
    FileInfoPointer info { InfoFactory::create<FileInfo>(url) };
    if (!info)
        return false;
    bool isFile { info->isAttributes(OptInfoType::kIsFile) };
    bool isDir { info->isAttributes(OptInfoType::kIsDir) };
    if (isFile)
        return setPermissions(url, permissions);

    if (isDir) {
        DFMIO::DEnumerator enumerator(url);
        bool succ { false };
        while (enumerator.hasNext()) {
            const QUrl &nextUrl = enumerator.next();
            info = InfoFactory::create<FileInfo>(nextUrl);
            isDir = info->isAttributes(OptInfoType::kIsDir);
            if (isDir)
                succ = setPermissionsRecursive(nextUrl, permissions);
            else
                succ = setPermissions(nextUrl, permissions);
        }
        succ = setPermissions(url, permissions);
        return succ;
    }

    return false;
}

bool LocalFileHandler::moveFile(const QUrl &sourceUrl, const QUrl &destUrl, DFMIO::DFile::CopyFlag flag /*= DFMIO::DFile::CopyFlag::kNone*/)
{
    QSharedPointer<DFMIO::DOperator> dOperator { new DFMIO::DOperator(sourceUrl) };
    if (!dOperator) {
        qCWarning(logDFMBase) << "create file operator failed, url: " << sourceUrl;
        return false;
    }

    bool success = dOperator->moveFile(destUrl, flag);
    if (!success) {
        qCWarning(logDFMBase) << "move file failed, source url: " << sourceUrl << " destUrl: " << destUrl;

        d->setError(dOperator->lastError());

        return false;
    }

    return true;
}

bool LocalFileHandler::copyFile(const QUrl &sourceUrl, const QUrl &destUrl, dfmio::DFile::CopyFlag flag)
{
    QSharedPointer<DFMIO::DOperator> dOperator { new DFMIO::DOperator(sourceUrl) };
    if (!dOperator) {
        qCWarning(logDFMBase) << "create file operator failed, url: " << sourceUrl;
        return false;
    }

    bool success = dOperator->copyFile(destUrl, flag);
    if (!success) {
        qCWarning(logDFMBase) << "copy file failed, source url: " << sourceUrl << " destUrl: " << destUrl;

        d->setError(dOperator->lastError());

        return false;
    }

    return true;
}

QString LocalFileHandler::trashFile(const QUrl &url)
{
    QSharedPointer<DFMIO::DOperator> dOperator { new DFMIO::DOperator(url) };
    if (!dOperator) {
        qCWarning(logDFMBase) << "create file operator failed, url: " << url;
        return QString();
    }

    QString targetTrash = dOperator->trashFile();
    if (targetTrash.isEmpty()) {
        qCWarning(logDFMBase) << "trash file failed, url: " << url;
        d->setError(dOperator->lastError());
    }

    return targetTrash;
}
/*!
 * \brief LocalFileHandler::deleteFile
 * \param file 文件的url
 * \return bool 删除文件是否失败
 */
bool LocalFileHandler::deleteFile(const QUrl &url)
{
    QSharedPointer<DFMIO::DOperator> dOperator { new DFMIO::DOperator(url) };

    if (!dOperator) {
        qCWarning(logDFMBase) << "create file operator failed, url: " << url;
        return false;
    }

    bool success = dOperator->deleteFile();
    if (!success) {
        qCWarning(logDFMBase) << "delete file failed, url: " << url;

        d->setError(dOperator->lastError());

        return false;
    }
    FileUtils::notifyFileChangeManual(DFMGLOBAL_NAMESPACE::FileNotifyType::kFileDeleted, url);
    qCWarning(logDFMBase) << "delete file success: " << url;

    return true;
}

bool LocalFileHandler::deleteFileRecursive(const QUrl &url)
{
    qCInfo(logDFMBase) << "Recursive delete " << url;
    if (SystemPathUtil::instance()->isSystemPath(url.toLocalFile())) {
        qCWarning(logDFMBase) << "Cannot delete system path!!!!!!!!!!!!!!!!!";
        abort();
    }

    FileInfoPointer info { InfoFactory::create<FileInfo>(url) };
    if (!info)
        return false;

    if (!info->isAttributes(OptInfoType::kIsDir))
        return deleteFile(url);

    QSharedPointer<DFMIO::DEnumerator> enumerator { new DFMIO::DEnumerator(url) };
    if (!enumerator) {
        qCWarning(logDFMBase) << "Cannot create enumerator";
        return false;
    }

    bool succ { false };
    while (enumerator->hasNext()) {
        const QUrl &urlNext = enumerator->next();
        info = InfoFactory::create<FileInfo>(urlNext);
        if (info->isAttributes(OptInfoType::kIsDir))
            succ = deleteFileRecursive(urlNext);
        else
            succ = deleteFile(urlNext);
    }

    succ = deleteFile(url);
    return succ;
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
    utimbuf buf = { accessDateTime.toSecsSinceEpoch(), lastModifiedTime.toSecsSinceEpoch() };

    if (::utime(url.toLocalFile().toLocal8Bit(), &buf) == 0) {
        return true;
    }

    d->setError(DFMIOError(DFM_IO_ERROR_NOT_SUPPORTED));

    return false;
}

bool LocalFileHandlerPrivate::launchApp(const QString &desktopFilePath, const QStringList &fileUrls)
{
    QStringList newFileUrls(fileUrls);

    if (isFileManagerSelf(desktopFilePath) && fileUrls.count() > 1) {
        for (const QString &url : fileUrls) {
            q->openFile(url);
        }
        return true;
    }

    // url path will be truncated at the index of '#', so replace it if it's real existed in url. (mostly it's for avfs archive paths)
    std::for_each(newFileUrls.begin(), newFileUrls.end(), [](QString &path) { path.replace("#", "%23"); });
    bool ok = launchAppByDBus(desktopFilePath, newFileUrls);
    if (!ok) {
        ok = launchAppByGio(desktopFilePath, newFileUrls);
    }
    return ok;
}

bool LocalFileHandlerPrivate::launchAppByDBus(const QString &desktopFile, const QStringList &filePaths)
{
    qCInfo(logDFMBase, "launch App By DBus, desktopFile : %s, files count : %d !", desktopFile.toStdString().c_str(), filePaths.count());
    qCDebug(logDFMBase) << "launch App By DBus, files : \n"
                        << filePaths;
    if (UniversalUtils::checkLaunchAppInterface())
        return UniversalUtils::launchAppByDBus(desktopFile, filePaths);
    return false;
}

bool LocalFileHandlerPrivate::launchAppByGio(const QString &desktopFilePath, const QStringList &fileUrls)
{
    qCDebug(logDFMBase) << "launchApp by gio:" << desktopFilePath << fileUrls;

    const QByteArray &cDesktopFilePath = desktopFilePath.toLocal8Bit();

    g_autoptr(GDesktopAppInfo) appInfo = g_desktop_app_info_new_from_filename(cDesktopFilePath.data());
    if (!appInfo) {
        qCWarning(logDFMBase) << "Failed to open desktop file with gio: g_desktop_app_info_new_from_filename returns NULL. Check PATH maybe?";
        return false;
    }

    GList *gfiles = nullptr;
    foreach (const QString &url, fileUrls) {
        const QByteArray &cFilePath = url.toLocal8Bit();
        GFile *gfile = g_file_new_for_uri(cFilePath.data());
        gfiles = g_list_append(gfiles, gfile);
    }

    g_autoptr(GError) gerror = nullptr;
    gboolean ok = g_app_info_launch(reinterpret_cast<GAppInfo *>(appInfo), gfiles, nullptr, &gerror);

    if (gerror) {
        qCWarning(logDFMBase) << "Error when trying to open desktop file with gio:" << gerror->message;
    }

    if (!ok) {
        qCWarning(logDFMBase) << "Failed to open desktop file with gio: g_app_info_launch returns false";
    }
    if (gfiles)
        g_list_free(gfiles);

    return ok;
}

bool LocalFileHandlerPrivate::isFileManagerSelf(const QString &desktopFile)
{
    /*
     *  return true if exec field contains dde-file-manager/file-manager.sh of dde-file-manager desktopFile
     */
    DesktopFile d(desktopFile);
    return d.desktopExec().contains("dde-file-manager") || d.desktopExec().contains("file-manager.sh");
}

bool LocalFileHandlerPrivate::isInvalidSymlinkFile(const QUrl &url)
{
    FileInfoPointer info { InfoFactory::create<FileInfo>(url) };
    if (!info)
        return true;

    const QString &path { info->pathOf(PathInfoType::kAbsoluteFilePath) };
    if (!DFMIO::DFile(path).exists() && !DeviceUtils::isSamba(url))
        return true;

    return false;
}

void LocalFileHandlerPrivate::doAddRecentFile(const QVariantMap &item)
{
    if (item.isEmpty())
        return;

    QDBusMessage message = QDBusMessage::createMethodCall("org.deepin.Filemanager.Daemon",
                                                          "/org/deepin/Filemanager/Daemon/RecentManager",
                                                          "org.deepin.Filemanager.Daemon.RecentManager",
                                                          "AddItem");
    message << QVariant::fromValue(item);

    QDBusReply<void> reply = QDBusConnection::sessionBus().call(message);
    if (!reply.isValid())
        qCWarning(logDFMBase) << "D-Bus call AddItem failed:" << reply.error().name() << reply.error().message();
    else
        qCDebug(logDFMBase) << "D-Bus call AddItem succeeded";
}

QVariantMap LocalFileHandlerPrivate::buildRecentItem(const QString &path, const DesktopFile &desktop, const QString &mimeType)
{
    QVariantMap item;

    item.insert(RecentProperty::kPath, path);
    item.insert(RecentProperty::kAppName, desktop.desktopName());
    item.insert(RecentProperty::kAppExec, desktop.desktopExec());
    item.insert(RecentProperty::kMimeType, mimeType);

    return item;
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

QString LocalFileHandlerPrivate::getFileMimetype(const QUrl &url)
{
    g_autoptr(GFile) file;
    g_autoptr(GFileInfo) info;
    QString result = QString();

    file = g_file_new_for_uri(url.toString().toStdString().c_str());
    info = g_file_query_info(file, "standard::content-type", G_FILE_QUERY_INFO_NONE, nullptr, nullptr);
    if (info)
        result = g_file_info_get_content_type(info);

    return result;
}

bool LocalFileHandlerPrivate::isExecutableScript(const QString &path)
{
    QString pathValue = path.endsWith(QDir::separator()) && path != QDir::separator() ? QString(path).left(path.length() - 1)
                                                                                      : path;
    QString mimetype = getFileMimetype(QUrl::fromLocalFile(path));
    FileInfoPointer info { InfoFactory::create<FileInfo>(QUrl::fromLocalFile(pathValue)) };
    if (!info)
        return false;
    bool isSymLink { info->isAttributes(OptInfoType::kIsSymLink) };
    QStringList targetList;
    targetList.append(path);
    while (isSymLink) {
        pathValue = info->pathOf(PathInfoType::kSymLinkTarget);
        pathValue = pathValue.endsWith(QDir::separator()) && pathValue != QDir::separator() ? QString(pathValue).left(pathValue.length() - 1)
                                                                                            : pathValue;
        if (targetList.contains(pathValue))
            break;
        targetList.append(pathValue);
        mimetype = getFileMimetype(QUrl::fromLocalFile(pathValue));
        info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(pathValue));
    }

    // blumia: it's not a good idea to check if it is a executable script by just checking
    //         mimetype.startsWith("text/"), should be fixed later.
    if (mimetype.startsWith("text/") || (mimetype == "application/x-shellscript"))
        return isFileExecutable(pathValue);

    return false;
}

bool LocalFileHandlerPrivate::isFileExecutable(const QString &path)
{
    FileInfoPointer info { InfoFactory::create<FileInfo>(QUrl::fromLocalFile(path)) };
    if (!info)
        return false;

    // regard these type as unexecutable.
    const static QStringList kinValidateType { "txt", "md" };
    if (kinValidateType.contains(info->nameOf(NameInfoType::kSuffix)))
        return false;

    QFile::Permissions permissions { info->permissions() };
    bool isExeUser = permissions & QFile::Permission::ExeUser;
    bool isReadUser = permissions & QFile::Permission::ReadUser;

    return isExeUser && isReadUser;
}

bool LocalFileHandlerPrivate::openExcutableScriptFile(const QString &path, int flag)
{
    bool result = false;
    switch (flag) {
    case 0:

        break;
    case 1:
        result = UniversalUtils::runCommand(path, QStringList(), QUrl(path).adjusted(QUrl::RemoveFilename).toString());
        break;
    case 2: {
        QStringList args;
        args << "-e" << path;
        result = UniversalUtils::runCommand(q->defaultTerminalPath(), args, QUrl(path).adjusted(QUrl::RemoveFilename).toString());
        break;
    }
    case 3:
        result = doOpenFile(QUrl::fromLocalFile(path));
        break;
    default:
        break;
    }

    return result;
}

bool LocalFileHandlerPrivate::openExcutableFile(const QString &path, int flag)
{
    bool result = false;
    switch (flag) {
    case 0:
        break;
    case 1: {
        QStringList args;
        args << "-e" << path;
        result = UniversalUtils::runCommand(q->defaultTerminalPath(), args, QUrl(path).adjusted(QUrl::RemoveFilename).toString());
        break;
    }
    case 2:
        result = UniversalUtils::runCommand(path, QStringList(), QUrl(path).adjusted(QUrl::RemoveFilename).toString());
        break;
    default:
        break;
    }

    return result;
}

bool LocalFileHandlerPrivate::isFileRunnable(const QString &path)
{
    QString pathValue = path.endsWith(QDir::separator()) && path != QDir::separator() ? QString(path).left(path.length() - 1)
                                                                                      : path;
    QString mimetype = getFileMimetype(QUrl::fromLocalFile(path));
    FileInfoPointer info { InfoFactory::create<FileInfo>(QUrl::fromLocalFile(pathValue)) };
    if (!info)
        return false;
    QStringList targetList;
    targetList.append(pathValue);
    while (info->isAttributes(OptInfoType::kIsSymLink)) {
        pathValue = info->pathOf(PathInfoType::kSymLinkTarget);
        pathValue = pathValue.endsWith(QDir::separator()) && pathValue != QDir::separator() ? QString(pathValue).left(pathValue.length() - 1)
                                                                                            : pathValue;
        if (targetList.contains(pathValue))
            break;
        targetList.append(pathValue);
        mimetype = getFileMimetype(QUrl::fromLocalFile(pathValue));
        info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(pathValue));
    }

    // blumia: about AppImage mime type, please refer to:
    //         https://cgit.freedesktop.org/xdg/shared-mime-info/tree/freedesktop.org.xml.in
    //         btw, consider using MimeTypeDisplayManager::ExecutableMimeTypes(private) ?
    if (mimetype == "application/x-executable"
        || mimetype == "application/x-sharedlib"
        || mimetype == "application/x-iso9660-appimage"
        || mimetype == "application/vnd.appimage") {
        return isFileExecutable(pathValue);
    }

    return false;
}

bool LocalFileHandlerPrivate::shouldAskUserToAddExecutableFlag(const QString &path)
{
    QString pathValue = path.endsWith(QDir::separator()) && path != QDir::separator() ? QString(path).left(path.length() - 1)
                                                                                      : path;
    ;
    QString mimetype = getFileMimetype(QUrl::fromLocalFile(path));
    FileInfoPointer info { InfoFactory::create<FileInfo>(QUrl::fromLocalFile(pathValue)) };
    if (!info)
        return false;

    QStringList targetList;
    targetList.append(pathValue);
    while (info->isAttributes(OptInfoType::kIsSymLink)) {
        pathValue = info->pathOf(PathInfoType::kSymLinkTarget);
        pathValue = pathValue.endsWith(QDir::separator()) && pathValue != QDir::separator() ? QString(pathValue).left(pathValue.length() - 1)
                                                                                            : pathValue;
        if (targetList.contains(pathValue))
            break;
        targetList.append(pathValue);
        mimetype = getFileMimetype(QUrl::fromLocalFile(pathValue));
        info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(pathValue));
    }

    if (mimetype == "application/x-executable"
        || mimetype == "application/x-sharedlib"
        || mimetype == "application/x-iso9660-appimage"
        || mimetype == "application/vnd.appimage") {
        return !isFileExecutable(pathValue);
    }

    return false;
}

bool LocalFileHandlerPrivate::addExecutableFlagAndExecuse(const QString &path, int flag)
{
    bool result = false;
    DFMIO::DFile file(path);
    switch (flag) {
    case 0:
        break;
    case 1:
        file.setPermissions(file.permissions() | DFMIO::DFile::Permission::kExeOwner | DFMIO::DFile::Permission::kExeUser | DFMIO::DFile::Permission::kExeGroup | DFMIO::DFile::Permission::kExeOther);
        result = UniversalUtils::runCommand(path, QStringList());
        break;
    default:
        break;
    }

    return result;
}

bool LocalFileHandlerPrivate::isFileWindowsUrlShortcut(const QString &path)
{
    QString mimetype = getFileMimetype(QUrl::fromLocalFile(path));
    qCDebug(logDFMBase) << mimetype;
    if (mimetype == "application/x-mswinurl")
        return true;
    return false;
}

QString LocalFileHandlerPrivate::getInternetShortcutUrl(const QString &path)
{
    QSettings settings(path, QSettings::IniFormat);
    settings.beginGroup("InternetShortcut");
    QString url = settings.value("URL").toString();
    settings.endGroup();
    return url;
}

QUrl LocalFileHandlerPrivate::loadTemplateInfo(const QUrl &url, const QUrl &templateUrl /*= QUrl()*/)
{
    QUrl templateFile = templateUrl;
    if (!templateFile.isValid()) {

        FileInfoPointer targetFileInfo { InfoFactory::create<FileInfo>(url, Global::CreateFileInfoType::kCreateFileInfoSync) };
        const QString &suffix = targetFileInfo->suffix();
        templateFile = loadTemplateUrl(suffix);
    }

    if (templateFile.isValid()) {
        const QByteArray &arr { DFMIO::DFile(templateFile).readAll() };
        DFMIO::DFile file(url);
        if (!arr.isEmpty() && file.open(DFMIO::DFile::OpenFlag::kAppend)) {
            qint64 writeCount = file.write(arr);
            if (writeCount <= 0)
                qCWarning(logDFMBase) << "file touch succ, but write template failed";
        }

        FileInfoPointer fileInfo = InfoFactory::create<FileInfo>(url);
        fileInfo->refresh();
    } else {
        templateFile = url;
    }
    return templateFile;
}

bool LocalFileHandlerPrivate::doOpenFile(const QUrl &url, const QString &desktopFile /*= QString()*/)
{
    return doOpenFiles({ url }, desktopFile);
}

bool LocalFileHandlerPrivate::doOpenFiles(const QList<QUrl> &urls, const QString &desktopFile /*= QString()*/)
{
    bool ret = false;

    QList<QUrl> transUrls = urls;
    for (const QUrl &url : urls) {
        FileInfoPointer info { InfoFactory::create<FileInfo>(url) };
        if (info->nameOf(NameInfoType::kSuffix) == Global::Scheme::kDesktop) {
            ret = launchApp(url.path()) || ret;   //有一个成功就成功
            transUrls.removeOne(url);
            continue;
        }
    }
    if (transUrls.isEmpty())
        return ret;

    // 找出所有可以打开的文件的信息，这里比较耗时，需要优化
    QMultiMap<QString, QString> openInfos, mountOpenInfos, cmdOpenInfos;
    QMap<QString, QString> openMineTypes, mountMineTypes, cmdMineTypes;
    for (auto url : transUrls) {
        const QUrl &fileUrl = url;
        const QString &filePath = fileUrl.path();

        FileInfoPointer fileInfo = InfoFactory::create<FileInfo>(fileUrl);

        QString mimeType;
        if (Q_UNLIKELY(!filePath.contains("#")) && fileInfo && fileInfo->size() == 0 && fileInfo->exists()) {
            mimeType = fileInfo->nameOf(NameInfoType::kMimeTypeName);
        } else {
            mimeType = getFileMimetype(fileUrl);
        }

        QString defaultDesktopFile;

        if (!desktopFile.isEmpty() && isFileManagerSelf(desktopFile)) {
            defaultDesktopFile = desktopFile;
            openInfos.insert(defaultDesktopFile, url.toString());
            openMineTypes.insert(url.toString(), mimeType);
        } else {
            defaultDesktopFile = MimesAppsManager::getDefaultAppDesktopFileByMimeType(mimeType);
            QAtomicInteger<bool> isOpenNow = false;
            if (defaultDesktopFile.isEmpty() || !dfmio::DFile(fileUrl).exists()) {
                if (DeviceUtils::isUnmountSamba(fileUrl)) {
                    mimeType = QString("inode/directory");
                    defaultDesktopFile = MimesAppsManager::getDefaultAppDesktopFileByMimeType(mimeType);
                    isOpenNow = true;
                    mimeType = QString();
                    mountOpenInfos.insert(defaultDesktopFile,
                                          DeviceUtils::getSambaFileUriFromNative(fileUrl).toString());
                    mountMineTypes.insert(DeviceUtils::getSambaFileUriFromNative(fileUrl).toString(),
                                          QString("inode/directory"));
                } else {
                    qCWarning(logDFMBase) << "no default application for" << fileUrl;
                }
                continue;
            }

            if (!isOpenNow && isFileManagerSelf(defaultDesktopFile) && mimeType != "inode/directory") {
                QStringList recommendApps = MimesAppsManager::getRecommendedApps(fileUrl);
                recommendApps.removeOne(defaultDesktopFile);
                if (recommendApps.count() > 0) {
                    defaultDesktopFile = recommendApps.first();
                    cmdOpenInfos.insert(defaultDesktopFile, url.toString());
                    cmdMineTypes.insert(url.toString(), mimeType);
                } else {
                    qCWarning(logDFMBase) << "no default application for" << transUrls;
                    continue;
                }
            }

            openInfos.insert(defaultDesktopFile, url.toString());
            openMineTypes.insert(url.toString(), mimeType);
        }
    }

    // 打开所有的文件
    if (openInfos.isEmpty() && mountOpenInfos.isEmpty() && cmdOpenInfos.isEmpty())
        return false;

    bool openResult = doOpenFiles(openInfos, openMineTypes);

    bool openMount = doOpenFiles(mountOpenInfos, mountMineTypes);

    bool openCmd = doOpenFiles(cmdOpenInfos, cmdMineTypes);

    if (openResult || openMount || openCmd)
        return true;

    if (DeviceUtils::isSamba(transUrls[0]))
        return false;

    QStringList paths;
    for (const QUrl &url : transUrls) {
        paths << url.path();
    }

    if (MimesAppsManager::getDefaultAppByFileName(transUrls.first().path()) == "org.gnome.font-viewer.desktop") {
        QProcess::startDetached("gio", QStringList() << "open" << paths);
        QTimer::singleShot(200, [=] {
            QProcess::startDetached("gio", QStringList() << "open" << paths);
        });
        return true;
    }

    bool result = QProcess::startDetached("gio", QStringList() << "open" << paths);

    if (!result) {
        result = false;
        for (const QUrl &url : transUrls)
            result = QDesktopServices::openUrl(url) || result;   //有一个成功就成功
    }
    return result;
}

bool LocalFileHandlerPrivate::doOpenFiles(const QMultiMap<QString, QString> &infos, const QMap<QString, QString> &mimeTypes)
{
    if (infos.isEmpty())
        return false;
    bool result { false };
    for (const auto &key : infos.uniqueKeys()) {
        auto urls = infos.values(key);
        bool tmp = launchApp(key, urls);
        result = result ? result : tmp;
        if (tmp) {
            // workaround since DTK apps doesn't support the recent file spec.
            // spec: https://www.freedesktop.org/wiki/Specifications/desktop-bookmark-spec/
            // the correct approach: let the app add it to the recent list.
            // addToRecentFile(DUrl::fromLocalFile(filePath), mimetype);
            addRecentFile(key, urls, mimeTypes);
        }
    }

    return result;
}

bool LocalFileHandler::renameFilesBatch(const QMap<QUrl, QUrl> &urls, QMap<QUrl, QUrl> &successUrls)
{
    successUrls.clear();

    QMap<QUrl, QUrl>::const_iterator beg = urls.constBegin();
    QMap<QUrl, QUrl>::const_iterator end = urls.constEnd();

    bool check = false;
    for (; beg != end; ++beg) {
        QUrl currentName { beg.key() };
        QUrl expectedName { beg.value() };

        if (currentName == expectedName) {
            continue;
        }

        auto fileinfo = InfoFactory::create<FileInfo>(expectedName);

        if (!check) {
            bool checkPass = doHiddenFileRemind(fileinfo->nameOf(NameInfoType::kFileName), &check);
            if (!checkPass)
                return true;
        }

        ///###: just cache files that rename successfully.
        if (renameFile(currentName, expectedName, false)) {
            successUrls[currentName] = expectedName;
        }
    }

    qCInfo(logDFMBase, "rename Files Batch, source files count : %d, success files count : %d .\
          ",
           urls.count(), successUrls.count());

    qCDebug(logDFMBase) << "rename Files Batch, source files : \n %1 " << urls << "\n successed files : \n %2" << successUrls;

    return successUrls.size() == urls.size();
}

bool LocalFileHandler::doHiddenFileRemind(const QString &name, bool *checkRule)
{
    if (!name.startsWith("."))
        return true;
    if (Application::instance()->genericAttribute(Application::kShowedHiddenFiles).toBool())
        return true;

    if (checkRule)
        *checkRule = true;

    // show dialog
    auto result = DialogManagerInstance->showRenameNameDotBeginDialog();
    return result == 1;
}
/*!
 * \brief LocalFileHandler::errorString 获取错误信息
 * \return
 */
QString LocalFileHandler::errorString()
{
    return d->lastError.errorMsg();
}

DFMIOErrorCode LocalFileHandler::errorCode()
{
    return d->lastError.code();
}

QList<QUrl> LocalFileHandler::getInvalidPath()
{
    return d->invalidPath;
}

GlobalEventType LocalFileHandler::lastEventType()
{
    return d->lastEvent;
}
/*!
 * \brief LocalFileHandler::setError 设置当前的错误信息
 * \param error 错误信息
 */
void LocalFileHandlerPrivate::setError(DFMIOError error)
{
    lastError = error;
}

QUrl LocalFileHandlerPrivate::loadTemplateUrl(const QString &suffix)
{
    QUrl templateFile;
    const QUrl &tempdir { QUrl::fromLocalFile(StandardPaths::location(StandardPaths::kTemplatesPath)) };
    DFMIO::DEnumerator enumerator(tempdir,
                                  {},
                                  static_cast<DFMIO::DEnumerator::DirFilter>(static_cast<int32_t>(QDir::Files)),
                                  DFMIO::DEnumerator::IteratorFlag::kNoIteratorFlags);
    while (enumerator.hasNext()) {
        if (enumerator.fileInfo()->attribute(DFMIO::DFileInfo::AttributeID::kStandardSuffix) == suffix) {
            templateFile = enumerator.next();
            break;
        }
    }
    return templateFile;
}

void LocalFileHandlerPrivate::addRecentFile(const QString &desktop, const QList<QString> urls, const QMap<QString, QString> &mimeTypes)
{
    for (const auto &tmpUrl : urls) {
        QUrl url(tmpUrl);
        QString filePath = url.toLocalFile();
        DesktopFile df(desktop);
        const auto &item { buildRecentItem(filePath, df, mimeTypes.value(tmpUrl)) };
        doAddRecentFile(item);
    }
}

void LocalFileHandlerPrivate::addRecentFile(const QString &desktop, const QList<QUrl> urls, const QString &mimeType)
{
    for (const auto &tmpUrl : urls) {
        QUrl url(tmpUrl);
        QString filePath = url.toLocalFile();
        DesktopFile df(desktop);
        const auto &item { buildRecentItem(filePath, df, mimeType) };
        doAddRecentFile(item);
    }
}

LocalFileHandlerPrivate::LocalFileHandlerPrivate(LocalFileHandler *handler)
    : q(handler)
{
}
