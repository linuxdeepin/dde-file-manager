// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileoperationseventreceiver.h"
#include "trashfileeventreceiver.h"
#include "fileoperationsevent/fileoperationseventhandler.h"
#include "fileoperations/operationsstackproxy.h"

#include <dfm-base/utils/hidefilehelper.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/utils/windowutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/utils/properties.h>
#include <dfm-base/utils/systempathutil.h>

#include <dfm-io/dfmio_utils.h>

#include <QDebug>

Q_DECLARE_METATYPE(QList<QUrl> *)
Q_DECLARE_METATYPE(bool *)
Q_DECLARE_METATYPE(QString *)
Q_DECLARE_METATYPE(QFileDevice::Permission)

DFMGLOBAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

namespace dfmplugin_fileoperations {

FileOperationsEventReceiver::FileOperationsEventReceiver(QObject *parent)
    : QObject(parent), dialogManager(DialogManagerInstance)
{
}

QString FileOperationsEventReceiver::newDocmentName(const QUrl &url,
                                                    const QString &suffix,
                                                    const CreateFileType fileType)
{
    QString suffixex;
    QString baseName;
    switch (fileType) {
    case CreateFileType::kCreateFileTypeFolder:
        baseName = QObject::tr("New Folder");
        break;
    case CreateFileType::kCreateFileTypeText:
        baseName = QObject::tr("New Text");
        suffixex = "txt";
        break;
    case CreateFileType::kCreateFileTypeWord:
        baseName = QObject::tr("Document");
        suffixex = "doc";
        break;
    case CreateFileType::kCreateFileTypeExcel:
        baseName = QObject::tr("Spreadsheet");
        suffixex = "xls";
        break;
    case CreateFileType::kCreateFileTypePowerpoint:
        baseName = QObject::tr("Presentation");
        suffixex = "ppt";
        break;
    case CreateFileType::kCreateFileTypeDefault:
        baseName = QObject::tr("New File");
        suffixex = suffix;
        break;
    default:
        return QString();
    }
    return newDocmentName(url, baseName, suffixex);
}

QString FileOperationsEventReceiver::newDocmentName(const QUrl &url, const QString &baseName, const QString &suffix)
{
    QString targetdir = url.path();
    if (targetdir.isEmpty())
        return QString();
    if (targetdir.endsWith(QDir::separator()))
        targetdir.chop(1);
    QString filePath = suffix.isEmpty() ? QString("%1/%2").arg(targetdir, baseName) : QString("%1/%2.%3").arg(targetdir, baseName, suffix);

    QString localTargetDir { targetdir };
    QString fileLocalPath { filePath };
    if (!dfmbase::FileUtils::isLocalFile(url)) {
        auto &&parentFileInfo { InfoFactory::create<FileInfo>(url) };
        if (!parentFileInfo) {
            fmCritical() << "create parent file info failed!";
            return QString();
        }
        localTargetDir = parentFileInfo->pathOf(FileInfo::FilePathInfoType::kFilePath);
        if (localTargetDir.endsWith(QDir::separator()))
            localTargetDir.chop(1);
        fileLocalPath = suffix.isEmpty() ? QString("%1/%2").arg(localTargetDir, baseName) : QString("%1/%2.%3").arg(localTargetDir, baseName, suffix);
    }

    int i = 0;
    while (true) {
        if (QFile(fileLocalPath).exists()) {
            ++i;
            filePath = suffix.isEmpty()
                    ? QString("%1/%2%3").arg(targetdir, baseName, QString::number(i))
                    : QString("%1/%2%3.%4").arg(targetdir, baseName, QString::number(i), suffix);
            fileLocalPath = suffix.isEmpty()
                    ? QString("%1/%2%3").arg(localTargetDir, baseName, QString::number(i))
                    : QString("%1/%2%3.%4").arg(localTargetDir, baseName, QString::number(i), suffix);
        } else {
            return filePath;
        }
    }
}

bool FileOperationsEventReceiver::revocation(const quint64 windowId, const QVariantMap &ret,
                                             DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handle)
{
    if (!ret.contains("undoevent") || !ret.contains("undosources") || !ret.contains("undotargets"))
        return false;
    GlobalEventType eventType = static_cast<GlobalEventType>(ret.value("undoevent").value<uint16_t>());
    QList<QUrl> sources = QUrl::fromStringList(ret.value("undosources").toStringList());
    QList<QUrl> targets = QUrl::fromStringList(ret.value("undotargets").toStringList());
    if (eventType != kRestoreFromTrash) {
        for (const auto &url : sources) {
            if (!DFMIO::DFile(url).exists()) {
                // Their sizes are equal, indicating that the current operation is many-to-many.
                // So files that do not exist in sources need to be deleted in targets as well
                if (targets.size() == sources.size()) {
                    int index = sources.indexOf(url);
                    targets.removeAt(index);
                }
                sources.removeOne(url);
            }
        }
    }

    if (sources.isEmpty())
        return true;

    switch (eventType) {
    case kCutFile:
        if (targets.isEmpty())
            return true;
        handleOperationUndoCut(windowId, sources, targets.first(), AbstractJobHandler::JobFlag::kRevocation, handle, ret);
        break;
    case kDeleteFiles:
        handleOperationUndoDeletes(windowId, sources,
                                   AbstractJobHandler::JobFlag::kRevocation,
                                   handle, ret);
        break;
    case kMoveToTrash:
        TrashFileEventReceiver::instance()->handleOperationUndoMoveToTrash(windowId, sources,
                                                                           AbstractJobHandler::JobFlag::kRevocation,
                                                                           handle, ret);
        break;
    case kRestoreFromTrash:
        TrashFileEventReceiver::instance()->handleOperationUndoRestoreFromTrash(windowId, sources, QUrl(),
                                                                                AbstractJobHandler::JobFlag::kRevocation,
                                                                                handle, ret);
        break;
    case kRenameFile:
        if (targets.isEmpty())
            return true;
        handleOperationRenameFile(windowId, sources.first(),
                                  targets.first(),
                                  AbstractJobHandler::JobFlag::kRevocation);
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

bool FileOperationsEventReceiver::redo(const quint64 windowId, const QVariantMap &ret, AbstractJobHandler::OperatorHandleCallback handle)
{
    if (!ret.contains("undoevent") || !ret.contains("undosources") || !ret.contains("undotargets"))
        return false;
    GlobalEventType eventType = static_cast<GlobalEventType>(ret.value("undoevent").value<uint16_t>());
    QList<QUrl> sources = QUrl::fromStringList(ret.value("undosources").toStringList());
    QList<QUrl> targets = QUrl::fromStringList(ret.value("undotargets").toStringList());
    if (eventType != kRestoreFromTrash && eventType != kMkdir && eventType != kTouchFile) {
        for (const auto &url : sources) {
            if (!DFMIO::DFile(url).exists()) {
                // Their sizes are equal, indicating that the current operation is many-to-many.
                // So files that do not exist in sources need to be deleted in targets as well
                if (targets.size() == sources.size()) {
                    int index = sources.indexOf(url);
                    targets.removeAt(index);
                }
                sources.removeOne(url);
            }
        }
    }

    if (sources.isEmpty())
        return true;

    switch (eventType) {
    case kCopy:
        if (targets.isEmpty())
            return true;
        handleOperationCopy(windowId, { sources }, targets.first(), DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kNoHint, handle);
        break;
    case kCutFile:
        if (targets.isEmpty())
            return true;
        handleOperationCut(windowId, { sources }, targets.first(), AbstractJobHandler::JobFlag::kNoHint, handle);
        break;
    case kDeleteFiles:
        handleOperationDeletes(windowId, sources, AbstractJobHandler::JobFlag::kNoHint, handle);
        break;
    case kMoveToTrash:
        TrashFileEventReceiver::instance()->handleOperationMoveToTrash(windowId, sources, AbstractJobHandler::JobFlag::kNoHint, handle);
        break;
    case kRestoreFromTrash:
        TrashFileEventReceiver::instance()->handleOperationRestoreFromTrash(windowId, sources, QUrl(), AbstractJobHandler::JobFlag::kNoHint, handle);
        break;
    case kRenameFile:
        if (targets.isEmpty())
            return true;
        if (handleOperationRenameFile(windowId, sources.first(),
                                      targets.first(), AbstractJobHandler::JobFlag::kRedo))
            saveFileOperation(targets, sources, GlobalEventType::kRenameFile,
                              sources, targets, GlobalEventType::kRenameFile);
        break;
    case kRenameFiles: {
        if (targets.isEmpty())
            return true;
        QList<QUrl> successSources, successTargets;
        for (int i = 0; i < sources.size(); ++i) {
            if (handleOperationRenameFile(windowId, sources[i], targets[i],
                                          AbstractJobHandler::JobFlag::kRedo)) {
                successSources.append(sources[i]);
                successTargets.append(targets[i]);
            }
        }
        if (!successSources.isEmpty()) {
            saveFileOperation(successTargets, successSources, GlobalEventType::kRenameFile,
                              successSources, successTargets, GlobalEventType::kRenameFile);
        }

        break;
    }
    case kMkdir:
        if (sources.isEmpty())
            return true;
        return doMkdir(windowId, sources.first(), QVariant(), nullptr, true);
    case kTouchFile: {
        if (sources.isEmpty())
            return true;
        QUrl templateUrl = ret.value("templateurl", QUrl()).toUrl();
        doTouchFilePractically(windowId, sources.first(), templateUrl);
        break;
    }
    default:
        return false;
    }

    return true;
}

bool FileOperationsEventReceiver::doRenameFiles(const quint64 windowId, const QList<QUrl> &urls, const QPair<QString, QString> &pair,
                                                const QPair<QString, DFMBASE_NAMESPACE::AbstractJobHandler::FileNameAddFlag> &pair2,
                                                const RenameTypes type, QMap<QUrl, QUrl> &successUrls, QString &errorMsg,
                                                const QVariant custom, DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback)
{
    DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
    bool ok = false, renameDesktop = true;
    switch (type) {
    case RenameTypes::kBatchRepalce: {
        auto tmpurls = urls;
        QMap<QUrl, QUrl> needDealUrls;
        renameDesktop = doRenameDesktopFiles(tmpurls, pair, needDealUrls, successUrls);
        QMap<QUrl, QUrl> needDealUrls1 = FileUtils::fileBatchReplaceText(tmpurls, pair);
        for (auto it = needDealUrls1.begin(); it != needDealUrls1.end(); ++it) {
            needDealUrls.insert(it.key(), it.value());
        }
        if (callback) {
            AbstractJobHandler::CallbackArgus args(new QMap<AbstractJobHandler::CallbackKey, QVariant>);
            args->insert(AbstractJobHandler::CallbackKey::kWindowId, QVariant::fromValue(windowId));
            args->insert(AbstractJobHandler::CallbackKey::kSourceUrls, QVariant::fromValue(QList<QUrl>() << needDealUrls.keys()));
            args->insert(AbstractJobHandler::CallbackKey::kTargets, QVariant::fromValue(QList<QUrl>() << needDealUrls.values()));
            args->insert(AbstractJobHandler::CallbackKey::kCustom, custom);
            callback(args);
        }

        if (needDealUrls1.isEmpty() || !renameDesktop)
            return false;
        ok = fileHandler.renameFilesBatch(needDealUrls1, successUrls);
        break;
    }
    case RenameTypes::kBatchCustom: {
        QMap<QUrl, QUrl> needDealUrls = FileUtils::fileBatchCustomText(urls, pair);
        if (callback) {
            AbstractJobHandler::CallbackArgus args(new QMap<AbstractJobHandler::CallbackKey, QVariant>);
            args->insert(AbstractJobHandler::CallbackKey::kWindowId, QVariant::fromValue(windowId));
            args->insert(AbstractJobHandler::CallbackKey::kSourceUrls, QVariant::fromValue(QList<QUrl>() << needDealUrls.keys()));
            args->insert(AbstractJobHandler::CallbackKey::kTargets, QVariant::fromValue(QList<QUrl>() << needDealUrls.values()));
            args->insert(AbstractJobHandler::CallbackKey::kCustom, custom);
            callback(args);
        }
        ok = fileHandler.renameFilesBatch(needDealUrls, successUrls);
        break;
    }
    case RenameTypes::kBatchAppend: {
        QMap<QUrl, QUrl> needDealUrls = FileUtils::fileBatchAddText(urls, pair2);
        if (callback) {
            AbstractJobHandler::CallbackArgus args(new QMap<AbstractJobHandler::CallbackKey, QVariant>);
            args->insert(AbstractJobHandler::CallbackKey::kWindowId, QVariant::fromValue(windowId));
            args->insert(AbstractJobHandler::CallbackKey::kSourceUrls, QVariant::fromValue(QList<QUrl>() << needDealUrls.keys()));
            args->insert(AbstractJobHandler::CallbackKey::kTargets, QVariant::fromValue(QList<QUrl>() << needDealUrls.values()));
            args->insert(AbstractJobHandler::CallbackKey::kCustom, custom);
            callback(args);
        }
        ok = fileHandler.renameFilesBatch(needDealUrls, successUrls);
        break;
    }
    }
    if (!ok) {
        errorMsg = fileHandler.errorString();
        DialogManagerInstance->showErrorDialog(tr("Rename file error"), errorMsg);
    }

    for (const auto &scUrl : successUrls.keys()) {
        ClipBoard::instance()->replaceClipboardUrl(scUrl, successUrls.value(scUrl));
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

    FileInfoPointer newFileInfo = InfoFactory::create<FileInfo>(newUrl);
    FileInfoPointer oldFileInfo = InfoFactory::create<FileInfo>(oldUrl);
    const QString &newName = newFileInfo->displayOf(DisPlayInfoType::kFileDisplayName);
    const QString &oldName = oldFileInfo->displayOf(DisPlayInfoType::kFileDisplayName);
    if (newName == oldName)
        return true;

    desktop.set(key, newFileInfo->displayOf(DisPlayInfoType::kFileDisplayName));
    desktop.set("X-Deepin-Vendor", QStringLiteral("user-custom"));
    if (desktop.save(desktopPath, "Desktop Entry")) {
        QMap<QUrl, QUrl> renamed { { oldUrl, newUrl } };
        dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kRenameFileResult,
                                     windowId, renamed, true, "");

        if (!flags.testFlag(AbstractJobHandler::JobFlag::kRedo)) {
            const QString path = QFileInfo(desktopPath).absoluteDir().absoluteFilePath(oldName);
            saveFileOperation({ oldUrl }, { QUrl::fromLocalFile(path) }, GlobalEventType::kRenameFile,
                              { QUrl::fromLocalFile(path) }, { oldUrl },
                              GlobalEventType::kRenameFile, flags.testFlag(AbstractJobHandler::JobFlag::kRevocation));
        }
        return true;
    }

    return false;
}

bool FileOperationsEventReceiver::doRenameDesktopFiles(QList<QUrl> &urls, const QPair<QString, QString> pair, QMap<QUrl, QUrl> &needDealUrls, QMap<QUrl, QUrl> &successUrls)
{
    for (auto it = urls.begin(); it != urls.end();) {
        auto oldUrl = *it;
        if (!FileUtils::isDesktopFile(oldUrl)) {
            it++;
            continue;
        }
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
        FileInfoPointer oldFileInfo = InfoFactory::create<FileInfo>(oldUrl);
        const QString &oldName = oldFileInfo->displayOf(DisPlayInfoType::kFileDisplayName);
        auto newUrl = oldUrl;
        auto newName = oldName;
        newName = newName.replace(pair.first, pair.second);
        newUrl.setPath(UrlRoute::urlParent(oldUrl).path() + QDir::separator() + newName);
        needDealUrls.insert(oldUrl, newUrl);
        if (newName == oldName) {
            it = urls.erase(it);
            continue;
        }

        desktop.set(key, newName);
        desktop.set("X-Deepin-Vendor", QStringLiteral("user-custom"));
        if (!desktop.save(desktopPath, "Desktop Entry")) {
            return false;
        }
        successUrls.insert(oldUrl, newUrl);
        it = urls.erase(it);
    }

    return true;
}

JobHandlePointer FileOperationsEventReceiver::doCopyFile(const quint64 windowId, const QList<QUrl> &sources, const QUrl &target,
                                                         const AbstractJobHandler::JobFlags flags, DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback callbaskHandle)
{
    // 深信服拷贝
    if (flags.testFlag(AbstractJobHandler::JobFlag::kCopyRemote)) {
        JobHandlePointer handle = copyMoveJob->copy(sources, target, flags);
        if (callbaskHandle)
            callbaskHandle(handle);
        return handle;
    }
    if (sources.isEmpty())
        return nullptr;

    QList<QUrl> sourcesTrans = sources;

    QList<QUrl> urls {};
    bool ok = UniversalUtils::urlsTransformToLocal(sourcesTrans, &urls);
    if (ok && !urls.isEmpty())
        sourcesTrans = urls;

    if (!dfmbase::FileUtils::isLocalFile(target)) {
        if (dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_CopyFile", windowId, sourcesTrans, target, flags)) {
            return nullptr;
        }
    }
    const QUrl &urlFrom = sources.first();
    if (!dfmbase::FileUtils::isLocalFile(urlFrom)) {
        if (dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_CopyFromFile", windowId, sourcesTrans, target, flags)) {
            return nullptr;
        }
    }

    JobHandlePointer handle = copyMoveJob->copy(sourcesTrans, target, flags);
    if (callbaskHandle)
        callbaskHandle(handle);
    return handle;
}

JobHandlePointer FileOperationsEventReceiver::doCutFile(quint64 windowId, const QList<QUrl> &sources,
                                                        const QUrl &target,
                                                        const AbstractJobHandler::JobFlags flags,
                                                        AbstractJobHandler::OperatorHandleCallback handleCallback,
                                                        const bool isInit)
{
    if (sources.isEmpty())
        return nullptr;

    // cut file to file current dir
    if (FileUtils::isSameFile(UrlRoute::urlParent(sources[0]), target)) {
        fmWarning() << "cut file to same dir!!!!!!!!!";
        return nullptr;
    }

    QList<QUrl> sourcesTrans = sources;
    QList<QUrl> urls {};
    bool ok = UniversalUtils::urlsTransformToLocal(sourcesTrans, &urls);
    if (ok && !urls.isEmpty())
        sourcesTrans = urls;

    if (!dfmbase::FileUtils::isLocalFile(target)) {
        if (dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_CutToFile", windowId, sourcesTrans, target, flags)) {
            return nullptr;
        }
    }
    const QUrl &urlFrom = sources.first();
    if (!dfmbase::FileUtils::isLocalFile(urlFrom)) {
        if (dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_CutFromFile", windowId, sourcesTrans, target, flags)) {
            return nullptr;
        }
    }

    JobHandlePointer handle = copyMoveJob->cut(sourcesTrans, target, flags, isInit);
    if (!isInit)
        return handle;

    if (handleCallback)
        handleCallback(handle);

    return handle;
}

JobHandlePointer FileOperationsEventReceiver::doDeleteFile(const quint64 windowId,
                                                           const QList<QUrl> &sources,
                                                           const AbstractJobHandler::JobFlags flags,
                                                           AbstractJobHandler::OperatorHandleCallback handleCallback,
                                                           const bool isInit,
                                                           DoDeleteErrorType &errorType)
{
    if (sources.isEmpty()) {
        errorType = DoDeleteErrorType::kSourceEmpty;
        return nullptr;
    }

    // hook events
    if (dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_DeleteFile", windowId, sources, flags)) {
        return nullptr;
    }

    if (SystemPathUtil::instance()->checkContainsSystemPath(sources)) {
        DialogManagerInstance->showDeleteSystemPathWarnDialog(windowId);
        errorType = DoDeleteErrorType::kNullPtr;
        return nullptr;
    }

    if (flags.testFlag(AbstractJobHandler::JobFlag::kRevocation)
        && DialogManagerInstance->showRestoreDeleteFilesDialog(sources) != QDialog::Accepted) {
        errorType = DoDeleteErrorType::kNullPtr;
        return nullptr;
    }

    // Delete local file with shift+delete, show a confirm dialog.
    if (!flags.testFlag(AbstractJobHandler::JobFlag::kRevocation)
        && DialogManagerInstance->showDeleteFilesDialog(sources) != QDialog::Accepted) {
        errorType = DoDeleteErrorType::kNullPtr;
        return nullptr;
    }

    JobHandlePointer handle = copyMoveJob->deletes(sources, flags, isInit);
    if (!isInit)
        return handle;

    if (handleCallback)
        handleCallback(handle);

    return handle;
}

JobHandlePointer FileOperationsEventReceiver::doCleanTrash(const quint64 windowId, const QList<QUrl> &sources, const AbstractJobHandler::DeleteDialogNoticeType deleteNoticeType, AbstractJobHandler::OperatorHandleCallback handleCallback)
{
    Q_UNUSED(windowId)
    Q_UNUSED(deleteNoticeType)

    if (!sources.isEmpty()) {
        // Show delete files dialog
        if (DialogManagerInstance->showDeleteFilesDialog(sources) != QDialog::Accepted)
            return nullptr;
    } else {
        // Show clear trash dialog
        int count = 0;
        auto info = InfoFactory::create<FileInfo>(FileUtils::trashRootUrl());
        if (info) {
            count = info->countChildFile();
        }
        if (DialogManagerInstance->showClearTrashDialog(count) != QDialog::Accepted) return nullptr;
    }

    QList<QUrl> urls = std::move(sources);
    if (urls.isEmpty())
        urls.push_back(FileUtils::trashRootUrl());

    JobHandlePointer handle = copyMoveJob->cleanTrash(urls);
    if (handleCallback)
        handleCallback(handle);
    return handle;
}

bool FileOperationsEventReceiver::doMkdir(const quint64 windowId, const QUrl &url,
                                          const QVariant &custom,
                                          AbstractJobHandler::OperatorCallback callback, const bool useUrlPath)
{
    QString newPath = useUrlPath ? url.path() : newDocmentName(url, QString(), CreateFileType::kCreateFileTypeFolder);
    if (newPath.isEmpty())
        return false;

    QUrl targetUrl;
    targetUrl.setScheme(url.scheme());
    targetUrl.setPath(newPath);

    bool ok = false;
    QString error;
    if (!dfmbase::FileUtils::isLocalFile(url)) {
        if (dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_MakeDir", windowId, url, targetUrl, custom, callback)) {
            dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kMkdirResult,
                                         windowId, QList<QUrl>() << url, true, error);
            return true;
        }
    }

    DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
    ok = fileHandler.mkdir(targetUrl);
    if (!ok) {
        error = fileHandler.errorString();
        dialogManager->showErrorDialog(tr("Failed to create the directory"), error);
    }

    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kMkdirResult,
                                 windowId, QList<QUrl>() << url, ok, error);
    saveFileOperation({ targetUrl }, {}, GlobalEventType::kDeleteFiles, { targetUrl }, {}, GlobalEventType::kMkdir);

    if (callback) {
        AbstractJobHandler::CallbackArgus args(new QMap<AbstractJobHandler::CallbackKey, QVariant>);
        args->insert(AbstractJobHandler::CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(AbstractJobHandler::CallbackKey::kSourceUrls, QVariant::fromValue(QList<QUrl>() << url));
        args->insert(AbstractJobHandler::CallbackKey::kTargets, QVariant::fromValue(QList<QUrl>() << targetUrl));
        args->insert(AbstractJobHandler::CallbackKey::kSuccessed, QVariant::fromValue(ok));
        args->insert(AbstractJobHandler::CallbackKey::kCustom, custom);
        callback(args);
    }

    return ok;
}

QString FileOperationsEventReceiver::doTouchFilePremature(const quint64 windowId, const QUrl &url, const CreateFileType fileType, const QString &suffix,
                                                          const QVariant &custom, AbstractJobHandler::OperatorCallback callbackImmediately)
{
    const QString newPath = newDocmentName(url, suffix, fileType);
    if (newPath.isEmpty())
        return newPath;

    QUrl urlNew;
    urlNew.setScheme(url.scheme());
    urlNew.setPath(newPath);

    if (dfmbase::FileUtils::isLocalFile(url)) {
        if (callbackImmediately) {
            AbstractJobHandler::CallbackArgus args(new QMap<AbstractJobHandler::CallbackKey, QVariant>);
            args->insert(AbstractJobHandler::CallbackKey::kWindowId, QVariant::fromValue(windowId));
            args->insert(AbstractJobHandler::CallbackKey::kSourceUrls, QVariant::fromValue(QList<QUrl>() << url));
            args->insert(AbstractJobHandler::CallbackKey::kTargets, QVariant::fromValue(QList<QUrl>() << QUrl::fromLocalFile(newPath)));
            args->insert(AbstractJobHandler::CallbackKey::kCustom, custom);
            callbackImmediately(args);
        }

        return doTouchFilePractically(windowId, urlNew) ? newPath : QString();
    } else {
        QString error;
        if (dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_TouchFile", windowId, url, urlNew, fileType, suffix, custom, callbackImmediately, &error)) {
            dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kTouchFileResult,
                                         windowId, QList<QUrl>() << url, true, error);
            return url.path();
        }

        return doTouchFilePractically(windowId, url) ? url.path() : QString();
    }
}

QString FileOperationsEventReceiver::doTouchFilePremature(const quint64 windowId, const QUrl &url, const QUrl &tempUrl, const QString &suffix, const QVariant &custom, AbstractJobHandler::OperatorCallback callbackImmediately)
{
    auto fileInfo = InfoFactory::create<FileInfo>(tempUrl);
    if (!fileInfo)
        return QString();

    const QString &newPath = newDocmentName(url, fileInfo->nameOf(FileInfo::FileNameInfoType::kCompleteBaseName),
                                            suffix.isEmpty() ? fileInfo->nameOf(FileInfo::FileNameInfoType::kSuffix) : suffix);
    if (newPath.isEmpty())
        return QString();

    QUrl urlNew;
    urlNew.setScheme(url.scheme());
    urlNew.setPath(newPath);

    if (dfmbase::FileUtils::isLocalFile(url)) {
        if (callbackImmediately) {
            AbstractJobHandler::CallbackArgus args(new QMap<AbstractJobHandler::CallbackKey, QVariant>);
            args->insert(AbstractJobHandler::CallbackKey::kWindowId, QVariant::fromValue(windowId));
            args->insert(AbstractJobHandler::CallbackKey::kSourceUrls, QVariant::fromValue(QList<QUrl>() << url));
            args->insert(AbstractJobHandler::CallbackKey::kTargets, QVariant::fromValue(QList<QUrl>() << QUrl::fromLocalFile(newPath)));
            args->insert(AbstractJobHandler::CallbackKey::kCustom, custom);
            callbackImmediately(args);
        }

        return doTouchFilePractically(windowId, urlNew, tempUrl) ? newPath : QString();
    } else {
        QString error;
        if (dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_TouchCustomFile", windowId, url, urlNew, tempUrl, suffix, custom, callbackImmediately, &error)) {
            dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kTouchFileResult,
                                         windowId, QList<QUrl>() << url, true, error);
            return url.path();
        }

        return doTouchFilePractically(windowId, url, tempUrl) ? url.path() : QString();
    }
}

