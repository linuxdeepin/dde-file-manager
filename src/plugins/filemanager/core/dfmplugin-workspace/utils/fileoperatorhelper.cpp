// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileoperatorhelper.h"
#include "workspacehelper.h"
#include "events/workspaceeventcaller.h"
#include "views/fileview.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <dfm-framework/dpf.h>

Q_DECLARE_METATYPE(QList<QUrl> *)

DFMGLOBAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_workspace;
FileOperatorHelper *FileOperatorHelper::instance()
{
    static FileOperatorHelper helper;
    return &helper;
}

void FileOperatorHelper::touchFolder(const FileView *view)
{
    auto windowId = WorkspaceHelper::instance()->windowId(view);
    dpfSignalDispatcher->publish(GlobalEventType::kMkdir,
                                 windowId,
                                 view->rootUrl(),
                                 GlobalEventType::kMkdir,
                                 callBack);
}

void FileOperatorHelper::touchFiles(const FileView *view, const CreateFileType type, QString suffix)
{
    const quint64 windowId = WorkspaceHelper::instance()->windowId(view);
    const QUrl &url = view->rootUrl();

    dpfSignalDispatcher->publish(GlobalEventType::kTouchFile,
                                 windowId,
                                 url,
                                 type,
                                 suffix,
                                 GlobalEventType::kTouchFile,
                                 callBack);
}

void FileOperatorHelper::touchFiles(const FileView *view, const QUrl &source)
{
    const quint64 windowId = WorkspaceHelper::instance()->windowId(view);
    const QUrl &url = view->rootUrl();

    dpfSignalDispatcher->publish(GlobalEventType::kTouchFile,
                                 windowId,
                                 url,
                                 source,
                                 QString(),
                                 GlobalEventType::kTouchFile,
                                 callBack);
}

void FileOperatorHelper::openFiles(const FileView *view)
{
    openFiles(view, view->selectedUrlList());
}

void FileOperatorHelper::openFiles(quint64 winId, const QList<QUrl> &urls)
{
    DirOpenMode openMode;
    if (Application::instance()->appAttribute(Application::kAllwayOpenOnNewWindow).toBool()) {
        openMode = DirOpenMode::kOpenNewWindow;
    } else {
        openMode = DirOpenMode::kOpenInCurrentWindow;
    }

    for (const QUrl &url : urls) {
        const FileInfoPointer &fileInfoPtr = InfoFactory::create<FileInfo>(url);
        if (fileInfoPtr) {
            if (!fileInfoPtr->exists()) {
                // show alert
                QString fileName = fileInfoPtr->nameOf(NameInfoType::kFileName);
                QFont f;
                f.setPixelSize(16);
                QFontMetrics fm(f);
                fileName = fm.elidedText(fileName, Qt::ElideMiddle, 200);

                UniversalUtils::notifyMessage(QObject::tr("dde-file-manager"),
                                              tr("Failed to open %1, which may be moved or renamed").arg(fileName));
                continue;
            }

            if (fileInfoPtr->isAttributes(OptInfoType::kIsDir)) {
                QUrl dirUrl = url;
                if (fileInfoPtr->isAttributes(OptInfoType::kIsSymLink))
                    dirUrl = QUrl::fromLocalFile(fileInfoPtr->pathOf(PathInfoType::kSymLinkTarget));

                auto flag = DConfigManager::instance()->value(kViewDConfName,
                                                              kOpenFolderWindowsInASeparateProcess, false)
                                    .toBool();
                if (openMode == DirOpenMode::kOpenNewWindow /*|| (flag && FileManagerWindowsManager::instance().containsCurrentUrl(dirUrl, view->window()))*/) {
                    WorkspaceEventCaller::sendOpenWindow({ dirUrl }, !flag);
                } else {
                    WorkspaceEventCaller::sendChangeCurrentUrl(winId, dirUrl);
                }
                continue;
            }
        }

        const QList<QUrl> &openUrls = { url };
        dpfSignalDispatcher->publish(GlobalEventType::kOpenFiles,
                                     winId,
                                     openUrls);
    }
}

void FileOperatorHelper::openFiles(const FileView *view, const QList<QUrl> &urls)
{
    DirOpenMode openMode = view->currentDirOpenMode();

    openFilesByMode(view, urls, openMode);
}

