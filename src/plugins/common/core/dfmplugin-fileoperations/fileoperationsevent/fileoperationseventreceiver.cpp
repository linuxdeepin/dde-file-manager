/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#include "fileoperationseventreceiver.h"
#include "fileoperations/filecopymovejob.h"
#include "fileoperationsevent/fileoperationseventhandler.h"

#include "dfm-base/utils/hidefilehelper.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/file/local/localfilehandler.h"
#include "dfm-base/utils/windowutils.h"
#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/interfaces/abstractfileinfo.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/utils/decorator/decoratorfile.h"
#include "dfm-base/utils/decorator/decoratorfileinfo.h"
#include "dfm-base/utils/properties.h"

#include <dfm-io/dfmio_utils.h>

#include <QDebug>

Q_DECLARE_METATYPE(QList<QUrl> *)
Q_DECLARE_METATYPE(bool *)
Q_DECLARE_METATYPE(QString *)
Q_DECLARE_METATYPE(QFileDevice::Permission)

DFMGLOBAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_USE_NAMESPACE

FileOperationsEventReceiver::FileOperationsEventReceiver(QObject *parent)
    : QObject(parent), dialogManager(DialogManagerInstance)
{
    getServiceMutex.reset(new QMutex);
    functionsMutex.reset(new QMutex);
    copyMoveJob.reset(new FileCopyMoveJob);
    initDBus();
}

QString FileOperationsEventReceiver::newDocmentName(QString targetdir,
                                                    const QString suffix,
                                                    const CreateFileType fileType)
{
    QString suffixex;
    QString baseName;
    switch (fileType) {
    case CreateFileType::kCreateFileTypeFolder:
        baseName = QObject::tr("New Folder");
        break;
    case CreateFileType::kCreateFileTypeText:
        baseName = QObject::tr("Text");
        suffixex = "txt";
        break;
    case CreateFileType::kCreateFileTypeWord:
        baseName = QObject::tr("Document");
        suffixex = DFMBASE_NAMESPACE::WindowUtils::isWayLand() ? "wps" : "doc";
        break;
    case CreateFileType::kCreateFileTypeExcel:
        baseName = QObject::tr("Spreadsheet");
        suffixex = DFMBASE_NAMESPACE::WindowUtils::isWayLand() ? "et" : "xls";
        break;
    case CreateFileType::kCreateFileTypePowerpoint:
        baseName = QObject::tr("Presentation");
        suffixex = DFMBASE_NAMESPACE::WindowUtils::isWayLand() ? "dps" : "ppt";
        break;
    case CreateFileType::kCreateFileTypeDefault:
        baseName = QObject::tr("New File");
        suffixex = suffix;
        break;
    default:
        return QString();
    }
    return newDocmentName(targetdir, baseName, suffixex);
}

QString FileOperationsEventReceiver::newDocmentName(QString targetdir, const QString &baseName, const QString &suffix)
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
}

void FileOperationsEventReceiver::initDBus()
{
    qInfo() << "Start initilize dbus: `OperationsStackManagerInterface`";
    // Note: the plugin depends on `dde-file-manager-server`!
    // the plugin will not work if `dde-file-manager-server` not run.
    static const QString OperationsStackService = "com.deepin.filemanager.service";
    static const QString OperationsStackPath = "/com/deepin/filemanager/service/OperationsStackManager";

    operationsStackDbus.reset(new OperationsStackManagerInterface(OperationsStackService,
                                                                  OperationsStackPath,
                                                                  QDBusConnection::sessionBus(),
                                                                  this));
    qInfo() << "Finish initilize dbus: `OperationsStackManagerInterface`";
}

bool FileOperationsEventReceiver::revocation(const quint64 windowId, const QVariantMap &ret,
                                             DFMGLOBAL_NAMESPACE::OperatorHandleCallback handle)
{
    if (!ret.contains("event") || !ret.contains("sources") || !ret.contains("targets"))
        return false;
    GlobalEventType eventType = static_cast<GlobalEventType>(ret.value("event").value<uint16_t>());
    QList<QUrl> sources = QUrl::fromStringList(ret.value("sources").toStringList());
    QList<QUrl> targets = QUrl::fromStringList(ret.value("targets").toStringList());
    for (const auto &url : sources) {
        AbstractFileInfoPointer fileInfo = InfoFactory::create<AbstractFileInfo>(url);
        if (!DecoratorFile(url).exists()) {
            // Their sizes are equal, indicating that the current operation is many-to-many.
            // So files that do not exist in sources need to be deleted in targets as well
            if (targets.size() == sources.size()) {
                int index = sources.indexOf(url);
                targets.removeAt(index);
            }
            sources.removeOne(url);
        }
    }

    if (sources.isEmpty())
        return true;

    switch (eventType) {
    case kCutFile:
        if (targets.isEmpty())
            return true;
        handleOperationCut(windowId, sources, targets.first(), AbstractJobHandler::JobFlag::kRevocation, handle);
        break;
    case kDeleteFiles:
        handleOperationDeletes(windowId, sources, AbstractJobHandler::JobFlag::kRevocation, handle);
        break;
    case kMoveToTrash:
        handleOperationMoveToTrash(windowId, sources, AbstractJobHandler::JobFlag::kRevocation, handle);
        break;
    case kRestoreFromTrash:
        handleOperationRestoreFromTrash(windowId, sources, AbstractJobHandler::JobFlag::kRevocation, handle);
        break;
    case kRenameFile:
        if (targets.isEmpty())
            return true;
        handleOperationRenameFile(windowId, sources.first(), targets.first(), AbstractJobHandler::JobFlag::kRevocation);
        break;
    case kRenameFiles:
        if (targets.isEmpty())
            return true;

        for (int i = 0; i < sources.size(); ++i) {
            handleOperationRenameFile(windowId, sources[i], targets[i], AbstractJobHandler::JobFlag::kRevocation);
        }
        break;
    default:
        return false;
    }

    return true;
}