void FileOperationsEventReceiver::saveFileOperation(const QList<QUrl> &sourcesUrls,
                                                    const QList<QUrl> &targetUrls,
                                                    GlobalEventType type,
                                                    const QList<QUrl> &redoSourcesUrls,
                                                    const QList<QUrl> &redoTargetUrls,
                                                    const GlobalEventType redo,
                                                    const bool isUndo, const QUrl &templateUrl)
{
    // save operation by dbus
    QVariantMap values;
    values.insert("undoevent", QVariant::fromValue(static_cast<uint16_t>(type)));
    values.insert("undosources", QUrl::toStringList(sourcesUrls));
    values.insert("undotargets", QUrl::toStringList(targetUrls));
    values.insert("redoevent", QVariant::fromValue(static_cast<uint16_t>(redo)));
    values.insert("redosources", QUrl::toStringList(redoSourcesUrls));
    values.insert("redotargets", QUrl::toStringList(redoTargetUrls));
    if (templateUrl.isValid() && !UniversalUtils::urlEquals(templateUrl, sourcesUrls.first()))
        values.insert("templateurl", templateUrl.toString());
    if (!isUndo) {
        dpfSignalDispatcher->publish(GlobalEventType::kSaveOperator, values);
    } else {
        dpfSignalDispatcher->publish(GlobalEventType::kSaveRedoOperator, values);
    }
}

