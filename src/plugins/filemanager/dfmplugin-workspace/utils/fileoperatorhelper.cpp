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

#include <QClipboard>
#include <QMimeData>
#include <QImage>
#include <QDateTime>
#include <QApplication>

Q_DECLARE_METATYPE(QList<QUrl> *)

DFMGLOBAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_workspace;
using namespace GlobalDConfDefines::ConfigPath;
using namespace GlobalDConfDefines::BaseConfig;

FileOperatorHelper *FileOperatorHelper::instance()
{
    static FileOperatorHelper helper;
    return &helper;
}

void FileOperatorHelper::touchFolder(const FileView *view)
{
    auto windowId = WorkspaceHelper::instance()->windowId(view);
    fmDebug() << "Creating new folder in directory:" << view->rootUrl().toString() << "window ID:" << windowId;

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

    fmDebug() << "Creating new file - type:" << static_cast<int>(type) << "suffix:" << suffix
              << "in directory:" << url.toString() << "window ID:" << windowId;

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

    fmDebug() << "Creating new file from source:" << source.toString()
              << "in directory:" << url.toString() << "window ID:" << windowId;

    dpfSignalDispatcher->publish(GlobalEventType::kTouchFile,
                                 windowId,
                                 url,
                                 source,
                                 QString(),
                                 GlobalEventType::kTouchFile,
                                 callBack);
}

void FileOperatorHelper::openFiles(const FileView *view, const QList<QUrl> &urls)
{
    fmDebug() << "Opening files with current dir open mode - files count:" << urls.size();
    DirOpenMode openMode = view->currentDirOpenMode();
    openFilesByMode(view, urls, openMode);
}

void FileOperatorHelper::openFilesByMode(const FileView *view, const QList<QUrl> &urls, const DirOpenMode mode)
{
    auto windowId = WorkspaceHelper::instance()->windowId(view);
    fmDebug() << "Opening files by mode:" << static_cast<int>(mode) << "files count:" << urls.size() << "window ID:" << windowId;

    auto flag = !DConfigManager::instance()->value(kViewDConfName,
                                                   kOpenFolderWindowsInASeparateProcess, true)
                         .toBool();
    QList<QUrl> dirListOpenInNewWindow {};
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

                fmWarning() << "Failed to open file - file not found:" << url.toString();
                UniversalUtils::notifyMessage(QObject::tr("dde-file-manager"),
                                              tr("Failed to open %1, which may be moved or renamed").arg(fileName));
                continue;
            }

            if (fileInfoPtr->isAttributes(OptInfoType::kIsDir)) {
                QUrl dirUrl = url;
                if (fileInfoPtr->isAttributes(OptInfoType::kIsSymLink)) {
                    dirUrl = QUrl::fromLocalFile(QDir(fileInfoPtr->pathOf(PathInfoType::kSymLinkTarget)).absolutePath());
                    fmDebug() << "Directory is symlink - resolved target:" << dirUrl.toString();
                }

                if (mode == DirOpenMode::kOpenNewWindow || (flag && FileManagerWindowsManager::instance().containsCurrentUrl(dirUrl, view->window()))) {
                    fmDebug() << "Opening directory in new window:" << dirUrl.toString();
                    dirListOpenInNewWindow.append(dirUrl);
                } else {
                    fmDebug() << "Changing current URL to directory:" << dirUrl.toString();
                    WorkspaceEventCaller::sendChangeCurrentUrl(windowId, dirUrl);
                }
                continue;
            }
        }

        const QList<QUrl> &openUrls = { url };
        fmDebug() << "Opening file with default application:" << url.toString();
        dpfSignalDispatcher->publish(GlobalEventType::kOpenFiles,
                                     windowId,
                                     openUrls);
    }

    if (dirListOpenInNewWindow.isEmpty()) {
        fmDebug() << "No directories to open in new windows";
        return;
    }

    if (dirListOpenInNewWindow.count() > DFMGLOBAL_NAMESPACE::kOpenNewWindowMaxCount) {
        fmWarning() << "Too many directories to open in new windows - count:" << dirListOpenInNewWindow.count() << "max allowed:" << DFMGLOBAL_NAMESPACE::kOpenNewWindowMaxCount;
        return;
    }

    WorkspaceEventCaller::sendOpenWindow(dirListOpenInNewWindow, !flag);
}

void FileOperatorHelper::openFilesByApp(const FileView *view, const QList<QUrl> &urls, const QList<QString> &apps)
{
    auto windowId = WorkspaceHelper::instance()->windowId(view);
    fmDebug() << "Opening files with specific applications - files count:" << urls.size() << "apps count:" << apps.size() << "window ID:" << windowId;

    dpfSignalDispatcher->publish(GlobalEventType::kOpenFilesByApp,
                                 windowId,
                                 urls,
                                 apps);
}