bool FileOperationsEventReceiver::doRenameFiles(const quint64 windowId, const QList<QUrl> urls, const QPair<QString, QString> pair,
                                                const QPair<QString, DFMBASE_NAMESPACE::AbstractJobHandler::FileNameAddFlag> pair2,
                                                const RenameTypes type, QMap<QUrl, QUrl> &successUrls, QString &errorMsg,
                                                const QVariant custom, DFMBASE_NAMESPACE::Global::OperatorCallback callback)
{
    DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
    bool ok = false;
    switch (type) {
    case RenameTypes::kBatchRepalce: {
        QMap<QUrl, QUrl> needDealUrls = FileUtils::fileBatchReplaceText(urls, pair);
        if (callback) {
            CallbackArgus args(new QMap<CallbackKey, QVariant>);
            args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
            args->insert(CallbackKey::kSourceUrls, QVariant::fromValue(QList<QUrl>() << needDealUrls.keys()));
            args->insert(CallbackKey::kTargets, QVariant::fromValue(QList<QUrl>() << needDealUrls.values()));
            args->insert(CallbackKey::kCustom, custom);
            callback(args);
        }
        ok = fileHandler.renameFilesBatch(needDealUrls, successUrls);
        break;
    }
    case RenameTypes::kBatchCustom: {
        QMap<QUrl, QUrl> needDealUrls = FileUtils::fileBatchCustomText(urls, pair);
        if (callback) {
            CallbackArgus args(new QMap<CallbackKey, QVariant>);
            args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
            args->insert(CallbackKey::kSourceUrls, QVariant::fromValue(QList<QUrl>() << needDealUrls.keys()));
            args->insert(CallbackKey::kTargets, QVariant::fromValue(QList<QUrl>() << needDealUrls.values()));
            args->insert(CallbackKey::kCustom, custom);
            callback(args);
        }
        ok = fileHandler.renameFilesBatch(needDealUrls, successUrls);
        break;
    }
    case RenameTypes::kBatchAppend: {
        QMap<QUrl, QUrl> needDealUrls = FileUtils::fileBatchAddText(urls, pair2);
        if (callback) {
            CallbackArgus args(new QMap<CallbackKey, QVariant>);
            args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
            args->insert(CallbackKey::kSourceUrls, QVariant::fromValue(QList<QUrl>() << needDealUrls.keys()));
            args->insert(CallbackKey::kTargets, QVariant::fromValue(QList<QUrl>() << needDealUrls.values()));
            args->insert(CallbackKey::kCustom, custom);
            callback(args);
        }
        ok = fileHandler.renameFilesBatch(needDealUrls, successUrls);
        break;
    }
    }
    if (!ok) {
        errorMsg = fileHandler.errorString();
        DialogManagerInstance->showErrorDialog("Rename file error: %1", errorMsg);
    }
    return ok;
}

bool FileOperationsEventReceiver::doRenameDesktopFile(const quint64 windowId, const QUrl oldUrl, const QUrl newUrl, const dfmbase::AbstractJobHandler::JobFlags flags)
{
    const QString &desktopPath = oldUrl.toLocalFile();
    Properties desktop(desktopPath, "Desktop Entry");
    static const QString kLocale = QLocale::system().name();
    static const QString kLocaleNameTemplate = QString("Name[%1]");

    auto localeName = kLocaleNameTemplate.arg(kLocale);

    QString key;   // to find the present displaying Name
    if (desktop.contains(localeName)) {
        key = localeName;
    } else {
        auto splittedLocale = kLocale.trimmed().split("_");
        if (splittedLocale.isEmpty()) {
            key = "Name";
        } else {
            localeName = kLocaleNameTemplate.arg(splittedLocale.first());
            key = desktop.contains(localeName) ? localeName : "Name";
        }
    }

    AbstractFileInfoPointer newFileInfo = InfoFactory::create<AbstractFileInfo>(newUrl);
    AbstractFileInfoPointer oldFileInfo = InfoFactory::create<AbstractFileInfo>(oldUrl);
    const QString &newName = newFileInfo->fileDisplayName();
    const QString &oldName = oldFileInfo->fileDisplayName();
    if (newName == oldName)
        return true;

    desktop.set(key, newFileInfo->fileDisplayName());
    desktop.set("X-Deepin-Vendor", QStringLiteral("user-custom"));
    if (desktop.save(desktopPath, "Desktop Entry")) {
        QMap<QUrl, QUrl> renamed { { oldUrl, newUrl } };
        dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kRenameFileResult,
                                     windowId, renamed, true, "");

        if (!flags.testFlag(AbstractJobHandler::JobFlag::kRevocation)) {
            const QString path = QFileInfo(desktopPath).absoluteDir().absoluteFilePath(oldName);
            saveFileOperation({ oldUrl }, { QUrl::fromLocalFile(path) }, GlobalEventType::kRenameFile);
        }
        return true;
    }

    return false;
}

JobHandlePointer FileOperationsEventReceiver::doMoveToTrash(const quint64 windowId, const QList<QUrl> sources, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                            DFMGLOBAL_NAMESPACE::OperatorHandleCallback handleCallback)
{
    Q_UNUSED(windowId);

    if (sources.isEmpty())
        return nullptr;

    const QUrl &sourceFirst = sources.first();

    if (!sourceFirst.isLocalFile()) {
        if (dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_MoveToTrash", windowId, sources, flags)) {
            return nullptr;
        }
    }

    // check url permission
    QList<QUrl> urlsCanTrash = sources;
    auto it = urlsCanTrash.begin();
    while (it != urlsCanTrash.end()) {
        auto info = InfoFactory::create<AbstractFileInfo>(*it);
        if (!info || !info->canRename() || (!info->isWritable() && !info->isFile() && !info->isSymLink()))
            it = urlsCanTrash.erase(it);
        else
            ++it;
    }

    if (urlsCanTrash.isEmpty())
        return nullptr;

    JobHandlePointer handle = nullptr;
    if (FileUtils::isGvfsFile(sourceFirst) || DFMIO::DFMUtils::fileIsRemovable(sourceFirst)) {
        if (DialogManagerInstance->showDeleteFilesClearTrashDialog(urlsCanTrash) != QDialog::Accepted)
            return nullptr;
        handle = copyMoveJob->deletes(urlsCanTrash, flags);
    } else {
        if (!flags.testFlag(AbstractJobHandler::JobFlag::kRevocation) && Application::instance()->genericAttribute(Application::kShowDeleteConfirmDialog).toBool()) {
            if (DialogManagerInstance->showNormalDeleteConfirmDialog(urlsCanTrash) != QDialog::Accepted)
                return nullptr;
        }
        handle = copyMoveJob->moveToTrash(urlsCanTrash, flags);
    }
    if (handleCallback)
        handleCallback(handle);
    return handle;
}

