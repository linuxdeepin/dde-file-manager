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
            qCritical() << "create parent file info failed!";
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
    if (!ret.contains("event") || !ret.contains("sources") || !ret.contains("targets"))
        return false;
    GlobalEventType eventType = static_cast<GlobalEventType>(ret.value("event").value<uint16_t>());
    QList<QUrl> sources = QUrl::fromStringList(ret.value("sources").toStringList());
    QList<QUrl> targets = QUrl::fromStringList(ret.value("targets").toStringList());
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
        handleOperationCut(windowId, sources, targets.first(), AbstractJobHandler::JobFlag::kRevocation, handle);
        break;
    case kDeleteFiles:
        handleOperationDeletes(windowId, sources, AbstractJobHandler::JobFlag::kRevocation, handle);
        break;
    case kMoveToTrash:
        TrashFileEventReceiver::instance()->handleOperationMoveToTrash(windowId, sources, AbstractJobHandler::JobFlag::kRevocation, handle);
        break;
    case kRestoreFromTrash:
        TrashFileEventReceiver::instance()->handleOperationRestoreFromTrash(windowId, sources, QUrl(), AbstractJobHandler::JobFlag::kRevocation, handle);
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
                                                const QVariant custom, DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback)
{
    DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
    bool ok = false;
    switch (type) {
    case RenameTypes::kBatchRepalce: {
        QMap<QUrl, QUrl> needDealUrls = FileUtils::fileBatchReplaceText(urls, pair);
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

        if (!flags.testFlag(AbstractJobHandler::JobFlag::kRevocation)) {
            const QString path = QFileInfo(desktopPath).absoluteDir().absoluteFilePath(oldName);
            saveFileOperation({ oldUrl }, { QUrl::fromLocalFile(path) }, GlobalEventType::kRenameFile);
        }
        return true;
    }

    return false;
}

JobHandlePointer FileOperationsEventReceiver::doCopyFile(const quint64 windowId, const QList<QUrl> sources, const QUrl target,
                                                         const AbstractJobHandler::JobFlags flags, DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback callbaskHandle)
{
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

JobHandlePointer FileOperationsEventReceiver::doCutFile(quint64 windowId, const QList<QUrl> sources, const QUrl target, const AbstractJobHandler::JobFlags flags, AbstractJobHandler::OperatorHandleCallback handleCallback)
{
    if (sources.isEmpty())
        return nullptr;

    // cut file to file current dir
    if (FileUtils::isSameFile(UrlRoute::urlParent(sources[0]), target)) {
        qWarning() << "cut file to same dir!!!!!!!!!";
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

    JobHandlePointer handle = copyMoveJob->cut(sourcesTrans, target, flags);
    if (handleCallback)
        handleCallback(handle);

    return handle;
}

JobHandlePointer FileOperationsEventReceiver::doDeleteFile(const quint64 windowId, const QList<QUrl> sources, const AbstractJobHandler::JobFlags flags, AbstractJobHandler::OperatorHandleCallback handleCallback)
{
    if (sources.isEmpty())
        return nullptr;

    if (SystemPathUtil::instance()->checkContainsSystemPath(sources)) {
        DialogManagerInstance->showDeleteSystemPathWarnDialog(windowId);
        return nullptr;
    }

    if (!dfmbase::FileUtils::isLocalFile(sources.first())) {
        // hook events
        if (dpfHookSequence->run("dfmplugin_fileoperations", "hook_Operation_DeleteFile", windowId, sources, flags)) {
            return nullptr;
        }
    }

    // Delete local file with shift+delete, show a confirm dialog.
    if (!flags.testFlag(AbstractJobHandler::JobFlag::kRevocation) && DialogManagerInstance->showDeleteFilesDialog(sources) != QDialog::Accepted)
        return nullptr;

    JobHandlePointer handle = copyMoveJob->deletes(sources, flags);
    if (handleCallback)
        handleCallback(handle);

    return handle;
}

JobHandlePointer FileOperationsEventReceiver::doCleanTrash(const quint64 windowId, const QList<QUrl> sources, const AbstractJobHandler::DeleteDialogNoticeType deleteNoticeType, AbstractJobHandler::OperatorHandleCallback handleCallback)
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

bool FileOperationsEventReceiver::doMkdir(const quint64 windowId, const QUrl url,
                                          const QVariant custom,
                                          AbstractJobHandler::OperatorCallback callback)
{
    const QString newPath = newDocmentName(url, QString(), CreateFileType::kCreateFileTypeFolder);
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
    saveFileOperation({ targetUrl }, {}, GlobalEventType::kDeleteFiles);

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

QString FileOperationsEventReceiver::doTouchFilePremature(const quint64 windowId, const QUrl url, const CreateFileType fileType, const QString suffix,
                                                          const QVariant custom, AbstractJobHandler::OperatorCallback callbackImmediately)
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

QString FileOperationsEventReceiver::doTouchFilePremature(const quint64 windowId, const QUrl url, const QUrl tempUrl, const QString suffix, const QVariant custom, AbstractJobHandler::OperatorCallback callbackImmediately)
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
                                                      const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                      DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback callbaskHandle)
{
    auto handle = doCopyFile(windowId, sources, target, flags, callbaskHandle);
    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kCopyType, handle);
}

void FileOperationsEventReceiver::handleOperationCut(quint64 windowId, const QList<QUrl> sources, const QUrl target, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                     DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback)
{
    auto handle = doCutFile(windowId, sources, target, flags, handleCallback);
    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kCutType, handle);
}

void FileOperationsEventReceiver::handleOperationDeletes(const quint64 windowId,
                                                         const QList<QUrl> sources,
                                                         const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                         DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback)
{
    auto handle = doDeleteFile(windowId, sources, flags, handleCallback);
    FileOperationsEventHandler::instance()->handleJobResult(AbstractJobHandler::JobType::kDeleteType, handle);
}

void FileOperationsEventReceiver::handleOperationCopy(const quint64 windowId,
                                                      const QList<QUrl> sources,
                                                      const QUrl target,
                                                      const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
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
                                                     const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
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
                                                         const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
                                                         DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback handleCallback,
                                                         const QVariant custom,
                                                         DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callback)
{
    JobHandlePointer handle = doDeleteFile(windowId, sources, flags, handleCallback);
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
                dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kDeleteFiles, windowId, urls, AbstractJobHandler::JobFlag::kNoHint, nullptr);
            else if (lastEvent == DFMBASE_NAMESPACE::GlobalEventType::kMoveToTrash)
                dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kMoveToTrash, windowId, urls, AbstractJobHandler::JobFlag::kNoHint, nullptr);
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
    ok = fileHandler.renameFile(oldUrl, newUrl);
    if (!ok) {
        error = fileHandler.errorString();
        dialogManager->showRenameBusyErrDialog();
    }
    // TODO:: file renameFile finished need to send file renameFile finished event
    QMap<QUrl, QUrl> renamedFiles { { oldUrl, newUrl } };
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kRenameFileResult,
                                 windowId, renamedFiles, ok, error);
    if (ok)
        ClipBoard::instance()->replaceClipboardUrl(oldUrl, newUrl);

    if (!flags.testFlag(AbstractJobHandler::JobFlag::kRevocation))
        saveFileOperation({ newUrl }, { oldUrl }, GlobalEventType::kRenameFile);
    return ok;
}