void FileOperatorHelper::renameFile(const FileView *view, const QUrl &oldUrl, const QUrl &newUrl)
{
    auto windowId = WorkspaceHelper::instance()->windowId(view);
    fmDebug() << "Renaming file from:" << oldUrl.toString() << "to:" << newUrl.toString() << "window ID:" << windowId;

    dpfSignalDispatcher->publish(GlobalEventType::kRenameFile,
                                 windowId,
                                 oldUrl,
                                 newUrl,
                                 DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kNoHint);
}

void FileOperatorHelper::copyFiles(const FileView *view)
{
    QList<QUrl> selectedUrls = view->selectedTreeViewUrlList();
    fmDebug() << "Copy operation started - initial selected count:" << selectedUrls.size();

    // trans url to local
    QList<QUrl> urls {};
    bool ok = UniversalUtils::urlsTransformToLocal(selectedUrls, &urls);
    if (ok && !urls.isEmpty()) {
        fmDebug() << "URLs transformed to local - count:" << urls.size();
        selectedUrls = urls;
    }

    if (selectedUrls.size() == 1) {
        const FileInfoPointer &fileInfo = InfoFactory::create<FileInfo>(selectedUrls.first());
        if (!fileInfo || !fileInfo->isAttributes(OptInfoType::kIsReadable)) {
            fmWarning() << "Cannot copy file - not readable:" << selectedUrls.first().toString();
            return;
        }
    }

    if (selectedUrls.isEmpty()) {
        fmDebug() << "Copy operation aborted - no files selected";
        return;
    }

    fmInfo() << "Copy shortcut key to clipboard, selected urls: " << selectedUrls.first()
             << ", selected count: " << selectedUrls.size()
             << ", current dir: " << view->rootUrl();

    auto windowId = WorkspaceHelper::instance()->windowId(view);

    dpfSignalDispatcher->publish(GlobalEventType::kWriteUrlsToClipboard,
                                 windowId,
                                 ClipBoard::ClipboardAction::kCopyAction,
                                 selectedUrls);
}

void FileOperatorHelper::copyFilePath(const FileView *view)
{
    const QList<QUrl> &selectedUrls = view->selectedUrlList();
    dpfSignalDispatcher->publish(GlobalEventType::kCopyFilePath, selectedUrls);
}

void FileOperatorHelper::cutFiles(const FileView *view)
{
    const FileInfoPointer &fileInfo = InfoFactory::create<FileInfo>(view->rootUrl());
    if (!fileInfo || !fileInfo->isAttributes(OptInfoType::kIsWritable)) {
        fmWarning() << "Cannot cut files - root directory not writable:" << view->rootUrl().toString();
        return;
    }

    QList<QUrl> selectedUrls = view->selectedTreeViewUrlList();
    QList<QUrl> urls {};
    bool ok = UniversalUtils::urlsTransformToLocal(selectedUrls, &urls);
    if (ok && !urls.isEmpty()) {
        fmDebug() << "URLs transformed to local for cut operation - count:" << urls.size();
        selectedUrls = urls;
    }

    if (selectedUrls.isEmpty()) {
        fmDebug() << "Cut operation aborted - no files selected";
        return;
    }

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

    // Check if target directory is trash
    if (FileUtils::isTrashFile(view->rootUrl())) {
        fmDebug() << "Paste operation blocked - target is trash directory";
        return;
    }

    // Try traditional file paste
    if (pasteTraditionalFiles(view)) {
        return;
    }

    // Try clipboard image paste
    pasteClipboardImage(view);
}

bool FileOperatorHelper::pasteTraditionalFiles(const FileView *view)
{
    auto action = ClipBoard::instance()->clipboardAction();
    auto sourceUrls = ClipBoard::instance()->clipboardFileUrlList();
    auto windowId = WorkspaceHelper::instance()->windowId(view);

    if (action == ClipBoard::kUnknownAction || sourceUrls.isEmpty()) {
        return false;
    }

    if (ClipBoard::kCopyAction == action) {
        fmDebug() << "Executing copy action";
        dpfSignalDispatcher->publish(GlobalEventType::kCopy,
                                     windowId,
                                     sourceUrls,
                                     view->rootUrl(),
                                     AbstractJobHandler::JobFlag::kNoHint, nullptr);
    } else if (ClipBoard::kCutAction == action) {
        if (ClipBoard::supportCut()) {
            fmDebug() << "Executing cut action and clearing clipboard";
            dpfSignalDispatcher->publish(GlobalEventType::kCutFile,
                                         windowId,
                                         sourceUrls,
                                         view->rootUrl(),
                                         AbstractJobHandler::JobFlag::kNoHint, nullptr);
            ClipBoard::clearClipboard();
        } else {
            fmWarning() << "Cut operation not supported";
        }
    } else if (action == ClipBoard::kRemoteCopiedAction) {   // 远程协助
        fmInfo() << "Remote Assistance Copy: set Current Url to Clipboard";
        ClipBoard::setCurUrlToClipboardForRemote(view->rootUrl());
    } else if (ClipBoard::kRemoteAction == action) {
        fmDebug() << "Executing remote copy action";
        dpfSignalDispatcher->publish(GlobalEventType::kCopy,
                                     windowId,
                                     sourceUrls,
                                     view->rootUrl(),
                                     AbstractJobHandler::JobFlag::kCopyRemote,
                                     nullptr);
    } else {
        return false;
    }

    return true;
}