JobHandlePointer FileOperationsEventReceiver::doRestoreFromTrash(const quint64 windowId, const QList<QUrl> sources, const AbstractJobHandler::JobFlags flags, OperatorHandleCallback handleCallback)
{
    if (sources.isEmpty())
        return nullptr;

    JobHandlePointer handle = copyMoveJob->restoreFromTrash(sources, flags);
    if (handleCallback)
        handleCallback(handle);
    return handle;
}

JobHandlePointer FileOperationsEventReceiver::doCopyFile(const quint64 windowId, const QList<QUrl> sources, const QUrl target,
                                                         const AbstractJobHandler::JobFlags flags, DFMGLOBAL_NAMESPACE::OperatorHandleCallback callbaskHandle)
{
    if (sources.isEmpty())
        return nullptr;

    QList<QUrl> sourcesTrans = sources;

    QList<QUrl> urls {};
    bool ok = dpfHookSequence->run("dfmplugin_utils", "hook_UrlsTransform", sourcesTrans, &urls);
    if (ok && !urls.isEmpty())
        sourcesTrans = urls;

    if (!target.isLocalFile()) {
        if (dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_CopyFile", windowId, sourcesTrans, target, flags)) {
            return nullptr;
        }
    }

    JobHandlePointer handle = copyMoveJob->copy(sourcesTrans, target, flags);
    if (callbaskHandle)
        callbaskHandle(handle);
    return handle;
}

JobHandlePointer FileOperationsEventReceiver::doCutFile(quint64 windowId, const QList<QUrl> sources, const QUrl target, const AbstractJobHandler::JobFlags flags, OperatorHandleCallback handleCallback)
{
    if (sources.isEmpty())
        return nullptr;

    QList<QUrl> sourcesTrans = sources;
    QList<QUrl> urls {};
    bool ok = dpfHookSequence->run("dfmplugin_utils", "hook_UrlsTransform", sourcesTrans, &urls);
    if (ok && !urls.isEmpty())
        sourcesTrans = urls;

    if (!target.isLocalFile()) {
        if (dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_CutFile", windowId, sourcesTrans, target, flags)) {
            return nullptr;
        }
    }

    JobHandlePointer handle = copyMoveJob->cut(sourcesTrans, target, flags);
    if (handleCallback)
        handleCallback(handle);

    return handle;
}

JobHandlePointer FileOperationsEventReceiver::doDeleteFile(const quint64 windowId, const QList<QUrl> sources, const AbstractJobHandler::JobFlags flags, OperatorHandleCallback handleCallback)
{
    if (sources.isEmpty())
        return nullptr;

    if (!sources.first().isLocalFile()) {
        // hook events
        if (dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_DeleteFile", windowId, sources, flags)) {
            return nullptr;
        }
    }

    // Delete local file with shift+delete, show a confirm dialog.
    if (!flags.testFlag(AbstractJobHandler::JobFlag::kRevocation)) {
        if (DialogManagerInstance->showDeleteFilesClearTrashDialog(sources) != QDialog::Accepted)
            return nullptr;
    }
    JobHandlePointer handle = copyMoveJob->deletes(sources, flags);
    if (handleCallback)
        handleCallback(handle);

    return handle;
}

JobHandlePointer FileOperationsEventReceiver::doCleanTrash(const quint64 windowId, const QList<QUrl> sources, const AbstractJobHandler::DeleteDialogNoticeType deleteNoticeType, OperatorHandleCallback handleCallback)
{
    //清空回收站操作弹框提示（这里只会显示Emtpy按钮）
    const bool isFileAlreadyInTrash = (deleteNoticeType == AbstractJobHandler::DeleteDialogNoticeType::kDeleteTashFiles);   //检查用户是否从回收站内部删除文件
    //在此处理从回收站内部选择文件的删除操作(若不在这里处理，会进入到handleOperationCleanTrash()中，导致弹框提示无法区分"Delete"和"Empty"按钮的显示)
    if (!sources.isEmpty()) {
        // Show clear trash dialog
        if (DialogManagerInstance->showDeleteFilesClearTrashDialog(sources, !isFileAlreadyInTrash) != QDialog::Accepted)
            return nullptr;
    }

    if (sources.isEmpty())
        return nullptr;

    if (!sources.first().isLocalFile()) {
        if (dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_CleanTrash", windowId, sources)) {
            return nullptr;
        }
    }
    JobHandlePointer handle = copyMoveJob->cleanTrash(sources);
    if (handleCallback)
        handleCallback(handle);
    return handle;
}

bool FileOperationsEventReceiver::doMkdir(const quint64 windowId, const QUrl url, QUrl &targetUrl)
{
    bool ok = false;
    QString error;
    if (!url.isLocalFile()) {
        if (dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_MakeDir", windowId, url)) {
            dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kMkdirResult,
                                         windowId, QList<QUrl>() << url, true, error);
            return true;
        }
    }

    QString newPath = newDocmentName(url.path(), QString(), CreateFileType::kCreateFileTypeFolder);
    if (newPath.isEmpty())
        return false;

    QUrl urlNew;
    urlNew.setScheme(url.scheme());
    urlNew.setPath(newPath);

    DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
    ok = fileHandler.mkdir(urlNew);
    if (!ok) {
        error = fileHandler.errorString();
        dialogManager->showErrorDialog("make dir error", error);
    }
    targetUrl = urlNew;

    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kMkdirResult,
                                 windowId, QList<QUrl>() << url, ok, error);
    saveFileOperation({ targetUrl }, {}, GlobalEventType::kDeleteFiles);
    return ok;
}