QUrl FileOperationsEventReceiver::checkTargetUrl(const QUrl &url)
{
    const QUrl &urlParent = DFMIO::DFMUtils::directParentUrl(url);
    if (!urlParent.isValid())
        return url;

    const QString &nameValid = FileUtils::nonExistSymlinkFileName(url, urlParent);
    if (!nameValid.isEmpty())
        return DFMIO::DFMUtils::buildFilePath(urlParent.toString().toStdString().c_str(),
                                              nameValid.toStdString().c_str(), nullptr);

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
                                                      const AbstractJobHandler::JobFlag flags,
                                                      DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback callbaskHandle)
{
    auto handle = doCopyFile(windowId, sources, target, flags, callbaskHandle);
    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kCopyType, handle);
}

void FileOperationsEventReceiver::handleOperationCut(quint64 windowId, const QList<QUrl> sources,
                                                     const QUrl target,
                                                     const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag flags,
                                                     DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback)
{
    auto handle = doCutFile(windowId, sources, target, flags, handleCallback);
    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kCutType, handle);
}

void FileOperationsEventReceiver::handleOperationDeletes(const quint64 windowId,
                                                         const QList<QUrl> sources,
                                                         const AbstractJobHandler::JobFlag flags,
                                                         DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback)
{
    DoDeleteErrorType erType { DoDeleteErrorType::kNoErrror };
    auto handle = doDeleteFile(windowId, sources, flags, handleCallback, true, erType);
    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kDeleteType, handle);
}

