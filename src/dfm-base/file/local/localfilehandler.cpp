// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "localfilehandler.h"
#include "localfilehandler_p.h"

#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/utils/protocolutils.h>
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
#include <dfm-base/utils/protocolutils.h>

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
#include <cerrno>
#include <cstring>

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
    if (!url.isValid()) {
        qCWarning(logDFMBase) << "LocalFileHandler::touchFile: Invalid URL provided:" << url;
        return QUrl();
    }

    qCDebug(logDFMBase) << "LocalFileHandler::touchFile: Creating file at:" << url;

    QSharedPointer<DFMIO::DOperator> oper { new DFMIO::DOperator(url) };
    if (!oper) {
        qCCritical(logDFMBase) << "LocalFileHandler::touchFile: Failed to create DOperator for:" << url;
        return QUrl();
    }

    bool success = oper->touchFile();
    if (!success) {
        qCWarning(logDFMBase) << "LocalFileHandler::touchFile: Failed to create file:" << url
                              << "Error:" << oper->lastError().errorMsg();
        d->setError(oper->lastError());
        return QUrl();
    } else {   // fix bug 189699 When the iPhone creates a file, the gio is created successfully, but there is no file
        auto info = InfoFactory::create<FileInfo>(url, Global::CreateFileInfoType::kCreateFileInfoSync);
        if (!info || !info->exists()) {
            qCWarning(logDFMBase) << "LocalFileHandler::touchFile: File creation reported success but file does not exist:" << url;
            d->lastError.setCode(DFMIOErrorCode::DFM_IO_ERROR_NOT_SUPPORTED);
            return QUrl();
        }
    }

    auto templateUrl = d->loadTemplateInfo(url, tempUrl);
    qCInfo(logDFMBase) << "LocalFileHandler::touchFile: Successfully created file:" << url
                       << "Template:" << (tempUrl.isValid() ? tempUrl.toString() : "none");
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
    qCDebug(logDFMBase) << "LocalFileHandler::mkdir: Creating directory:" << dir;

    QSharedPointer<DFMIO::DOperator> oper { new DFMIO::DOperator(dir) };
    if (!oper) {
        qCCritical(logDFMBase) << "LocalFileHandler::mkdir: Failed to create DOperator for:" << dir;
        return false;
    }

    bool success = oper->makeDirectory();
    if (!success) {
        qCWarning(logDFMBase) << "LocalFileHandler::mkdir: Failed to create directory:" << dir
                              << "Error:" << oper->lastError().errorMsg();
        d->setError(oper->lastError());
        return false;
    }

    FileInfoPointer fileInfo = InfoFactory::create<FileInfo>(dir);
    fileInfo->refresh();

    qCInfo(logDFMBase) << "LocalFileHandler::mkdir: Successfully created directory:" << dir;
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
    qCDebug(logDFMBase) << "LocalFileHandler::rmdir: Moving directory to trash:" << url;

    QSharedPointer<DFMIO::DOperator> oper { new DFMIO::DOperator(url) };

    if (!oper) {
        qCCritical(logDFMBase) << "LocalFileHandler::rmdir: Failed to create DOperator for:" << url;
        return false;
    }

    QString targetTrash = oper->trashFile();
    if (targetTrash.isEmpty()) {
        qCWarning(logDFMBase) << "LocalFileHandler::rmdir: Failed to move directory to trash:" << url
                              << "Error:" << oper->lastError().errorMsg();
        d->setError(oper->lastError());
        return false;
    }

    qCInfo(logDFMBase) << "LocalFileHandler::rmdir: Successfully moved directory to trash:" << url
                       << "Trash location:" << targetTrash;

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
    if (!url.isLocalFile() || !newUrl.isLocalFile())
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
    if (Q_UNLIKELY(ProtocolUtils::isMTPFile(newUrl))) {
        const QUrl &fromParentUrl = UrlRoute::urlParent(url);
        const QUrl &toParentUrl = UrlRoute::urlParent(newUrl);
        if (fromParentUrl == toParentUrl) {
            // if fileinfo or other operation query info in mtp device, file will rename false. mtp device is busy
            qCDebug(logDFMBase) << "LocalFileHandler::renameFile: Renaming MTP file:" << url << "to:" << newUrl;
            const QString &newName = newUrl.fileName();
            QSharedPointer<DFMIO::DOperator> oper { new DFMIO::DOperator(url) };
            if (!oper) {
                qCCritical(logDFMBase) << "LocalFileHandler::renameFile: Failed to create DOperator for MTP file:" << url;
                return false;
            }

            bool success = oper->renameFile(newName);
            if (success) {
                qCInfo(logDFMBase) << "LocalFileHandler::renameFile: Successfully renamed MTP file:" << url
                                   << "to:" << newUrl;
                return true;
            } else {
                qCWarning(logDFMBase) << "LocalFileHandler::renameFile: Failed to rename MTP file:" << url
                                      << "to:" << newUrl << "Error:" << oper->lastError().errorMsg();
            }
        }
    }

    // use system api
    const QString &sourceFile = url.toLocalFile();
    const QString &targetFile = newUrl.toLocalFile();

    if (DFMIO::DFile(targetFile).exists()) {
        qCWarning(logDFMBase) << "LocalFileHandler::renameFile: Target file already exists:" << targetFile;
        DFMIOError error;
        error.setCode(DFM_IO_ERROR_EXISTS);
        d->setError(error);
        return false;   // TODO(xust/lanxuesong): user interaction?
    }

    qCDebug(logDFMBase) << "LocalFileHandler::renameFile: Using system rename API for:" << sourceFile << "to:" << targetFile;

    if (::rename(sourceFile.toLocal8Bit().constData(), targetFile.toLocal8Bit().constData()) == 0) {
        FileUtils::notifyFileChangeManual(DFMGLOBAL_NAMESPACE::FileNotifyType::kFileDeleted, url);
        FileUtils::notifyFileChangeManual(DFMGLOBAL_NAMESPACE::FileNotifyType::kFileAdded, newUrl);

        FileInfoPointer fileInfo = InfoFactory::create<FileInfo>(newUrl);
        fileInfo->refresh();
        qCInfo(logDFMBase) << "LocalFileHandler::renameFile: Successfully renamed file using system API:"
                           << sourceFile << "to:" << targetFile;
        return true;
    }

    qCDebug(logDFMBase) << "LocalFileHandler::renameFile: System rename failed, trying dfmio rename for:" << url;

    QSharedPointer<DFMIO::DOperator> oper { new DFMIO::DOperator(url) };
    if (!oper) {
        qCCritical(logDFMBase) << "LocalFileHandler::renameFile: Failed to create DOperator for:" << url;
        return false;
    }

    bool success = oper->renameFile(newUrl);
    if (!success) {
        qCWarning(logDFMBase) << "LocalFileHandler::renameFile: Failed to rename file:" << url
                              << "to:" << newUrl << "Error:" << oper->lastError().errorMsg();
        d->setError(oper->lastError());
        return false;
    }

    FileInfoPointer fileInfo = InfoFactory::create<FileInfo>(newUrl);
    fileInfo->refresh();

    FileUtils::notifyFileChangeManual(DFMGLOBAL_NAMESPACE::FileNotifyType::kFileDeleted, url);
    FileUtils::notifyFileChangeManual(DFMGLOBAL_NAMESPACE::FileNotifyType::kFileAdded, newUrl);

    qCInfo(logDFMBase) << "LocalFileHandler::renameFile: Successfully renamed file using dfmio:"
                       << url << "to:" << newUrl;

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
    if (fileUrls.isEmpty()) {
        qCDebug(logDFMBase) << "LocalFileHandler::openFiles: No files to open, returning success";
        return true;
    }

    qCDebug(logDFMBase) << "LocalFileHandler::openFiles: Opening" << fileUrls.size() << "files";

    QList<QUrl> pathList;
    bool result = false;
    d->invalidPath.clear();

    // Process each file
    for (const QUrl &fileUrl : fileUrls) {
        // Skip invalid URLs
        if (!fileUrl.isValid() || !fileUrl.isLocalFile()) {
            qCWarning(logDFMBase) << "LocalFileHandler::openFiles: Skipping invalid URL:" << fileUrl;
            continue;
        }

        qCDebug(logDFMBase) << "LocalFileHandler::openFiles: Processing file:" << fileUrl;

        // 1. Handle symlinks
        auto resolvedUrl = d->resolveSymlink(fileUrl);
        if (!resolvedUrl) {   // If resolution fails, continue to next file
            qCDebug(logDFMBase) << "LocalFileHandler::openFiles: Failed to resolve symlink for:" << fileUrl;
            continue;
        }

        // 2. Handle executable files
        if (d->handleExecutableFile(*resolvedUrl, &result)) {
            qCDebug(logDFMBase) << "LocalFileHandler::openFiles: Handled as executable file:" << *resolvedUrl;
            continue;
        }

        // 3. Collect file paths to open
        d->collectFilePath(*resolvedUrl, &pathList);
    }

    // 4. Open collected files
    if (!pathList.empty()) {
        qCDebug(logDFMBase) << "LocalFileHandler::openFiles: Opening" << pathList.size() << "collected files";
        result = d->doOpenFiles(pathList);
        if (result) {
            qCInfo(logDFMBase) << "LocalFileHandler::openFiles: Successfully opened" << pathList.size() << "files";
        } else {
            qCWarning(logDFMBase) << "LocalFileHandler::openFiles: Failed to open some or all files";
        }
    } else {
        result = d->invalidPath.isEmpty();
        if (!result) {
            qCWarning(logDFMBase) << "LocalFileHandler::openFiles: No valid files to open, invalid paths:" << d->invalidPath.size();
        }
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
        qCWarning(logDFMBase) << "LocalFileHandler::openFilesByApp: Desktop file path is empty";
        return ok;
    }

    if (fileUrls.isEmpty()) {
        qCWarning(logDFMBase) << "LocalFileHandler::openFilesByApp: No files provided to open";
        return ok;
    }

    qCDebug(logDFMBase) << "LocalFileHandler::openFilesByApp: Opening" << fileUrls.size()
                        << "files with app:" << desktopFile;

    GDesktopAppInfo *appInfo = g_desktop_app_info_new_from_filename(desktopFile.toLocal8Bit().constData());
    if (!appInfo) {
        qCWarning(logDFMBase) << "LocalFileHandler::openFilesByApp: Failed to create GDesktopAppInfo from:"
                              << desktopFile << "Check if file exists and PATH is correct";
        return false;
    }

    QStringList filePathsStr;
    for (const auto &url : fileUrls) {
        filePathsStr << url.toString();
    }

    QString terminalFlag = QString(g_desktop_app_info_get_string(appInfo, "Terminal"));
    if (terminalFlag == "true") {
        qCDebug(logDFMBase) << "LocalFileHandler::openFilesByApp: Running terminal application:" << desktopFile;
        QString exec = QString(g_desktop_app_info_get_string(appInfo, "Exec"));
        QStringList args;
        args << "-e" << exec.split(" ").at(0) << filePathsStr;
        QString termPath = defaultTerminalPath();
        qCDebug(logDFMBase) << "LocalFileHandler::openFilesByApp: Terminal command:" << termPath << args;
        ok = QProcess::startDetached(termPath, args);
    } else {
        qCDebug(logDFMBase) << "LocalFileHandler::openFilesByApp: Launching GUI application:" << desktopFile;
        ok = d->launchApp(desktopFile, filePathsStr);
    }
    g_object_unref(appInfo);

    if (ok) {
        qCInfo(logDFMBase) << "LocalFileHandler::openFilesByApp: Successfully opened files with app:"
                           << desktopFile << "Files count:" << fileUrls.size();
        // workaround since DTK apps doesn't support the recent file spec.
        // spec: https://www.freedesktop.org/wiki/Specifications/desktop-bookmark-spec/
        // the correct approach: let the app add it to the recent list.
        // addToRecentFile(DUrl::fromLocalFile(filePath), mimetype);
        QString mimetype = d->getFileMimetype(fileUrls.first());
        d->addRecentFile(desktopFile, fileUrls, mimetype);
    } else {
        qCWarning(logDFMBase) << "LocalFileHandler::openFilesByApp: Failed to open files with app:"
                              << desktopFile;
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
    qCDebug(logDFMBase) << "LocalFileHandler::createSystemLink: Creating system link from:" << sourcefile << "to:" << link;

    QSharedPointer<DFMIO::DOperator> oper { new DFMIO::DOperator(sourcefile) };

    if (!oper) {
        qCCritical(logDFMBase) << "LocalFileHandler::createSystemLink: Failed to create DOperator for:" << sourcefile;
        return false;
    }

    bool success = oper->createLink(link);
    if (!success) {
        qCWarning(logDFMBase) << "LocalFileHandler::createSystemLink: Failed to create link from:" << sourcefile
                              << "to:" << link << "Error:" << oper->lastError().errorMsg();
        d->setError(oper->lastError());
        return false;
    }

    qCInfo(logDFMBase) << "LocalFileHandler::createSystemLink: Successfully created system link from:"
                       << sourcefile << "to:" << link;
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
    qCDebug(logDFMBase) << "LocalFileHandler::setPermissions: Setting permissions for:" << url
                        << "Permissions:" << QString::number(static_cast<uint16_t>(permissions), 8);

    QSharedPointer<DFMIO::DFile> dfile { new DFMIO::DFile(url) };
    if (!dfile) {
        qCCritical(logDFMBase) << "LocalFileHandler::setPermissions: Failed to create DFile for:" << url;
        return false;
    }

    // if the `permissions` is invalid, do not set permissions
    // eg. bug-199607: Copy MTP folder to local, folder permissions wrong
    // reason: `dfm-io` uses gio to query the `unix::mode` field to get file permissions, but this field is not available in MTP file
    if (0 == permissions) {
        qCDebug(logDFMBase) << "LocalFileHandler::setPermissions: Skipping permission setting for:" << url
                            << "Permissions value is 0 (likely from MTP or unsupported filesystem)";
        return true;
    }

    bool success = dfile->setPermissions(DFMIO::DFile::Permissions(uint16_t(permissions)));
    if (!success) {
        qCWarning(logDFMBase) << "LocalFileHandler::setPermissions: Failed to set permissions for:" << url
                              << "Permissions:" << QString::number(static_cast<uint16_t>(permissions), 8)
                              << "Error:" << dfile->lastError().errorMsg();
        d->setError(dfile->lastError());
        return false;
    }

    qCDebug(logDFMBase) << "LocalFileHandler::setPermissions: Successfully set permissions for:" << url
                        << "Permissions:" << QString::number(static_cast<uint16_t>(permissions), 8);
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
    qCDebug(logDFMBase) << "LocalFileHandler::deleteFile: Deleting file:" << url;

    // Fast path: For local files, try unlink directly to avoid DOperator overhead
    if (url.isLocalFile()) {
        const QByteArray localPath = url.toLocalFile().toUtf8();
        const char *path = localPath.constData();

        if (::unlink(path) == 0) {
            // Successfully deleted file or symlink
            qCDebug(logDFMBase) << "LocalFileHandler::deleteFile: Successfully deleted (fast path):" << url;
            return true;
        }

        // If it's a directory, try rmdir (for empty directories)
        if (errno == EISDIR && ::rmdir(path) == 0) {
            qCDebug(logDFMBase) << "LocalFileHandler::deleteFile: Successfully deleted directory (fast path):" << url;
            return true;
        }

        // Fast path failed, log and fall through to DOperator
        qCDebug(logDFMBase) << "LocalFileHandler::deleteFile: Fast path failed (errno:" << errno
                            << strerror(errno) << "), using DOperator for:" << url;
    } else {
        // Non-file scheme (e.g., trash://) or non-local file, use DOperator directly
        qCDebug(logDFMBase) << "LocalFileHandler::deleteFile: Non-local file scheme:" << url.scheme()
                            << ", using DOperator for:" << url;
    }

    // Slow path: Use DOperator for special schemes, errors, or detailed error information
    QSharedPointer<DFMIO::DOperator> dOperator { new DFMIO::DOperator(url) };

    if (!dOperator) {
        qCCritical(logDFMBase) << "LocalFileHandler::deleteFile: Failed to create DOperator for:" << url;
        return false;
    }

    bool success = dOperator->deleteFile();
    if (!success) {
        qCWarning(logDFMBase) << "LocalFileHandler::deleteFile: Failed to delete file:" << url
                              << "Error:" << dOperator->lastError().errorMsg();
        d->setError(dOperator->lastError());
        return false;
    }

    return true;
}