void FileOperatorHelper::openFilesByMode(const FileView *view, const QList<QUrl> &urls, const DirOpenMode mode)
{
    auto windowId = WorkspaceHelper::instance()->windowId(view);

    for (const QUrl &url : urls) {
        const FileInfoPointer &fileInfoPtr = InfoFactory::create<FileInfo>(url);
        if (fileInfoPtr) {
            if (!fileInfoPtr->exists()) {
                // show alert
                QString fileName = fileInfoPtr->nameOf(NameInfoType::kFileName);
                QFont f;
                f.setPixelSize(16);
                QFontMetrics fm(f);
                fileName = fm.elidedText(fileName, Qt::ElideMiddle, 200);

                UniversalUtils::notifyMessage(QObject::tr("dde-file-manager"),
                                              tr("Failed to open %1, which may be moved or renamed").arg(fileName));
                continue;
            }

            if (fileInfoPtr->isAttributes(OptInfoType::kIsDir)) {
                QUrl dirUrl = url;
                if (fileInfoPtr->isAttributes(OptInfoType::kIsSymLink))
                    dirUrl = QUrl::fromLocalFile(fileInfoPtr->pathOf(PathInfoType::kSymLinkTarget));

                auto flag = DConfigManager::instance()->value(kViewDConfName,
                                                              kOpenFolderWindowsInASeparateProcess, false)
                                    .toBool();
                if (mode == DirOpenMode::kOpenNewWindow || (flag && FileManagerWindowsManager::instance().containsCurrentUrl(dirUrl, view->window()))) {
                    WorkspaceEventCaller::sendOpenWindow({ dirUrl }, !flag);
                } else {
                    WorkspaceEventCaller::sendChangeCurrentUrl(windowId, dirUrl);
                }
                continue;
            }
        }

        const QList<QUrl> &openUrls = { url };
        dpfSignalDispatcher->publish(GlobalEventType::kOpenFiles,
                                     windowId,
                                     openUrls);
    }
}

void FileOperatorHelper::openFilesByApp(const FileView *view, const QList<QUrl> &urls, const QList<QString> &apps)
{
    auto windowId = WorkspaceHelper::instance()->windowId(view);
    dpfSignalDispatcher->publish(GlobalEventType::kOpenFilesByApp,
                                 windowId,
                                 urls,
                                 apps);
}

void FileOperatorHelper::renameFile(const FileView *view, const QUrl &oldUrl, const QUrl &newUrl)
{
    auto windowId = WorkspaceHelper::instance()->windowId(view);
    dpfSignalDispatcher->publish(GlobalEventType::kRenameFile,
                                 windowId,
                                 oldUrl,
                                 newUrl,
                                 DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kNoHint);
}

void FileOperatorHelper::copyFiles(const FileView *view)
{
    QList<QUrl> selectedUrls = view->selectedTreeViewUrlList();
    // trans url to local
    QList<QUrl> urls {};
    bool ok = UniversalUtils::urlsTransformToLocal(selectedUrls, &urls);
    if (ok && !urls.isEmpty())
        selectedUrls = urls;

    if (selectedUrls.size() == 1) {
        const FileInfoPointer &fileInfo = InfoFactory::create<FileInfo>(selectedUrls.first());
        if (!fileInfo || !fileInfo->isAttributes(OptInfoType::kIsReadable))
            return;
    }

    if (selectedUrls.isEmpty())
        return;

    fmInfo() << "Copy shortcut key to clipboard, selected urls: " << selectedUrls.first()
             << ", selected count: " << selectedUrls.size()
             << ", current dir: " << view->rootUrl();

    auto windowId = WorkspaceHelper::instance()->windowId(view);

    dpfSignalDispatcher->publish(GlobalEventType::kWriteUrlsToClipboard,
                                 windowId,
                                 ClipBoard::ClipboardAction::kCopyAction,
                                 selectedUrls);
}