void FileOperationsEventReceiver::handleOperationCopy(const quint64 windowId,
                                                      const QList<QUrl> sources,
                                                      const QUrl target,
                                                      const AbstractJobHandler::JobFlag flags,
                                                      DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback,
                                                      const QVariant custom,
                                                      DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback)
{
    JobHandlePointer handle = doCopyFile(windowId, sources, target, flags, handleCallback);

    if (callback) {
        AbstractJobHandler::CallbackArgus args(new QMap<AbstractJobHandler::CallbackKey, QVariant>);
        args->insert(AbstractJobHandler::CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(AbstractJobHandler::CallbackKey::kJobHandle, QVariant::fromValue(handle));
        args->insert(AbstractJobHandler::CallbackKey::kCustom, custom);
        callback(args);
    }

    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kCopyType, handle);
}

void FileOperationsEventReceiver::handleOperationCut(const quint64 windowId,
                                                     const QList<QUrl> sources,
                                                     const QUrl target,
                                                     const AbstractJobHandler::JobFlag flags,
                                                     DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback,
                                                     const QVariant custom,
                                                     DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback)
{
    JobHandlePointer handle = doCutFile(windowId, sources, target, flags, handleCallback);
    if (callback && handle) {
        AbstractJobHandler::CallbackArgus args(new QMap<AbstractJobHandler::CallbackKey, QVariant>);
        args->insert(AbstractJobHandler::CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(AbstractJobHandler::CallbackKey::kJobHandle, QVariant::fromValue(handle));
        args->insert(AbstractJobHandler::CallbackKey::kCustom, custom);
        callback(args);
    }

    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kCutType, handle);
}