bool LocalFileHandler::deleteFileRecursive(const QUrl &url)
{
    qCInfo(logDFMBase) << "LocalFileHandler::deleteFileRecursive: Starting recursive deletion of:" << url;

    if (!url.isValid()) {
        qCWarning(logDFMBase) << "LocalFileHandler::deleteFileRecursive: Invalid URL provided:" << url;
        return false;
    }

    if (SystemPathUtil::instance()->isSystemPath(url.toLocalFile())) {
        qCCritical(logDFMBase) << "LocalFileHandler::deleteFileRecursive: CRITICAL - Attempted to delete system path:"
                               << url << "Operation aborted for safety";
        abort();
    }

    FileInfoPointer info { InfoFactory::create<FileInfo>(url) };
    if (!info) {
        qCWarning(logDFMBase) << "LocalFileHandler::deleteFileRecursive: Failed to create FileInfo for:" << url;
        return false;
    }

    // 首先检查是否是符号链接，如果是则只删除链接本身
    if (info->isAttributes(OptInfoType::kIsSymLink)) {
        qCDebug(logDFMBase) << "LocalFileHandler::deleteFileRecursive: Deleting symbolic link:" << url;
        return deleteFile(url);
    }

    if (!info->isAttributes(OptInfoType::kIsDir)) {
        qCDebug(logDFMBase) << "LocalFileHandler::deleteFileRecursive: Deleting regular file:" << url;
        return deleteFile(url);
    }

    qCDebug(logDFMBase) << "LocalFileHandler::deleteFileRecursive: Processing directory contents:" << url;

    QSharedPointer<DFMIO::DEnumerator> enumerator { new DFMIO::DEnumerator(url) };
    if (!enumerator) {
        qCWarning(logDFMBase) << "LocalFileHandler::deleteFileRecursive: Failed to create enumerator for:" << url;
        return false;
    }

    bool succ { true };
    int deletedCount = 0;
    while (enumerator->hasNext()) {
        const QUrl &urlNext = enumerator->next();
        info = InfoFactory::create<FileInfo>(urlNext);
        if (!info) {
            qCWarning(logDFMBase) << "LocalFileHandler::deleteFileRecursive: Failed to create FileInfo for:" << urlNext;
            succ = false;
            continue;
        }

        bool itemSuccess = false;
        if (info->isAttributes(OptInfoType::kIsDir)) {
            itemSuccess = deleteFileRecursive(urlNext);
        } else {
            itemSuccess = deleteFile(urlNext);
        }

        if (itemSuccess) {
            deletedCount++;
        } else {
            succ = false;
        }
    }

    qCDebug(logDFMBase) << "LocalFileHandler::deleteFileRecursive: Deleted" << deletedCount
                        << "items from directory:" << url;

    bool dirDeleted = deleteFile(url);
    if (dirDeleted) {
        qCInfo(logDFMBase) << "LocalFileHandler::deleteFileRecursive: Successfully completed recursive deletion of:" << url;
    } else {
        qCWarning(logDFMBase) << "LocalFileHandler::deleteFileRecursive: Failed to delete directory after clearing contents:" << url;
    }

    return succ && dirDeleted;
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
    return AppLaunchUtils::instance().launchApp(desktopFilePath, newFileUrls);
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
    if (!DFMIO::DFile(path).exists() && !ProtocolUtils::isSMBFile(url))
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
        if (!info)
            return false;
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

    return info->isAttributes(FileInfo::FileIsType::kIsExecutable)
            && info->isAttributes(FileInfo::FileIsType::kIsReadable);
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
        if (!url.isValid()) {
            qCWarning(logDFMBase) << "Invalid URL skipped:" << url;
            transUrls.removeOne(url);
            continue;
        }

        // Do not check the file exists, as it may affect the SMB connection mounting
        FileInfoPointer info = InfoFactory::create<FileInfo>(url);
        if (!info) {
            qCWarning(logDFMBase) << "Failed to create FileInfo for:" << url;
            transUrls.removeOne(url);
            continue;
        }

        if (info->nameOf(NameInfoType::kSuffix) == Global::Scheme::kDesktop) {
            ret = launchApp(url.path()) || ret;   // Success if at least one succeeds
            transUrls.removeOne(url);
            continue;
        }
    }
    if (transUrls.isEmpty())
        return ret;

    // Find information for all openable files, this is time-consuming and needs optimization
    QMultiMap<QString, QString> openInfos, mountOpenInfos, cmdOpenInfos;
    QMap<QString, QString> openMineTypes, mountMineTypes, cmdMineTypes;
    for (auto url : transUrls) {
        const QUrl &fileUrl = url;
        const QString &filePath = fileUrl.path();

        FileInfoPointer fileInfo = InfoFactory::create<FileInfo>(fileUrl);
        if (!fileInfo) {
            qCWarning(logDFMBase) << "Failed to create FileInfo for:" << fileUrl;
            continue;
        }

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
                    qCWarning(logDFMBase) << "No default application for" << fileUrl;
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
                    qCWarning(logDFMBase) << "No default application for" << transUrls;
                    continue;
                }
            }

            openInfos.insert(defaultDesktopFile, url.toString());
            openMineTypes.insert(url.toString(), mimeType);
        }
    }

    // Open all files
    if (openInfos.isEmpty() && mountOpenInfos.isEmpty() && cmdOpenInfos.isEmpty())
        return false;

    bool openResult = doOpenFiles(openInfos, openMineTypes);
    bool openMount = doOpenFiles(mountOpenInfos, mountMineTypes);
    bool openCmd = doOpenFiles(cmdOpenInfos, cmdMineTypes);

    if (openResult || openMount || openCmd)
        return true;

    if (!transUrls.isEmpty() && ProtocolUtils::isSMBFile(transUrls[0]))
        return false;

    QStringList paths;
    for (const QUrl &url : transUrls) {
        paths << url.path();
    }

    if (!transUrls.isEmpty() && MimesAppsManager::getDefaultAppByFileName(transUrls.first().path()) == "org.gnome.font-viewer.desktop") {
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
            result = QDesktopServices::openUrl(url) || result;   // 有一个成功就成功
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

        /// ###: just cache files that rename successfully.
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

// 处理符号链接解析
std::optional<QUrl> LocalFileHandlerPrivate::resolveSymlink(const QUrl &url)
{
    if (!url.isValid()) {
        qCWarning(logDFMBase) << "Invalid URL provided to resolveSymlink:" << url;
        return std::nullopt;
    }

    // Track visited paths to detect circular links
    QStringList visitedPaths;
    QString currentPath = url.toLocalFile();
    visitedPaths << currentPath;

    QFileInfo fileInfo(currentPath);
    while (fileInfo.isSymLink()) {
        // Get absolute path of link target (automatically handles relative paths)
        QString canonicalPath = fileInfo.canonicalFilePath();

        // If canonical path retrieval fails, link may be broken
        if (canonicalPath.isEmpty()) {
            FileInfoPointer fileInfo = InfoFactory::create<FileInfo>(url);
            if (!fileInfo) {
                qCWarning(logDFMBase) << "Failed to create FileInfo for:" << url;
                return std::nullopt;
            }

            QString targetLink = fileInfo->pathOf(PathInfoType::kSymLinkTarget);
            canonicalPath = SystemPathUtil::instance()->getRealpathSafely(targetLink);
        }

        if (canonicalPath.isEmpty()) {
            DialogManagerInstance->showErrorDialog(QObject::tr("Unable to find the original file"), QString());
            return std::nullopt;
        }

        // Check for circular links
        if (visitedPaths.contains(canonicalPath))
            break;   // Circular link detected, use last valid path

        visitedPaths << canonicalPath;

        // Network file check
        if (NetworkUtils::instance()->checkFtpOrSmbBusy(QUrl::fromLocalFile(canonicalPath))) {
            DialogManager::instance()->showUnableToVistDir(canonicalPath);
            return std::nullopt;
        }

        // Check if link target exists
        fileInfo.setFile(canonicalPath);
        if (!fileInfo.exists() && !ProtocolUtils::isSMBFile(QUrl::fromLocalFile(canonicalPath))) {
            // Link is broken
            lastEvent = DialogManagerInstance->showBreakSymlinkDialog(
                    QFileInfo(currentPath).fileName(),
                    url);
            invalidPath << url;
            return std::nullopt;
        }

        currentPath = canonicalPath;
    }

    // Return final resolved URL (if circular link, returns last valid path)
    return QUrl::fromLocalFile(currentPath);
}

// 处理可执行文件
bool LocalFileHandlerPrivate::handleExecutableFile(const QUrl &fileUrl, bool *result)
{
    if (!fileUrl.isValid() || !result) {
        qCWarning(logDFMBase) << "Invalid parameters to handleExecutableFile";
        return false;
    }

    // Handle executable scripts
    if (isExecutableScript(fileUrl.path())) {
        int code = DialogManagerInstance->showRunExcutableScriptDialog(fileUrl);
        *result = openExcutableScriptFile(fileUrl.path(), code) || *result;
        return true;
    }

    // Handle runnable files
    if (isFileRunnable(fileUrl.path()) && !FileUtils::isDesktopFile(fileUrl)) {
        int code = DialogManagerInstance->showRunExcutableFileDialog(fileUrl);
        *result = openExcutableFile(fileUrl.path(), code) || *result;
        return true;
    }

    // Handle files that need executable permission
    if (shouldAskUserToAddExecutableFlag(fileUrl.path()) && !FileUtils::isDesktopFile(fileUrl)) {
        int code = DialogManagerInstance->showAskIfAddExcutableFlagAndRunDialog();
        *result = addExecutableFlagAndExecuse(fileUrl.path(), code) || *result;
        return true;
    }

    return false;
}

// Collect file paths to open
void LocalFileHandlerPrivate::collectFilePath(const QUrl &fileUrl, QList<QUrl> *pathList)
{
    if (!fileUrl.isValid() || !pathList) {
        qCWarning(logDFMBase) << "Invalid parameters to collectFilePath";
        return;
    }

    QString urlPath = fileUrl.path();
    if (isFileWindowsUrlShortcut(urlPath)) {
        urlPath = getInternetShortcutUrl(urlPath);
        if (!urlPath.isEmpty()) {
            pathList->append(QUrl::fromLocalFile(urlPath));
        }
    } else {
        pathList->append(fileUrl);
    }
}