void FileOperatorHelper::cutFiles(const FileView *view)
{
    const FileInfoPointer &fileInfo = InfoFactory::create<FileInfo>(view->rootUrl());
    if (!fileInfo || !fileInfo->isAttributes(OptInfoType::kIsWritable))
        return;
    QList<QUrl> selectedUrls = view->selectedTreeViewUrlList();
    QList<QUrl> urls {};
    bool ok = UniversalUtils::urlsTransformToLocal(selectedUrls, &urls);
    if (ok && !urls.isEmpty())
        selectedUrls = urls;

    if (selectedUrls.isEmpty())
        return;

    fmInfo() << "Cut shortcut key to clipboard, selected urls: " << selectedUrls.first()
             << ", selected count: " << selectedUrls.size()
             << ", current dir: " << view->rootUrl();

    auto windowId = WorkspaceHelper::instance()->windowId(view);
    dpfSignalDispatcher->publish(GlobalEventType::kWriteUrlsToClipboard,
                                 windowId,
                                 ClipBoard::ClipboardAction::kCutAction,
                                 selectedUrls);
}

void FileOperatorHelper::pasteFiles(const FileView *view)
{
    fmInfo() << "Paste file by clipboard and current dir: " << view->rootUrl();
    auto action = ClipBoard::instance()->clipboardAction();
    // trash dir can't paste files for copy
    if (FileUtils::isTrashFile(view->rootUrl()))
        return;
    auto sourceUrls = ClipBoard::instance()->clipboardFileUrlList();
    auto windowId = WorkspaceHelper::instance()->windowId(view);

    if (ClipBoard::kCopyAction == action) {
        dpfSignalDispatcher->publish(GlobalEventType::kCopy,
                                     windowId,
                                     sourceUrls,
                                     view->rootUrl(),
                                     AbstractJobHandler::JobFlag::kNoHint, nullptr);
    } else if (ClipBoard::kCutAction == action) {

        if (ClipBoard::supportCut()) {
            dpfSignalDispatcher->publish(GlobalEventType::kCutFile,
                                         windowId,
                                         sourceUrls,
                                         view->rootUrl(),
                                         AbstractJobHandler::JobFlag::kNoHint, nullptr);
            ClipBoard::clearClipboard();
        }
    } else if (action == ClipBoard::kRemoteCopiedAction) {   // 远程协助
        fmInfo() << "Remote Assistance Copy: set Current Url to Clipboard";
        ClipBoard::setCurUrlToClipboardForRemote(view->rootUrl());
    } else if (ClipBoard::kRemoteAction == action) {
        dpfSignalDispatcher->publish(GlobalEventType::kCopy,
                                     windowId,
                                     sourceUrls,
                                     view->rootUrl(),
                                     AbstractJobHandler::JobFlag::kCopyRemote,
                                     nullptr, nullptr,
                                     QVariant(), nullptr);
    } else {
        fmWarning() << "Unknown clipboard past action:" << action << " urls:" << sourceUrls;
    }
}

void FileOperatorHelper::undoFiles(const FileView *view)
{
    fmInfo() << "Undo files in the directory: " << view->rootUrl();
    auto windowId = WorkspaceHelper::instance()->windowId(view);

    dpfSignalDispatcher->publish(GlobalEventType::kRevocation,
                                 windowId, undoCallBack);
}

void FileOperatorHelper::moveToTrash(const FileView *view)
{
    const QList<QUrl> selectedUrls = view->selectedTreeViewUrlList();
    if (selectedUrls.isEmpty())
        return;

    fmInfo() << "Move files to trash, selected urls: " << selectedUrls
             << ", current dir: " << view->rootUrl();

    auto windowId = WorkspaceHelper::instance()->windowId(view);

    dpfSignalDispatcher->publish(GlobalEventType::kMoveToTrash,
                                 windowId,
                                 selectedUrls,
                                 AbstractJobHandler::JobFlag::kNoHint, nullptr);
}

void FileOperatorHelper::moveToTrash(const FileView *view, const QList<QUrl> &urls)
{
    if (urls.isEmpty())
        return;

    fmInfo() << "Move files to trash, files urls: " << urls
             << ", current dir: " << view->rootUrl();

    auto windowId = WorkspaceHelper::instance()->windowId(view);
    dpfSignalDispatcher->publish(GlobalEventType::kMoveToTrash,
                                 windowId,
                                 urls,
                                 AbstractJobHandler::JobFlag::kNoHint, nullptr);
}