void FileOperationsEventReceiver::handleOperationDeletes(const quint64 windowId,
                                                         const QList<QUrl> sources,
                                                         const AbstractJobHandler::JobFlag flags,
                                                         DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback,
                                                         const QVariant custom,
                                                         DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback)
{
    DoDeleteErrorType erType { DoDeleteErrorType::kNoErrror };
    JobHandlePointer handle = doDeleteFile(windowId, sources, flags, handleCallback, true, erType);
    if (callback) {
        AbstractJobHandler::CallbackArgus args(new QMap<AbstractJobHandler::CallbackKey, QVariant>);
        args->insert(AbstractJobHandler::CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(AbstractJobHandler::CallbackKey::kJobHandle, QVariant::fromValue(handle));
        args->insert(AbstractJobHandler::CallbackKey::kCustom, custom);
        callback(args);
    }
    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kDeleteType, handle);
}

bool FileOperationsEventReceiver::handleOperationOpenFiles(const quint64 windowId, const QList<QUrl> urls)
{
    if (urls.isEmpty())
        return false;

    QString error;
    if (!urls.isEmpty() && !dfmbase::FileUtils::isLocalFile(urls.first())) {
        // hook events
        if (dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_OpenFileInPlugin", windowId, urls)) {
            dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenFilesResult, windowId, urls, true, error);
            return true;
        }
    }

    if (dpfHookSequence->run("dfmplugin_fileoperations", "hook_OpenLocalFiles", windowId, urls))
        return true;

    DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
    bool ok = fileHandler.openFiles(urls);
    if (!ok) {
        DFMBASE_NAMESPACE::GlobalEventType lastEvent = fileHandler.lastEventType();
        if (lastEvent != DFMBASE_NAMESPACE::GlobalEventType::kUnknowType) {
            if (lastEvent == DFMBASE_NAMESPACE::GlobalEventType::kDeleteFiles)
                dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kDeleteFiles, windowId, fileHandler.getInvalidPath(), AbstractJobHandler::JobFlag::kNoHint, nullptr);
            else if (lastEvent == DFMBASE_NAMESPACE::GlobalEventType::kMoveToTrash)
                dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kMoveToTrash, windowId, fileHandler.getInvalidPath(), AbstractJobHandler::JobFlag::kNoHint, nullptr);
        } else {
            // deal open file with custom dialog
            dpfSlotChannel->push("dfmplugin_utils", "slot_OpenWith_ShowDialog", windowId, urls);
            ok = true;
        }
    }

    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenFilesResult, windowId, urls, ok, error);
    return ok;
}

bool FileOperationsEventReceiver::handleOperationOpenFiles(const quint64 windowId, const QList<QUrl> urls, bool *ok)
{
    if (urls.isEmpty())
        return false;

    QString error;
    if (!urls.isEmpty() && !dfmbase::FileUtils::isLocalFile(urls.first())) {
        // hook events
        if (dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_OpenFileInPlugin", windowId, urls)) {
            if (ok)
                *ok = false;
            dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenFilesResult, windowId, urls, true, error);
            return true;
        }
    }
    return handleOperationOpenFiles(windowId, urls);
}