QString FileOperationsEventReceiver::doTouchFilePremature(const quint64 windowId, const QUrl url, const CreateFileType fileType, const QString suffix,
                                                          const QVariant custom, OperatorCallback callbackImmediately)
{
    if (url.isLocalFile()) {
        QString newPath = newDocmentName(url.path(), suffix, fileType);
        if (newPath.isEmpty())
            return newPath;

        QUrl urlNew;
        urlNew.setScheme(url.scheme());
        urlNew.setPath(newPath);

        if (callbackImmediately) {
            CallbackArgus args(new QMap<CallbackKey, QVariant>);
            args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
            args->insert(CallbackKey::kSourceUrls, QVariant::fromValue(QList<QUrl>() << url));
            args->insert(CallbackKey::kTargets, QVariant::fromValue(QList<QUrl>() << QUrl::fromLocalFile(newPath)));
            args->insert(CallbackKey::kCustom, custom);
            callbackImmediately(args);
        }

        return doTouchFilePractically(windowId, urlNew) ? newPath : QString();
    } else {
        QString error;
        if (dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_TouchFile", windowId, url, fileType, &error)) {
            dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kTouchFileResult,
                                         windowId, QList<QUrl>() << url, true, error);
            return url.path();
        }

        return doTouchFilePractically(windowId, url) ? url.path() : QString();
    }
}

void FileOperationsEventReceiver::saveFileOperation(const QList<QUrl> &sourcesUrls, const QList<QUrl> &targetUrls, GlobalEventType type)
{
    // save operation by dbus
    QVariantMap values;
    values.insert("event", QVariant::fromValue(static_cast<uint16_t>(type)));
    values.insert("sources", QUrl::toStringList(sourcesUrls));
    values.insert("targets", QUrl::toStringList(targetUrls));
    dpfSignalDispatcher->publish(GlobalEventType::kSaveOperator, values);
}

QUrl FileOperationsEventReceiver::checkTargetUrl(const QUrl &url)
{
    const QUrl &urlParent = DFMIO::DFMUtils::directParentUrl(url);
    if (!urlParent.isValid())
        return url;

    const QString &nameValid = FileUtils::nonExistSymlinkFileName(url, urlParent);
    if (!nameValid.isEmpty())
        return urlParent.toString() + QDir::separator() + nameValid;

    return url;
}

FileOperationsEventReceiver *FileOperationsEventReceiver::instance()
{
    static FileOperationsEventReceiver receiver;
    return &receiver;
}

void FileOperationsEventReceiver::handleOperationCopy(const quint64 windowId,
                                                      const QList<QUrl> sources,
                                                      const QUrl target,
                                                      const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                      DFMGLOBAL_NAMESPACE::OperatorHandleCallback callbaskHandle)
{
    auto handle = doCopyFile(windowId, sources, target, flags, callbaskHandle);
    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kCopyType, handle);
}

void FileOperationsEventReceiver::handleOperationCut(quint64 windowId, const QList<QUrl> sources, const QUrl target, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                     DFMGLOBAL_NAMESPACE::OperatorHandleCallback handleCallback)
{
    auto handle = doCutFile(windowId, sources, target, flags, handleCallback);
    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kCutType, handle);
}

void FileOperationsEventReceiver::handleOperationMoveToTrash(const quint64 windowId, const QList<QUrl> sources, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                             DFMGLOBAL_NAMESPACE::OperatorHandleCallback handleCallback)
{
    auto handle = doMoveToTrash(windowId, sources, flags, handleCallback);
    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kMoveToTrashType, handle);
}

void FileOperationsEventReceiver::handleOperationRestoreFromTrash(const quint64 windowId, const QList<QUrl> sources, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                                  DFMGLOBAL_NAMESPACE::OperatorHandleCallback handleCallback)
{
    auto handle = doRestoreFromTrash(windowId, sources, flags, handleCallback);
    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kRestoreType, handle);
}

void FileOperationsEventReceiver::handleOperationDeletes(const quint64 windowId,
                                                         const QList<QUrl> sources,
                                                         const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                         DFMGLOBAL_NAMESPACE::OperatorHandleCallback handleCallback)
{
    auto handle = doDeleteFile(windowId, sources, flags, handleCallback);
    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kDeleteTpye, handle);
}

void FileOperationsEventReceiver::handleOperationCleanTrash(const quint64 windowId, const QList<QUrl> sources, const AbstractJobHandler::DeleteDialogNoticeType deleteNoticeType,
                                                            DFMGLOBAL_NAMESPACE::OperatorHandleCallback handleCallback)
{
    auto handle = doCleanTrash(windowId, sources, deleteNoticeType, handleCallback);
    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kCleanTrashType, handle);
}

void FileOperationsEventReceiver::handleOperationCopy(const quint64 windowId,
                                                      const QList<QUrl> sources,
                                                      const QUrl target,
                                                      const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                      DFMGLOBAL_NAMESPACE::OperatorHandleCallback handleCallback,
                                                      const QVariant custom,
                                                      DFMGLOBAL_NAMESPACE::OperatorCallback callback)
{
    JobHandlePointer handle = doCopyFile(windowId, sources, target, flags, handleCallback);

    if (callback) {
        CallbackArgus args(new QMap<CallbackKey, QVariant>);
        args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(CallbackKey::kJobHandle, QVariant::fromValue(handle));
        args->insert(CallbackKey::kCustom, custom);
        callback(args);
    }

    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kCopyType, handle);
}

void FileOperationsEventReceiver::handleOperationCut(const quint64 windowId,
                                                     const QList<QUrl> sources,
                                                     const QUrl target,
                                                     const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                     DFMGLOBAL_NAMESPACE::OperatorHandleCallback handleCallback,
                                                     const QVariant custom,
                                                     DFMBASE_NAMESPACE::Global::OperatorCallback callback)
{
    JobHandlePointer handle = doCutFile(windowId, sources, target, flags, handleCallback);
    if (callback) {
        CallbackArgus args(new QMap<CallbackKey, QVariant>);
        args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(CallbackKey::kJobHandle, QVariant::fromValue(handle));
        args->insert(CallbackKey::kCustom, custom);
        callback(args);
    }

    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kCutType, handle);
}