void FileOperatorHelper::deleteFiles(const FileView *view)
{
    const QList<QUrl> selectedUrls = view->selectedTreeViewUrlList();
    if (selectedUrls.isEmpty())
        return;

    fmInfo() << "Delete files, selected urls: " << selectedUrls
             << ", current dir: " << view->rootUrl();

    auto windowId = WorkspaceHelper::instance()->windowId(view);
    dpfSignalDispatcher->publish(GlobalEventType::kDeleteFiles,
                                 windowId,
                                 selectedUrls,
                                 AbstractJobHandler::JobFlag::kNoHint, nullptr);
}

void FileOperatorHelper::createSymlink(const FileView *view, QUrl targetParent)
{
    if (targetParent.isEmpty())
        targetParent = view->rootUrl();

    auto windowId = WorkspaceHelper::instance()->windowId(view);

    for (const QUrl &fileUrl : view->selectedUrlList()) {
        QString linkName = FileUtils::nonExistSymlinkFileName(fileUrl, targetParent);
        QUrl linkUrl;
        linkUrl.setScheme(targetParent.scheme());
        linkUrl.setPath(targetParent.path() + "/" + linkName);

        dpfSignalDispatcher->publish(GlobalEventType::kCreateSymlink,
                                     windowId,
                                     fileUrl,
                                     linkUrl,
                                     false,
                                     false);
    }
}

void FileOperatorHelper::openInTerminal(const FileView *view)
{
    auto windowId = WorkspaceHelper::instance()->windowId(view);
    QList<QUrl> urls = view->selectedUrlList();
    if (urls.isEmpty())
        urls.append(view->rootUrl());
    dpfSignalDispatcher->publish(GlobalEventType::kOpenInTerminal,
                                 windowId,
                                 urls);
}

void FileOperatorHelper::showFilesProperty(const FileView *view)
{
    QList<QUrl> urls = view->selectedUrlList();
    if (urls.isEmpty())
        urls.append(view->rootUrl());
    dpfSlotChannel->push("dfmplugin_propertydialog", "slot_PropertyDialog_Show", urls, QVariantHash());
}

void FileOperatorHelper::sendBluetoothFiles(const FileView *view)
{
    QList<QUrl> urls = view->selectedTreeViewUrlList();
    if (urls.isEmpty())
        return;

    fmInfo() << "Send to bluetooth, selected urls: " << urls
             << ", current dir: " << view->rootUrl();

    QStringList paths;
    for (const auto &u : urls)
        paths << u.path();
    dpfSlotChannel->push("dfmplugin_utils", "slot_Bluetooth_SendFiles", paths);
}

void FileOperatorHelper::previewFiles(const FileView *view, const QList<QUrl> &selectUrls, const QList<QUrl> &currentDirUrls)
{
    quint64 winID = WorkspaceHelper::instance()->windowId(view);
    dpfSlotChannel->push("dfmplugin_filepreview", "slot_PreviewDialog_Show", winID, selectUrls, currentDirUrls);
}

void FileOperatorHelper::dropFiles(const FileView *view, const Qt::DropAction &action, const QUrl &targetUrl, const QList<QUrl> &urls)
{
    auto windowId = WorkspaceHelper::instance()->windowId(view);
    if (action == Qt::MoveAction) {
        dpfSignalDispatcher->publish(GlobalEventType::kCutFile,
                                     windowId,
                                     urls,
                                     targetUrl,
                                     AbstractJobHandler::JobFlag::kNoHint, nullptr);
    } else {
        // default is copy file
        dpfSignalDispatcher->publish(GlobalEventType::kCopy,
                                     windowId,
                                     urls,
                                     targetUrl,
                                     AbstractJobHandler::JobFlag::kNoHint, nullptr);
    }
}

void FileOperatorHelper::renameFilesByReplace(const QWidget *sender, const QList<QUrl> &urlList, const QPair<QString, QString> &replacePair)
{
    fmInfo() << "Rename files with replace string: " << replacePair
             << ", files urls: " << urlList;

    auto windowId = WorkspaceHelper::instance()->windowId(sender);
    dpfSignalDispatcher->publish(GlobalEventType::kRenameFiles,
                                 windowId,
                                 urlList,
                                 replacePair,
                                 true);
}