void FileOperationsEventReceiver::handleOperationOpenFiles(const quint64 windowId,
                                                           const QList<QUrl> urls,
                                                           const QVariant custom,
                                                           DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback)
{
    bool ok = handleOperationOpenFiles(windowId, urls);
    if (callback) {
        AbstractJobHandler::CallbackArgus args(new QMap<AbstractJobHandler::CallbackKey, QVariant>);
        args->insert(AbstractJobHandler::CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(AbstractJobHandler::CallbackKey::kSourceUrls, QVariant::fromValue(urls));
        args->insert(AbstractJobHandler::CallbackKey::kSuccessed, QVariant::fromValue(ok));
        args->insert(AbstractJobHandler::CallbackKey::kCustom, custom);
        callback(args);
    }
}

void FileOperationsEventReceiver::handleOperationOpenFilesByApp(const quint64 windowId,
                                                                const QList<QUrl> urls,
                                                                const QList<QString> apps,
                                                                const QVariant custom,
                                                                DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback)
{
    bool ok = handleOperationOpenFilesByApp(windowId, urls, apps);
    if (callback) {
        AbstractJobHandler::CallbackArgus args(new QMap<AbstractJobHandler::CallbackKey, QVariant>);
        args->insert(AbstractJobHandler::CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(AbstractJobHandler::CallbackKey::kSourceUrls, QVariant::fromValue(urls));
        args->insert(AbstractJobHandler::CallbackKey::kSuccessed, QVariant::fromValue(ok));
        args->insert(AbstractJobHandler::CallbackKey::kCustom, custom);
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
    if (!urls.isEmpty() && !dfmbase::FileUtils::isLocalFile(urls.first())) {
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
    if (!ok)
        fmWarning() << "open file by app error: "
                    << fileHandler.errorString()
                    << " app name: "
                    << app;

    // TODO:: file openFilesByApp finished need to send file openFilesByApp finished event
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenFilesByAppResult, windowId, urls, ok, error);
    return ok;
}

bool FileOperationsEventReceiver::handleOperationRenameFile(const quint64 windowId,
                                                            const QUrl oldUrl,
                                                            const QUrl newUrl,
                                                            const AbstractJobHandler::JobFlag flags)
{
    Q_UNUSED(windowId);
    bool ok = false;
    QString error;

    bool isSymLink { DFMIO::DFileInfo(oldUrl).attribute(DFMIO::DFileInfo::AttributeID::kStandardIsSymlink).toBool() };
    if (FileUtils::isDesktopFile(oldUrl) && !isSymLink)
        return doRenameDesktopFile(windowId, oldUrl, newUrl, flags);

    if (!dfmbase::FileUtils::isLocalFile(oldUrl)) {
        if (dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_RenameFile", windowId, oldUrl, newUrl, flags))
            return true;
    }

    // async fileinfo need wait file quer over todo:: liyigang
    FileInfoPointer toFileInfo = InfoFactory::create<FileInfo>(newUrl, Global::CreateFileInfoType::kCreateFileInfoSync);
    if (toFileInfo && toFileInfo->exists()) {
        dialogManager->showRenameNameSameErrorDialog(toFileInfo->nameOf(NameInfoType::kFileName));
        return false;
    }

    DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
    ok = fileHandler.renameFile(oldUrl, newUrl, true);
    if (!ok) {
        error = fileHandler.errorString();
        dialogManager->showRenameBusyErrDialog();
    }
    // TODO:: file renameFile finished need to send file renameFile finished event
    QMap<QUrl, QUrl> renamedFiles { { oldUrl, newUrl } };
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kRenameFileResult,
                                 windowId, renamedFiles, ok, error);
    if (ok) {
        ClipBoard::instance()->replaceClipboardUrl(oldUrl, newUrl);
        dpfSignalDispatcher->publish("dfmplugin_fileoperations", "signal_File_Rename",
                                     oldUrl, newUrl);
    }

    AbstractJobHandler::JobFlags tmFlags = flags;
    if (!tmFlags.testFlag(AbstractJobHandler::JobFlag::kRedo))
        saveFileOperation({ newUrl }, { oldUrl }, GlobalEventType::kRenameFile,
                          { oldUrl }, { newUrl }, GlobalEventType::kRenameFile,
                          tmFlags.testFlag(AbstractJobHandler::JobFlag::kRevocation));
    return ok;
}

void FileOperationsEventReceiver::handleOperationRenameFile(const quint64 windowId,
                                                            const QUrl oldUrl,
                                                            const QUrl newUrl,
                                                            const AbstractJobHandler::JobFlag flags,
                                                            const QVariant custom,
                                                            AbstractJobHandler::OperatorCallback callback)
{
    bool ok = handleOperationRenameFile(windowId, oldUrl, newUrl, flags);
    if (callback) {
        AbstractJobHandler::CallbackArgus args(new QMap<AbstractJobHandler::CallbackKey, QVariant>);
        args->insert(AbstractJobHandler::CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(AbstractJobHandler::CallbackKey::kSourceUrls, QVariant::fromValue(QList<QUrl>() << oldUrl));
        args->insert(AbstractJobHandler::CallbackKey::kTargets, QVariant::fromValue(QList<QUrl>() << oldUrl));
        args->insert(AbstractJobHandler::CallbackKey::kSuccessed, QVariant::fromValue(ok));
        args->insert(AbstractJobHandler::CallbackKey::kCustom, custom);
        callback(args);
    }
}

bool FileOperationsEventReceiver::handleOperationRenameFiles(const quint64 windowId, const QList<QUrl> urls, const QPair<QString, QString> pair, const bool replace)
{
    QMap<QUrl, QUrl> successUrls;
    bool ok = false;
    QString error;
    if (!urls.isEmpty() && !dfmbase::FileUtils::isLocalFile(urls.first())) {
        if (dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_RenameFiles", windowId, urls, pair, replace))
            return true;
    }

    RenameTypes type = RenameTypes::kBatchRepalce;
    if (!replace)
        type = RenameTypes::kBatchCustom;
    ok = doRenameFiles(windowId, urls, pair, {}, type, successUrls, error);

    // publish result
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kRenameFileResult,
                                 windowId, successUrls, ok, error);
    if (!successUrls.isEmpty()) {
        saveFileOperation(successUrls.values(), successUrls.keys(), GlobalEventType::kRenameFiles,
                          successUrls.keys(), successUrls.values(), GlobalEventType::kRenameFiles);
        for (const auto &source : successUrls.keys())
            dpfSignalDispatcher->publish("dfmplugin_fileoperations", "signal_File_Rename",
                                         source, successUrls.value(source));
    }

    return ok;
}

void FileOperationsEventReceiver::handleOperationRenameFiles(const quint64 windowId, const QList<QUrl> urls, const QPair<QString, QString> pair, const bool replace, const QVariant custom, AbstractJobHandler::OperatorCallback callback)
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
        saveFileOperation(successUrls.values(), successUrls.keys(), GlobalEventType::kRenameFiles,
                          successUrls.keys(), successUrls.values(), GlobalEventType::kRenameFiles);
}

bool FileOperationsEventReceiver::handleOperationRenameFiles(const quint64 windowId, const QList<QUrl> urls, const QPair<QString, AbstractJobHandler::FileNameAddFlag> pair)
{
    QMap<QUrl, QUrl> successUrls;
    bool ok = false;
    QString error;
    if (!urls.isEmpty() && !dfmbase::FileUtils::isLocalFile(urls.first())) {
        if (dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_RenameFilesAddText", windowId, urls, pair)) {

            dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kRenameFileResult,
                                         windowId, successUrls, true, error);
            if (!successUrls.isEmpty())
                saveFileOperation(successUrls.values(), successUrls.keys(), GlobalEventType::kRenameFiles,
                                  successUrls.keys(), successUrls.values(), GlobalEventType::kRenameFiles);

            return true;
        }
    }

    ok = doRenameFiles(windowId, urls, {}, pair, RenameTypes::kBatchAppend, successUrls, error);

    // publish result
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kRenameFileResult,
                                 windowId, successUrls, ok, error);
    if (!successUrls.isEmpty())
        saveFileOperation(successUrls.values(), successUrls.keys(), GlobalEventType::kRenameFiles,
                          successUrls.keys(), successUrls.values(), GlobalEventType::kRenameFiles);

    return ok;
}

void FileOperationsEventReceiver::handleOperationRenameFiles(const quint64 windowId, const QList<QUrl> urls, const QPair<QString, AbstractJobHandler::FileNameAddFlag> pair, const QVariant custom, AbstractJobHandler::OperatorCallback callback)
{
    QMap<QUrl, QUrl> successUrls;
    QString error;
    bool ok = doRenameFiles(windowId, urls, {}, pair, RenameTypes::kBatchAppend, successUrls, error, custom, callback);
    // publish result
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kRenameFileResult,
                                 windowId, successUrls, ok, error);
    if (!successUrls.isEmpty()) {
        saveFileOperation(successUrls.values(), successUrls.keys(), GlobalEventType::kRenameFiles,
                          successUrls.keys(), successUrls.values(), GlobalEventType::kRenameFiles);
        for (const auto &source : successUrls.keys())
            dpfSignalDispatcher->publish("dfmplugin_fileoperations", "signal_File_Rename",
                                         source, successUrls.value(source));
    }
}

bool FileOperationsEventReceiver::handleOperationMkdir(const quint64 windowId, const QUrl url)
{
    return doMkdir(windowId, url, QVariant(), nullptr);
}

void FileOperationsEventReceiver::handleOperationMkdir(const quint64 windowId,
                                                       const QUrl url,
                                                       const QVariant custom,
                                                       AbstractJobHandler::OperatorCallback callback)
{
    doMkdir(windowId, url, custom, callback);
}