void FileOperationsEventReceiver::handleOperationMoveToTrash(const quint64 windowId,
                                                             const QList<QUrl> sources,
                                                             const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                             DFMGLOBAL_NAMESPACE::OperatorHandleCallback handleCallback,
                                                             const QVariant custom,
                                                             DFMBASE_NAMESPACE::Global::OperatorCallback callback)
{

    JobHandlePointer handle = doMoveToTrash(windowId, sources, flags, handleCallback);
    if (callback) {
        CallbackArgus args(new QMap<CallbackKey, QVariant>);
        args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(CallbackKey::kJobHandle, QVariant::fromValue(handle));
        args->insert(CallbackKey::kCustom, custom);
        callback(args);
    }
    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kMoveToTrashType, handle);
}

void FileOperationsEventReceiver::handleOperationRestoreFromTrash(const quint64 windowId,
                                                                  const QList<QUrl> sources,
                                                                  const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                                  DFMGLOBAL_NAMESPACE::OperatorHandleCallback handleCallback,
                                                                  const QVariant custom,
                                                                  DFMBASE_NAMESPACE::Global::OperatorCallback callback)
{
    JobHandlePointer handle = doRestoreFromTrash(windowId, sources, flags, handleCallback);
    if (callback) {
        CallbackArgus args(new QMap<CallbackKey, QVariant>);
        args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(CallbackKey::kJobHandle, QVariant::fromValue(handle));
        args->insert(CallbackKey::kCustom, custom);
        callback(args);
    }
    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kRestoreType, handle);
}

void FileOperationsEventReceiver::handleOperationDeletes(const quint64 windowId,
                                                         const QList<QUrl> sources,
                                                         const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                         DFMGLOBAL_NAMESPACE::OperatorHandleCallback handleCallback,
                                                         const QVariant custom,
                                                         DFMBASE_NAMESPACE::Global::OperatorCallback callback)
{
    JobHandlePointer handle = doDeleteFile(windowId, sources, flags, handleCallback);
    if (callback) {
        CallbackArgus args(new QMap<CallbackKey, QVariant>);
        args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(CallbackKey::kJobHandle, QVariant::fromValue(handle));
        args->insert(CallbackKey::kCustom, custom);
        callback(args);
    }
    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kDeleteTpye, handle);
}

void FileOperationsEventReceiver::handleOperationCleanTrash(const quint64 windowId, const QList<QUrl> sources,
                                                            DFMGLOBAL_NAMESPACE::OperatorHandleCallback handleCallback,
                                                            const QVariant custom, OperatorCallback callback)
{
    JobHandlePointer handle = doCleanTrash(windowId, sources, AbstractJobHandler::DeleteDialogNoticeType::kEmptyTrash, handleCallback);
    if (callback) {
        CallbackArgus args(new QMap<CallbackKey, QVariant>);
        args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(CallbackKey::kJobHandle, QVariant::fromValue(handle));
        args->insert(CallbackKey::kCustom, custom);
        callback(args);
    }
    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kCleanTrashType, handle);
}

bool FileOperationsEventReceiver::handleOperationOpenFiles(const quint64 windowId, const QList<QUrl> urls)
{
    if (urls.isEmpty())
        return false;

    bool ok = false;
    QString error;
    if (!urls.isEmpty() && !urls.first().isLocalFile()) {
        // hook events
        if (dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_OpenFileInPlugin", windowId, urls)) {
            dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenFilesResult, windowId, urls, true, error);
            return true;
        }
    }

    if (dpfHookSequence->run("dfmplugin_fileoperations", "hook_OpenLocalFiles", windowId, urls))
        return true;

    DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
    ok = fileHandler.openFiles(urls);
    if (!ok) {
        DFMBASE_NAMESPACE::GlobalEventType lastEvent = fileHandler.lastEventType();
        if (lastEvent != DFMBASE_NAMESPACE::GlobalEventType::kUnknowType) {
            if (lastEvent == DFMBASE_NAMESPACE::GlobalEventType::kDeleteFiles)
                dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kDeleteFiles, windowId, urls, AbstractJobHandler::JobFlag::kNoHint, nullptr);
            else if (lastEvent == DFMBASE_NAMESPACE::GlobalEventType::kMoveToTrash)
                dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kMoveToTrash, windowId, urls, AbstractJobHandler::JobFlag::kNoHint, nullptr);
        } else {
            // deal open file with custom dialog
            dpfSlotChannel->push("dfmplugin_utils", "slot_OpenWith_ShowDialog", urls);
            ok = true;
        }
    }
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenFilesResult, windowId, urls, ok, error);
    return ok;
}

void FileOperationsEventReceiver::handleOperationOpenFiles(const quint64 windowId,
                                                           const QList<QUrl> urls,
                                                           const QVariant custom,
                                                           DFMBASE_NAMESPACE::Global::OperatorCallback callback)
{
    bool ok = handleOperationOpenFiles(windowId, urls);
    if (callback) {
        CallbackArgus args(new QMap<CallbackKey, QVariant>);
        args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(CallbackKey::kSourceUrls, QVariant::fromValue(urls));
        args->insert(CallbackKey::kSuccessed, QVariant::fromValue(ok));
        args->insert(CallbackKey::kCustom, custom);
        callback(args);
    }
}

void FileOperationsEventReceiver::handleOperationOpenFilesByApp(const quint64 windowId,
                                                                const QList<QUrl> urls,
                                                                const QList<QString> apps,
                                                                const QVariant custom,
                                                                DFMBASE_NAMESPACE::Global::OperatorCallback callback)
{
    bool ok = handleOperationOpenFilesByApp(windowId, urls, apps);
    if (callback) {
        CallbackArgus args(new QMap<CallbackKey, QVariant>);
        args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(CallbackKey::kSourceUrls, QVariant::fromValue(urls));
        args->insert(CallbackKey::kSuccessed, QVariant::fromValue(ok));
        args->insert(CallbackKey::kCustom, custom);
        callback(args);
    }
}