void FileOperatorHelper::renameFilesByAdd(const QWidget *sender, const QList<QUrl> &urlList, const QPair<QString, AbstractJobHandler::FileNameAddFlag> &addPair)
{
    fmInfo() << "Rename files with add string: " << addPair
             << ", files urls: " << urlList;

    auto windowId = WorkspaceHelper::instance()->windowId(sender);
    dpfSignalDispatcher->publish(GlobalEventType::kRenameFiles,
                                 windowId,
                                 urlList,
                                 addPair);
}

void FileOperatorHelper::renameFilesByCustom(const QWidget *sender, const QList<QUrl> &urlList, const QPair<QString, QString> &customPair)
{
    fmInfo() << "Rename files with custom string: " << customPair
             << ", files urls: " << urlList;

    auto windowId = WorkspaceHelper::instance()->windowId(sender);
    dpfSignalDispatcher->publish(GlobalEventType::kRenameFiles,
                                 windowId,
                                 urlList,
                                 customPair,
                                 false);
}

void FileOperatorHelper::redoFiles(const FileView *view)
{
    fmInfo() << "Undo files in the directory: " << view->rootUrl();
    auto windowId = WorkspaceHelper::instance()->windowId(view);

    dpfSignalDispatcher->publish(GlobalEventType::kRedo,
                                 windowId, undoCallBack);
}

FileOperatorHelper::FileOperatorHelper(QObject *parent)
    : QObject(parent)
{
    callBack = std::bind(&FileOperatorHelper::callBackFunction, this, std::placeholders::_1);
    undoCallBack = std::bind(&FileOperatorHelper::undoCallBackFunction, this, std::placeholders::_1);
}

void FileOperatorHelper::callBackFunction(const AbstractJobHandler::CallbackArgus args)
{
    const QVariant &customValue = args->value(AbstractJobHandler::CallbackKey::kCustom);
    GlobalEventType type = static_cast<GlobalEventType>(customValue.toInt());

    switch (type) {
    case kMkdir: {
        quint64 windowID = args->value(AbstractJobHandler::CallbackKey::kWindowId).toULongLong();
        QList<QUrl> sourceUrlList = args->value(AbstractJobHandler::CallbackKey::kSourceUrls).value<QList<QUrl>>();
        if (sourceUrlList.isEmpty())
            break;

        QList<QUrl> targetUrlList = args->value(AbstractJobHandler::CallbackKey::kTargets).value<QList<QUrl>>();
        if (targetUrlList.isEmpty())
            break;

        QUrl rootUrl = sourceUrlList.first();
        QUrl newFolder = targetUrlList.first();
        WorkspaceHelper::kSelectionAndRenameFile[windowID] = qMakePair(rootUrl, newFolder);
        break;
    }
    case kTouchFile: {
        quint64 windowID = args->value(AbstractJobHandler::CallbackKey::kWindowId).toULongLong();
        QList<QUrl> sourceUrlList = args->value(AbstractJobHandler::CallbackKey::kSourceUrls).value<QList<QUrl>>();
        if (sourceUrlList.isEmpty())
            break;

        QList<QUrl> targetUrlList = args->value(AbstractJobHandler::CallbackKey::kTargets).value<QList<QUrl>>();
        if (targetUrlList.isEmpty())
            break;

        QUrl rootUrl = sourceUrlList.first();
        QUrl newFile = targetUrlList.first();
        WorkspaceHelper::kSelectionAndRenameFile[windowID] = qMakePair(rootUrl, newFile);
        break;
    }
    default:
        break;
    }
}

void FileOperatorHelper::undoCallBackFunction(QSharedPointer<AbstractJobHandler> handler)
{
    connect(handler.data(), &AbstractJobHandler::finishedNotify, this, [=](const JobInfoPointer jobInfo) {
        AbstractJobHandler::JobType type = static_cast<AbstractJobHandler::JobType>(jobInfo->value(AbstractJobHandler::kJobtypeKey).toInt());
        if (type == AbstractJobHandler::JobType::kCutType) {
            QList<QUrl> targetUrls(jobInfo->value(AbstractJobHandler::kCompleteTargetFilesKey).value<QList<QUrl>>());
            WorkspaceHelper::instance()->setUndoFiles(targetUrls);
        }
    });

    connect(handler.data(), &AbstractJobHandler::workerFinish, this, [=]() {
        WorkspaceHelper::instance()->setUndoFiles({});
    });
}