void FileOperatorHelper::pasteClipboardImage(const FileView *view)
{
    const QMimeData *mimeData = QApplication::clipboard()->mimeData();
    if (!mimeData || !mimeData->hasImage()) {
        fmDebug() << "No image data in clipboard to paste";
        return;
    }

    fmInfo() << "Detected clipboard image, creating file via kTouchFile event";

    // Prepare custom data
    QVariantMap clipboardData;
    clipboardData.insert("clipboardImage", true);

    // Define callback to select newly created file
    AbstractJobHandler::OperatorCallback callback = [](const AbstractJobHandler::CallbackArgus args) {
        if (args->value(AbstractJobHandler::CallbackKey::kSuccessed).toBool() != false) {
            auto targets = args->value(AbstractJobHandler::CallbackKey::kTargets)
                                   .value<QList<QUrl>>();
            if (!targets.isEmpty()) {
                fmDebug() << "Requesting selection for created image file:" << targets;
                WorkspaceHelper::instance()->laterRequestSelectFiles(targets);
            }
        }
    };

    auto windowId = WorkspaceHelper::instance()->windowId(view);

    // Publish kTouchFile event
    dpfSignalDispatcher->publish(
            GlobalEventType::kTouchFile,
            windowId,
            view->rootUrl(),
            CreateFileType::kCreateFileTypeDefault,
            QString("png"),
            QVariant::fromValue(clipboardData),
            callback);
}

void FileOperatorHelper::undoFiles(const FileView *view)
{
    fmInfo() << "Undo files in the directory: " << view->rootUrl();
    auto windowId = WorkspaceHelper::instance()->windowId(view);

    dpfSignalDispatcher->publish(GlobalEventType::kRevocation,
                                 windowId, undoCallBack);
}

void FileOperatorHelper::moveToTrash(const FileView *view, const QList<QUrl> &urls)
{
    if (urls.isEmpty()) {
        fmDebug() << "Move to trash aborted - no URLs provided";
        return;
    }

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
    if (selectedUrls.isEmpty()) {
        fmDebug() << "Delete files aborted - no files selected";
        return;
    }

    fmInfo() << "Delete files, selected urls: " << selectedUrls
             << ", current dir: " << view->rootUrl();

    auto windowId = WorkspaceHelper::instance()->windowId(view);
    dpfSignalDispatcher->publish(GlobalEventType::kDeleteFiles,
                                 windowId,
                                 selectedUrls,
                                 AbstractJobHandler::JobFlag::kNoHint, nullptr);
}

void FileOperatorHelper::openInTerminal(const FileView *view)
{
    auto windowId = WorkspaceHelper::instance()->windowId(view);
    QList<QUrl> urls = view->selectedUrlList();
    if (urls.isEmpty())
        urls.append(view->rootUrl());

    fmInfo() << "Opening terminal for URLs - count:" << urls.size() << "window ID:" << windowId;
    dpfSignalDispatcher->publish(GlobalEventType::kOpenInTerminal,
                                 windowId,
                                 urls);
}

void FileOperatorHelper::showFilesProperty(const FileView *view)
{
    QList<QUrl> urls = view->selectedUrlList();
    if (urls.isEmpty())
        urls.append(view->rootUrl());

    fmInfo() << "Showing file properties for URLs - count:" << urls.size();
    dpfSlotChannel->push("dfmplugin_propertydialog", "slot_PropertyDialog_Show", urls, QVariantHash());
}

void FileOperatorHelper::previewFiles(const FileView *view, const QList<QUrl> &selectUrls, const QList<QUrl> &currentDirUrls)
{
    quint64 winID = WorkspaceHelper::instance()->windowId(view);
    fmInfo() << "Starting file preview - selected files:" << selectUrls.size()
             << "current dir files:" << currentDirUrls.size() << "window ID:" << winID;

    dpfSlotChannel->push("dfmplugin_fileoperations", "slot_Operation_FilesPreview", winID, selectUrls, currentDirUrls);
}