bool FileOperationsEventReceiver::handleOperationOpenFilesByApp(const quint64 windowId,
                                                                const QList<QUrl> urls,
                                                                const QList<QString> apps)
{
    Q_UNUSED(windowId);
    bool ok = false;
    QString error;
    if (!urls.isEmpty() && !urls.first().isLocalFile()) {
        if (dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_OpenFileByApp", windowId, urls, apps)) {
            dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenFilesByAppResult, windowId, urls, true, error);
            return true;
        }
    }
    DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
    QString app;
    if (apps.count() == 1) {
        app = apps.at(0);
    }
    ok = fileHandler.openFilesByApp(urls, app);
    if (!ok) {
        error = fileHandler.errorString();
        dialogManager->showErrorDialog("open file by app error", error);
    }
    // TODO:: file openFilesByApp finished need to send file openFilesByApp finished event
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenFilesByAppResult, windowId, urls, ok, error);
    return ok;
}

bool FileOperationsEventReceiver::handleOperationRenameFile(const quint64 windowId,
                                                            const QUrl oldUrl,
                                                            const QUrl newUrl,
                                                            const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags)
{
    Q_UNUSED(windowId);
    bool ok = false;
    QString error;

    if (FileUtils::isDesktopFile(oldUrl) && !DecoratorFileInfo(oldUrl).isSymLink())
        return doRenameDesktopFile(windowId, oldUrl, newUrl, flags);

    if (!oldUrl.isLocalFile()) {
        if (dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_RenameFile", windowId, oldUrl, newUrl, flags)) {
            QMap<QUrl, QUrl> renamedFiles { { oldUrl, newUrl } };
            dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kRenameFileResult,
                                         windowId, renamedFiles, ok, error);
            if (!flags.testFlag(AbstractJobHandler::JobFlag::kRevocation))
                saveFileOperation({ newUrl }, { oldUrl }, GlobalEventType::kRenameFile);
            return true;
        }
    }

    AbstractFileInfoPointer toFileInfo = InfoFactory::create<AbstractFileInfo>(newUrl);
    if (toFileInfo && DecoratorFile(newUrl).exists()) {
        dialogManager->showRenameNameSameErrorDialog(toFileInfo->fileName());
        return false;
    }

    DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
    ok = fileHandler.renameFile(oldUrl, newUrl);
    if (!ok) {
        error = fileHandler.errorString();
        dialogManager->showRenameBusyErrDialog();
    }
    // TODO:: file renameFile finished need to send file renameFile finished event
    QMap<QUrl, QUrl> renamedFiles { { oldUrl, newUrl } };
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kRenameFileResult,
                                 windowId, renamedFiles, ok, error);
    if (!flags.testFlag(AbstractJobHandler::JobFlag::kRevocation))
        saveFileOperation({ newUrl }, { oldUrl }, GlobalEventType::kRenameFile);
    return ok;
}

void FileOperationsEventReceiver::handleOperationRenameFile(const quint64 windowId,
                                                            const QUrl oldUrl,
                                                            const QUrl newUrl,
                                                            const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                            const QVariant custom,
                                                            OperatorCallback callback)
{
    bool ok = handleOperationRenameFile(windowId, oldUrl, newUrl, flags);
    if (callback) {
        CallbackArgus args(new QMap<CallbackKey, QVariant>);
        args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(CallbackKey::kSourceUrls, QVariant::fromValue(QList<QUrl>() << oldUrl));
        args->insert(CallbackKey::kTargets, QVariant::fromValue(QList<QUrl>() << oldUrl));
        args->insert(CallbackKey::kSuccessed, QVariant::fromValue(ok));
        args->insert(CallbackKey::kCustom, custom);
        callback(args);
    }
}

bool FileOperationsEventReceiver::handleOperationRenameFiles(const quint64 windowId, const QList<QUrl> urls, const QPair<QString, QString> pair, const bool replace)
{
    QMap<QUrl, QUrl> successUrls;
    bool ok = false;
    QString error;
    if (!urls.isEmpty() && !urls.first().isLocalFile()) {
        if (dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_RenameFiles", windowId, urls, pair, replace)) {

            dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kRenameFileResult,
                                         windowId, successUrls, true, error);
            if (!successUrls.isEmpty())
                saveFileOperation(successUrls.values(), successUrls.keys(), GlobalEventType::kRenameFiles);

            return true;
        }
    }

    RenameTypes type = RenameTypes::kBatchRepalce;
    if (!replace)
        type = RenameTypes::kBatchCustom;
    ok = doRenameFiles(windowId, urls, pair, {}, type, successUrls, error);

    // publish result
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kRenameFileResult,
                                 windowId, successUrls, ok, error);
    if (!successUrls.isEmpty())
        saveFileOperation(successUrls.values(), successUrls.keys(), GlobalEventType::kRenameFiles);

    return ok;
}

void FileOperationsEventReceiver::handleOperationRenameFiles(const quint64 windowId, const QList<QUrl> urls, const QPair<QString, QString> pair, const bool replace, const QVariant custom, OperatorCallback callback)
{
    QMap<QUrl, QUrl> successUrls;
    QString error;
    RenameTypes type = RenameTypes::kBatchRepalce;
    if (!replace)
        type = RenameTypes::kBatchCustom;
    bool ok = doRenameFiles(windowId, urls, pair, {}, type, successUrls, error, custom, callback);
    // publish result
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kRenameFileResult,
                                 windowId, successUrls, ok, error);
    if (!successUrls.isEmpty())
        saveFileOperation(successUrls.values(), successUrls.keys(), GlobalEventType::kRenameFiles);
}