void FileOperationsEventReceiver::handleOperationRenameFile(const quint64 windowId,
                                                            const QUrl oldUrl,
                                                            const QUrl newUrl,
                                                            const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags,
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
    if (!successUrls.isEmpty())
        saveFileOperation(successUrls.values(), successUrls.keys(), GlobalEventType::kRenameFiles);

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
        saveFileOperation(successUrls.values(), successUrls.keys(), GlobalEventType::kRenameFiles);
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

void FileOperationsEventReceiver::handleOperationRenameFiles(const quint64 windowId, const QList<QUrl> urls, const QPair<QString, AbstractJobHandler::FileNameAddFlag> pair, const QVariant custom, AbstractJobHandler::OperatorCallback callback)
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
    return doMkdir(windowId, url, QVariant(), nullptr);
}

void FileOperationsEventReceiver::handleOperationMkdir(const quint64 windowId,
                                                       const QUrl url,
                                                       const QVariant custom,
                                                       AbstractJobHandler::OperatorCallback callback)
{
    doMkdir(windowId, url, custom, callback);
}

bool FileOperationsEventReceiver::doTouchFilePractically(const quint64 windowId, const QUrl url, const QUrl &tempUrl /*= QUrl()*/)
{
    QString error;

    DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
    bool ok = fileHandler.touchFile(url, tempUrl);
    if (!ok) {
        error = fileHandler.errorString();
        dialogManager->showErrorDialog(tr("Failed to create the file"), error);
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
    qInfo("set file permissions successed, file : %s, permissions : %d !", url.path().toStdString().c_str(),
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
}   // namespace dfmplugin_fileoperations