void FileOperatorHelper::dropFiles(const FileView *view, const Qt::DropAction &action, const QUrl &targetUrl, const QList<QUrl> &urls)
{
    auto windowId = WorkspaceHelper::instance()->windowId(view);
    fmInfo() << "Drop files operation - action:" << action << "target:" << targetUrl.toString()
             << "files count:" << urls.size() << "window ID:" << windowId;

    if (action == Qt::MoveAction) {
        fmDebug() << "Executing move action via cut";
        dpfSignalDispatcher->publish(GlobalEventType::kCutFile,
                                     windowId,
                                     urls,
                                     targetUrl,
                                     AbstractJobHandler::JobFlag::kNoHint, nullptr);
    } else {
        // default is copy file
        fmDebug() << "Executing copy action (default)";
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
    fmDebug() << "FileOperatorHelper initialized";
    callBack = std::bind(&FileOperatorHelper::callBackFunction, this, std::placeholders::_1);
    undoCallBack = std::bind(&FileOperatorHelper::undoCallBackFunction, this, std::placeholders::_1);
}

void FileOperatorHelper::callBackFunction(const AbstractJobHandler::CallbackArgus args)
{
    const QVariant &customValue = args->value(AbstractJobHandler::CallbackKey::kCustom);
    GlobalEventType type = static_cast<GlobalEventType>(customValue.toInt());

    fmDebug() << "Callback function triggered for event type:" << static_cast<int>(type);

    switch (type) {
    case kMkdir: {
        quint64 windowID = args->value(AbstractJobHandler::CallbackKey::kWindowId).toULongLong();
        QList<QUrl> sourceUrlList = args->value(AbstractJobHandler::CallbackKey::kSourceUrls).value<QList<QUrl>>();
        if (sourceUrlList.isEmpty()) {
            fmWarning() << "Mkdir callback - empty source URL list";
            break;
        }

        QList<QUrl> targetUrlList = args->value(AbstractJobHandler::CallbackKey::kTargets).value<QList<QUrl>>();
        if (targetUrlList.isEmpty()) {
            fmWarning() << "Mkdir callback - empty target URL list";
            break;
        }

        QUrl rootUrl = sourceUrlList.first();
        QUrl newFolder = targetUrlList.first();
        fmInfo() << "Mkdir completed - root:" << rootUrl.toString() << "new folder:" << newFolder.toString() << "window:" << windowID;
        WorkspaceHelper::kSelectionAndRenameFile[windowID] = qMakePair(rootUrl, newFolder);
        break;
    }
    case kTouchFile: {
        quint64 windowID = args->value(AbstractJobHandler::CallbackKey::kWindowId).toULongLong();
        QList<QUrl> sourceUrlList = args->value(AbstractJobHandler::CallbackKey::kSourceUrls).value<QList<QUrl>>();
        if (sourceUrlList.isEmpty()) {
            fmWarning() << "TouchFile callback - empty source URL list";
            break;
        }

        QList<QUrl> targetUrlList = args->value(AbstractJobHandler::CallbackKey::kTargets).value<QList<QUrl>>();
        if (targetUrlList.isEmpty()) {
            fmWarning() << "TouchFile callback - empty target URL list";
            break;
        }

        QUrl rootUrl = sourceUrlList.first();
        QUrl newFile = targetUrlList.first();
        fmInfo() << "TouchFile completed - root:" << rootUrl.toString() << "new file:" << newFile.toString() << "window:" << windowID;
        WorkspaceHelper::kSelectionAndRenameFile[windowID] = qMakePair(rootUrl, newFile);
        break;
    }
    default:
        break;
    }
}

void FileOperatorHelper::undoCallBackFunction(QSharedPointer<AbstractJobHandler> handler)
{
    fmDebug() << "Setting up undo callback handlers";

    connect(handler.data(), &AbstractJobHandler::finishedNotify, this, [=](const JobInfoPointer jobInfo) {
        AbstractJobHandler::JobType type = static_cast<AbstractJobHandler::JobType>(jobInfo->value(AbstractJobHandler::kJobtypeKey).toInt());
        fmDebug() << "Undo operation finished notification - job type:" << static_cast<int>(type);

        if (type == AbstractJobHandler::JobType::kCutType) {
            QList<QUrl> targetUrls(jobInfo->value(AbstractJobHandler::kCompleteTargetFilesKey).value<QList<QUrl>>());
            fmInfo() << "Cut operation completed in undo - setting undo files, count:" << targetUrls.size();
            WorkspaceHelper::instance()->setUndoFiles(targetUrls);
        }
    });

    connect(handler.data(), &AbstractJobHandler::workerFinish, this, [=]() {
        fmDebug() << "Undo operation worker finished - clearing undo files";
        WorkspaceHelper::instance()->setUndoFiles({});
    });
}