bool FileOperationsEventReceiver::handleOperationRenameFiles(const quint64 windowId, const QList<QUrl> urls, const QPair<QString, AbstractJobHandler::FileNameAddFlag> pair)
{
    QMap<QUrl, QUrl> successUrls;
    bool ok = false;
    QString error;
    if (!urls.isEmpty() && !urls.first().isLocalFile()) {
        if (dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_RenameFilesAddText", windowId, urls, pair)) {

            dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kRenameFileResult,
                                         windowId, successUrls, true, error);
            if (!successUrls.isEmpty())
                saveFileOperation(successUrls.values(), successUrls.keys(), GlobalEventType::kRenameFiles);

            return true;
        }
    }

    ok = doRenameFiles(windowId, urls, {}, pair, RenameTypes::kBatchAppend, successUrls, error);

    // publish result
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kRenameFileResult,
                                 windowId, successUrls, ok, error);
    if (!successUrls.isEmpty())
        saveFileOperation(successUrls.values(), successUrls.keys(), GlobalEventType::kRenameFiles);

    return ok;
}

void FileOperationsEventReceiver::handleOperationRenameFiles(const quint64 windowId, const QList<QUrl> urls, const QPair<QString, AbstractJobHandler::FileNameAddFlag> pair, const QVariant custom, OperatorCallback callback)
{
    QMap<QUrl, QUrl> successUrls;
    QString error;
    bool ok = doRenameFiles(windowId, urls, {}, pair, RenameTypes::kBatchAppend, successUrls, error, custom, callback);
    // publish result
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kRenameFileResult,
                                 windowId, successUrls, ok, error);
    if (!successUrls.isEmpty())
        saveFileOperation(successUrls.values(), successUrls.keys(), GlobalEventType::kRenameFiles);
}

bool FileOperationsEventReceiver::handleOperationMkdir(const quint64 windowId, const QUrl url)
{
    QUrl targetUrl;
    return doMkdir(windowId, url, targetUrl);
}

void FileOperationsEventReceiver::handleOperationMkdir(const quint64 windowId,
                                                       const QUrl url,
                                                       const QVariant custom,
                                                       OperatorCallback callback)
{
    QUrl targetUrl;
    bool ok = doMkdir(windowId, url, targetUrl);
    if (callback) {
        CallbackArgus args(new QMap<CallbackKey, QVariant>);
        args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(CallbackKey::kSourceUrls, QVariant::fromValue(QList<QUrl>() << url));
        args->insert(CallbackKey::kTargets, QVariant::fromValue(QList<QUrl>() << targetUrl));
        args->insert(CallbackKey::kSuccessed, QVariant::fromValue(ok));
        args->insert(CallbackKey::kCustom, custom);
        callback(args);
    }
}

bool FileOperationsEventReceiver::doTouchFilePractically(const quint64 windowId, const QUrl url)
{
    QString error;

    DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
    bool ok = fileHandler.touchFile(url);
    if (!ok) {
        error = fileHandler.errorString();
        dialogManager->showErrorDialog("touch file error", error);
    }
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kTouchFileResult,
                                 windowId, QList<QUrl>() << url, ok, error);
    saveFileOperation({ url }, {}, GlobalEventType::kDeleteFiles);

    return ok;
}

QString FileOperationsEventReceiver::handleOperationTouchFile(const quint64 windowId,
                                                              const QUrl url,
                                                              const CreateFileType fileType,
                                                              const QString suffix)
{
    return doTouchFilePremature(windowId, url, fileType, suffix, QVariant(), nullptr);
}

void FileOperationsEventReceiver::handleOperationTouchFile(const quint64 windowId,
                                                           const QUrl url,
                                                           const CreateFileType fileType,
                                                           const QString suffix,
                                                           const QVariant custom,
                                                           OperatorCallback callbackImmediately)
{
    doTouchFilePremature(windowId, url, fileType, suffix, custom, callbackImmediately);
}

bool FileOperationsEventReceiver::handleOperationLinkFile(const quint64 windowId,
                                                          const QUrl url,
                                                          const QUrl link,
                                                          const bool force,
                                                          const bool silence)
{
    bool ok = false;
    QString error;
    if (!url.isLocalFile()) {
        if (dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_LinkFile", windowId, url, link, force, silence)) {
            dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kCreateSymlinkResult,
                                         windowId, QList<QUrl>() << url << link, true, error);
            return true;
        }
    }

    DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
    // check link
    if (force) {
        AbstractFileInfoPointer toInfo = InfoFactory::create<AbstractFileInfo>(link);
        if (toInfo && DecoratorFile(link).exists()) {
            DFMBASE_NAMESPACE::LocalFileHandler fileHandlerDelete;
            fileHandlerDelete.deleteFile(link);
        }
    }
    QUrl urlValid = link;
    if (silence) {
        urlValid = checkTargetUrl(link);
    }
    ok = fileHandler.createSystemLink(url, urlValid);
    if (!ok) {
        error = fileHandler.errorString();
        dialogManager->showErrorDialog("link file error", error);
    }
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kCreateSymlinkResult,
                                 windowId, QList<QUrl>() << url << urlValid, ok, error);
    return ok;
}

void FileOperationsEventReceiver::handleOperationLinkFile(const quint64 windowId,
                                                          const QUrl url,
                                                          const QUrl link,
                                                          const bool force,
                                                          const bool silence,
                                                          const QVariant custom,
                                                          DFMBASE_NAMESPACE::Global::OperatorCallback callback)
{
    bool ok = handleOperationLinkFile(windowId, url, link, force, silence);

    if (callback) {
        CallbackArgus args(new QMap<CallbackKey, QVariant>);
        args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(CallbackKey::kSourceUrls, QVariant::fromValue(QList<QUrl>() << url));
        args->insert(CallbackKey::kTargets, QVariant::fromValue(QList<QUrl>() << link));
        args->insert(CallbackKey::kSuccessed, QVariant::fromValue(ok));
        args->insert(CallbackKey::kCustom, custom);
        callback(args);
    }
}