bool FileOperationsEventReceiver::doTouchFilePractically(const quint64 windowId, const QUrl &url, const QUrl &tempUrl /*= QUrl()*/)
{
    QString error;

    DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
    auto templateUrl = fileHandler.touchFile(url, tempUrl);
    if (!templateUrl.isValid()) {
        error = fileHandler.errorString();
        dialogManager->showErrorDialog(tr("Failed to create the file"), error);
    }
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kTouchFileResult,
                                 windowId, QList<QUrl>() << url, templateUrl.isValid(), error);
    if (templateUrl.isValid())
        saveFileOperation({ url }, {}, GlobalEventType::kDeleteFiles, { url }, {}, GlobalEventType::kTouchFile, false, templateUrl);

    return templateUrl.isValid();
}

QString FileOperationsEventReceiver::handleOperationTouchFile(const quint64 windowId,
                                                              const QUrl url,
                                                              const CreateFileType fileType,
                                                              const QString suffix)
{
    return doTouchFilePremature(windowId, url, fileType, suffix, QVariant(), nullptr);
}

QString FileOperationsEventReceiver::handleOperationTouchFile(const quint64 windowId,
                                                              const QUrl url,
                                                              const QUrl tempUrl,
                                                              const QString suffix)
{
    return doTouchFilePremature(windowId, url, tempUrl, suffix, QVariant(), nullptr);
}

void FileOperationsEventReceiver::handleOperationTouchFile(const quint64 windowId,
                                                           const QUrl url,
                                                           const CreateFileType fileType,
                                                           const QString suffix,
                                                           const QVariant custom,
                                                           AbstractJobHandler::OperatorCallback callbackImmediately)
{
    doTouchFilePremature(windowId, url, fileType, suffix, custom, callbackImmediately);
}

void FileOperationsEventReceiver::handleOperationTouchFile(const quint64 windowId,
                                                           const QUrl url,
                                                           const QUrl tempUrl,
                                                           const QString suffix,
                                                           const QVariant custom,
                                                           AbstractJobHandler::OperatorCallback callbackImmediately)
{
    doTouchFilePremature(windowId, url, tempUrl, suffix, custom, callbackImmediately);
}

bool FileOperationsEventReceiver::handleOperationLinkFile(const quint64 windowId,
                                                          const QUrl url,
                                                          const QUrl link,
                                                          const bool force,
                                                          const bool silence)
{
    bool ok = false;
    QString error;
    if (!dfmbase::FileUtils::isLocalFile(url)) {
        if (dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_LinkFile", windowId, url, link, force, silence)) {
            dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kCreateSymlinkResult,
                                         windowId, QList<QUrl>() << url << link, true, error);
            return true;
        }
    }

    DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
    // check link
    if (force) {
        FileInfoPointer toInfo = InfoFactory::create<FileInfo>(link);
        if (toInfo && toInfo->exists()) {
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
        dialogManager->showErrorDialog(tr("link file error"), error);
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
                                                          DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback)
{
    bool ok = handleOperationLinkFile(windowId, url, link, force, silence);

    if (callback) {
        AbstractJobHandler::CallbackArgus args(new QMap<AbstractJobHandler::CallbackKey, QVariant>);
        args->insert(AbstractJobHandler::CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(AbstractJobHandler::CallbackKey::kSourceUrls, QVariant::fromValue(QList<QUrl>() << url));
        args->insert(AbstractJobHandler::CallbackKey::kTargets, QVariant::fromValue(QList<QUrl>() << link));
        args->insert(AbstractJobHandler::CallbackKey::kSuccessed, QVariant::fromValue(ok));
        args->insert(AbstractJobHandler::CallbackKey::kCustom, custom);
        callback(args);
    }
}

bool FileOperationsEventReceiver::handleOperationSetPermission(const quint64 windowId,
                                                               const QUrl url,
                                                               const QFileDevice::Permissions permissions)
{
    QString error;
    bool ok = false;
    if (!dfmbase::FileUtils::isLocalFile(url)) {
        // hook events
        bool hookOk = false;
        if (dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_SetPermission", windowId, url, permissions, &hookOk, &error)) {
            if (!hookOk)
                dialogManager->showErrorDialog(tr("Failed to modify file permissions"), error);
            dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kSetPermissionResult, windowId, QList<QUrl>() << url, hookOk, error);
            return hookOk;
        }
    }
    DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
    ok = fileHandler.setPermissions(url, permissions);
    if (!ok) {
        error = fileHandler.errorString();
        dialogManager->showErrorDialog(tr("Failed to modify file permissions"), error);
    }
    FileInfoPointer info = InfoFactory::create<FileInfo>(url);
    info->refresh();
    fmInfo("set file permissions successed, file : %s, permissions : %d !", url.path().toStdString().c_str(),
           static_cast<int>(permissions));
    // TODO:: set file permissions finished need to send set file permissions finished event
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kSetPermissionResult,
                                 windowId, QList<QUrl>() << url, ok, error);
    return ok;
}