bool FileOperationsEventReceiver::handleOperationSetPermission(const quint64 windowId,
                                                               const QUrl url,
                                                               const QFileDevice::Permissions permissions)
{
    QString error;
    bool ok = false;
    if (!url.isLocalFile()) {
        // hook events
        bool ok = false;
        if (dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_SetPermission", windowId, url, permissions, &ok, &error)) {
            if (!ok)
                dialogManager->showErrorDialog("set file permissions error", error);
            dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kSetPermissionResult, windowId, QList<QUrl>() << url, ok, error);
            return ok;
        }
    }
    DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
    ok = fileHandler.setPermissions(url, permissions);
    if (!ok) {
        error = fileHandler.errorString();
        dialogManager->showErrorDialog("set file permissions error", error);
    }
    AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url);
    info->refresh();
    // TODO:: set file permissions finished need to send set file permissions finished event
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kSetPermissionResult,
                                 windowId, QList<QUrl>() << url, ok, error);
    return ok;
}

void FileOperationsEventReceiver::handleOperationSetPermission(const quint64 windowId,
                                                               const QUrl url,
                                                               const QFileDevice::Permissions permissions, const QVariant custom,
                                                               DFMBASE_NAMESPACE::Global::OperatorCallback callback)
{
    bool ok = handleOperationSetPermission(windowId, url, permissions);
    if (callback) {
        CallbackArgus args(new QMap<CallbackKey, QVariant>);
        args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(CallbackKey::kSourceUrls, QVariant::fromValue(QList<QUrl>() << url));
        args->insert(CallbackKey::kSuccessed, QVariant::fromValue(ok));
        args->insert(CallbackKey::kCustom, custom);
        callback(args);
    }
}

bool FileOperationsEventReceiver::handleOperationWriteToClipboard(const quint64 windowId,
                                                                  const DFMBASE_NAMESPACE::ClipBoard::ClipboardAction action,
                                                                  const QList<QUrl> urls)
{
    QString error;
    if (!urls.isEmpty() && !urls.first().isLocalFile()) {
        if (dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_WriteUrlsToClipboard", windowId, action, urls)) {
            return true;
        }
    }
    DFMBASE_NAMESPACE::ClipBoard::instance()->setUrlsToClipboard(urls, action);
    return true;
}

bool FileOperationsEventReceiver::handleOperationWriteDataToClipboard(const quint64 windowId, QMimeData *data)
{
    Q_UNUSED(windowId);
    if (!data) {
        qWarning() << " write to clipboard data is nullptr!!!!!!!";
        return false;
    }

    DFMBASE_NAMESPACE::ClipBoard::instance()->setDataToClipboard(data);
    return true;
}

bool FileOperationsEventReceiver::handleOperationOpenInTerminal(const quint64 windowId, const QList<QUrl> urls)
{
    QString error;
    bool ok = false;
    bool result = false;
    QSharedPointer<LocalFileHandler> fileHandler = nullptr;
    if (urls.count() > 0 && !urls.first().isLocalFile()) {
        if (dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_OpenInTerminal", windowId, urls)) {
            dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenInTerminalResult,
                                         windowId, urls, true, error);
            return true;
        }
    }

    for (const auto &url : urls) {
        const QString &current_dir = QDir::currentPath();
        QDir::setCurrent(url.toLocalFile());
        if (!url.isLocalFile()) {
        }
        if (fileHandler.isNull())
            fileHandler.reset(new LocalFileHandler());
        ok = QProcess::startDetached(fileHandler->defaultTerminalPath());
        if (!result)
            result = ok;
        QDir::setCurrent(current_dir);
    }

    // TODO:: open file in terminal finished need to send open file in terminal finished event
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenInTerminalResult,
                                 windowId, urls, result, error);

    return ok;
}

bool FileOperationsEventReceiver::handleOperationSaveOperations(const QVariantMap values)
{
    if (operationsStackDbus) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__;
        auto &&reply = operationsStackDbus->SaveOperations(values);
        reply.waitForFinished();
        if (!reply.isValid()) {
            qCritical() << "D-Bus reply is invalid " << reply.error();
            return false;
        }
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;
        return true;
    }
    return false;
}

bool FileOperationsEventReceiver::handleOperationCleanSaveOperationsStack()
{
    if (operationsStackDbus) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__;
        operationsStackDbus->CleanOperations();
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;
        return true;
    }
    return false;
}

bool FileOperationsEventReceiver::handleOperationRevocation(const quint64 windowId,
                                                            DFMGLOBAL_NAMESPACE::OperatorHandleCallback handle)
{
    QVariantMap ret;
    if (operationsStackDbus) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__;
        auto &&reply = operationsStackDbus->RevocationOperations();
        reply.waitForFinished();
        if (reply.isValid())
            ret = reply.value();
        else {
            qCritical() << "D-Bus reply is invalid " << reply.error();
            return false;
        }
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;

        return revocation(windowId, ret, handle);
    }

    return false;
}

bool FileOperationsEventReceiver::handleOperationHideFiles(const quint64 windowId, const QList<QUrl> urls)
{
    Q_UNUSED(windowId)

    bool ok { true };
    for (const QUrl &url : urls) {
        AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url);
        if (info) {
            const QUrl &parentUrl = info->parentUrl();
            const QString &fileName = info->fileName();

            HideFileHelper helper(parentUrl);
            helper.contains(fileName) ? helper.remove(fileName) : helper.insert(fileName);
            if (!helper.save())
                ok = false;
        }
    }

    if (ok && !urls.isEmpty())
        FileUtils::notifyFileChangeManual(DFMGLOBAL_NAMESPACE::FileNotifyType::kFileChanged, urls.first());

    dpfSignalDispatcher->publish(GlobalEventType::kHideFilesResult, windowId, urls, ok);
    return ok;
}

void FileOperationsEventReceiver::handleOperationHideFiles(const quint64 windowId, const QList<QUrl> urls,
                                                           const QVariant custom, OperatorCallback callback)
{
    bool ok = handleOperationHideFiles(windowId, urls);
    if (callback) {
        CallbackArgus args(new QMap<CallbackKey, QVariant>);
        args->insert(CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(CallbackKey::kSourceUrls, QVariant::fromValue(QList<QUrl>() << urls));
        args->insert(CallbackKey::kSuccessed, QVariant::fromValue(ok));
        args->insert(CallbackKey::kCustom, custom);
        callback(args);
    }
}