void FileOperationsEventReceiver::handleOperationSetPermission(const quint64 windowId,
                                                               const QUrl url,
                                                               const QFileDevice::Permissions permissions, const QVariant custom,
                                                               DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback)
{
    bool ok = handleOperationSetPermission(windowId, url, permissions);
    if (callback) {
        AbstractJobHandler::CallbackArgus args(new QMap<AbstractJobHandler::CallbackKey, QVariant>);
        args->insert(AbstractJobHandler::CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(AbstractJobHandler::CallbackKey::kSourceUrls, QVariant::fromValue(QList<QUrl>() << url));
        args->insert(AbstractJobHandler::CallbackKey::kSuccessed, QVariant::fromValue(ok));
        args->insert(AbstractJobHandler::CallbackKey::kCustom, custom);
        callback(args);
    }
}

bool FileOperationsEventReceiver::handleOperationWriteToClipboard(const quint64 windowId,
                                                                  const DFMBASE_NAMESPACE::ClipBoard::ClipboardAction action,
                                                                  const QList<QUrl> urls)
{
    QString error;
    if (!urls.isEmpty() && !dfmbase::FileUtils::isLocalFile(urls.first())) {
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
        fmWarning() << " write to clipboard data is nullptr!!!!!!!";
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
    if (urls.count() > 0 && !dfmbase::FileUtils::isLocalFile(urls.first())) {
        if (dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_OpenInTerminal", windowId, urls)) {
            dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kOpenInTerminalResult,
                                         windowId, urls, true, error);
            return true;
        }
    }

    for (const auto &url : urls) {
        const QString &current_dir = QDir::currentPath();
        QDir::setCurrent(url.toLocalFile());
        if (!dfmbase::FileUtils::isLocalFile(url)) {
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

void FileOperationsEventReceiver::handleOperationSaveOperations(const QVariantMap values)
{
    OperationsStackProxy::instance().saveOperations(values);
}

void FileOperationsEventReceiver::handleOperationCleanSaveOperationsStack()
{
    OperationsStackProxy::instance().cleanOperations();
}

void FileOperationsEventReceiver::handleOperationRevocation(const quint64 windowId,
                                                            DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handle)
{
    const QVariantMap &ret { OperationsStackProxy::instance().revocationOperations() };
    revocation(windowId, ret, handle);
}

bool FileOperationsEventReceiver::handleOperationHideFiles(const quint64 windowId, const QList<QUrl> urls)
{
    Q_UNUSED(windowId)

    bool ok { true };
    for (const QUrl &url : urls) {
        FileInfoPointer info = InfoFactory::create<FileInfo>(url);
        if (info) {
            const QUrl &parentUrl = info->urlOf(UrlInfoType::kParentUrl);
            const QString &fileName = info->nameOf(NameInfoType::kFileName);

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
                                                           const QVariant custom, AbstractJobHandler::OperatorCallback callback)
{
    bool ok = handleOperationHideFiles(windowId, urls);
    if (callback) {
        AbstractJobHandler::CallbackArgus args(new QMap<AbstractJobHandler::CallbackKey, QVariant>);
        args->insert(AbstractJobHandler::CallbackKey::kWindowId, QVariant::fromValue(windowId));
        args->insert(AbstractJobHandler::CallbackKey::kSourceUrls, QVariant::fromValue(QList<QUrl>() << urls));
        args->insert(AbstractJobHandler::CallbackKey::kSuccessed, QVariant::fromValue(ok));
        args->insert(AbstractJobHandler::CallbackKey::kCustom, custom);
        callback(args);
    }
}

bool FileOperationsEventReceiver::handleShortCut(quint64, const QList<QUrl> &urls, const QUrl &rootUrl)
{
    if (urls.isEmpty())
        return false;
    const auto &currentFileInfo = InfoFactory::create<FileInfo>(rootUrl);
    // v5功能 判断当前目录是否有写权限，没有就提示权限错误
    if (urls.first().scheme() == Global::Scheme::kFile
        && !currentFileInfo->isAttributes(OptInfoType::kIsWritable)) {
        DialogManager::instance()->showNoPermissionDialog(urls);
        return true;
    }
    return false;
}

bool FileOperationsEventReceiver::handleShortCutPaste(quint64, const QList<QUrl> &, const QUrl &target)
{
    if (target.scheme() == Global::Scheme::kFile) {
        const auto &currentFileInfo = InfoFactory::create<FileInfo>(target);
        if (currentFileInfo && currentFileInfo->isAttributes(OptInfoType::kIsDir) && !currentFileInfo->isAttributes(OptInfoType::kIsWritable)) {
            // show error tip message
            DialogManager::instance()->showNoPermissionDialog(QList<QUrl>() << target);
            return true;
        }
    }
    return false;
}

void FileOperationsEventReceiver::handleOperationSaveRedoOperations(const QVariantMap &values)
{
    OperationsStackProxy::instance().SaveRedoOperations(values);
}

void FileOperationsEventReceiver::handleOperationCleanByUrls(const QList<QUrl> &urls)
{
    if (urls.isEmpty()) {
        qCWarning(logDFMBase) << "error : urls is empty!!";
        return;
    }
    QStringList strs;
    for (const auto &url : urls) {
        if (url.isValid())
            strs.append(url.toString());
    }
    OperationsStackProxy::instance().CleanOperationsByUrl(strs);
}

void FileOperationsEventReceiver::handleRecoveryOperationRedoRecovery(const quint64 windowId, AbstractJobHandler::OperatorHandleCallback handle)
{
    const QVariantMap &ret { OperationsStackProxy::instance().RevocationRedoOperations() };
    redo(windowId, ret, handle);
}

// ctrl + z 执行后保存当前的redo操作
void FileOperationsEventReceiver::handleSaveRedoOpt(const QString &token, const qint64 fileSize)
{
    QVariantMap ret;
    {
        QMutexLocker lk(&undoLock);
        if (!undoOpts.contains(token))
            return;
        ret = undoOpts.take(token);
    }
    if (ret.isEmpty())
        return;
    GlobalEventType undoEventType = static_cast<GlobalEventType>(ret.value("undoevent").value<uint16_t>());
    QList<QUrl> undoSources = QUrl::fromStringList(ret.value("undosources").toStringList());
    QList<QUrl> undoTargets = QUrl::fromStringList(ret.value("undotargets").toStringList());
    GlobalEventType redoEventType = static_cast<GlobalEventType>(ret.value("redoevent").value<uint16_t>());
    QList<QUrl> redoSources = QUrl::fromStringList(ret.value("redosources").toStringList());
    QList<QUrl> redoTargets = QUrl::fromStringList(ret.value("redotargets").toStringList());
    QUrl templateUrl = ret.value("templateurl", QUrl()).toUrl();
    qint64 compare = 0;
    if (templateUrl.isValid()) {
        auto info = InfoFactory::create<FileInfo>(templateUrl, Global::CreateFileInfoType::kCreateFileInfoSync);
        if (info)
            compare = info->size();
    }
    if (redoEventType != GlobalEventType::kTouchFile || fileSize == compare)
        saveFileOperation(redoSources, redoTargets, redoEventType, undoSources, undoTargets, undoEventType, true, templateUrl);
}

void FileOperationsEventReceiver::handleOperationUndoDeletes(const quint64 windowId, const QList<QUrl> &sources, const AbstractJobHandler::JobFlag flags, AbstractJobHandler::OperatorHandleCallback handleCallback, const QVariantMap &op)
{
    DoDeleteErrorType erType { DoDeleteErrorType::kNoErrror };
    auto handle = doDeleteFile(windowId, sources, flags, handleCallback, false, erType);
    if (!handle && erType == DoDeleteErrorType::kNullPtr) {
        // 此次ctrl+z回撤失败，重新保存这次操作
        dpfSignalDispatcher->publish(GlobalEventType::kSaveOperator, op);
        return;
    }
    connect(handle.get(), &AbstractJobHandler::requestSaveRedoOperation, this,
            &FileOperationsEventReceiver::handleSaveRedoOpt, Qt::QueuedConnection);
    {
        QMutexLocker lk(&undoLock);
        undoOpts.insert(QString::number(quintptr(handle.get()), 16), op);
    }
    copyMoveJob->initArguments(handle);
    if (handleCallback)
        handleCallback(handle);
    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kDeleteType, handle);
}

void FileOperationsEventReceiver::handleOperationUndoCut(const quint64 windowId, const QList<QUrl> &sources, const QUrl target, const AbstractJobHandler::JobFlag flags, AbstractJobHandler::OperatorHandleCallback handleCallback, const QVariantMap &op)
{
    auto handle = doCutFile(windowId, sources, target, flags, handleCallback, false);
    if (!handle)
        return;
    connect(handle.get(), &AbstractJobHandler::requestSaveRedoOperation, this,
            &FileOperationsEventReceiver::handleSaveRedoOpt);
    {
        QMutexLocker lk(&undoLock);
        undoOpts.insert(QString::number(quintptr(handle.get()), 16), op);
    }
    copyMoveJob->initArguments(handle);
    if (handleCallback)
        handleCallback(handle);
    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kCutType, handle);
}

void FileOperationsEventReceiver::handleOperationFilesPreview(const quint64 windowId,
                                                              const QList<QUrl> &selectUrls,
                                                              const QList<QUrl> &dirUrls)
{
    if (selectUrls.isEmpty() || dirUrls.isEmpty())
        return;

    QString selectListStr;
    QString dirListStr;

    for (auto url : selectUrls) {
        selectListStr.append(url.toString());
        selectListStr.append(";");
    }
    if (!selectListStr.isEmpty())
        selectListStr.chop(1);

    for (auto url : dirUrls) {
        dirListStr.append(url.toString());
        dirListStr.append(";");
    }
    if (!dirListStr.isEmpty())
        dirListStr.chop(1);

    QStringList args;
    args << QString::number(windowId) << selectListStr << dirListStr;
    QString cmd("/usr/libexec/dde-file-manager-preview");
    QProcess::startDetached(cmd, args);
}

bool FileOperationsEventReceiver::handleIsSubFile(const QUrl &parent, const QUrl &sub)
{
    if (parent.scheme() != Global::Scheme::kFile)
        return false;

    return sub.path().startsWith(parent.path());
}
}   // namespace dfmplugin_fileoperations
