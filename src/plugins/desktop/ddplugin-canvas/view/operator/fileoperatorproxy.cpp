// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileoperatorproxy.h"
#include "fileoperatorproxy_p.h"
#include "canvasmanager.h"
#include "view/canvasview.h"
#include "model/canvasproxymodel.h"
#include "grid/canvasgrid.h"
#include "model/canvasselectionmodel.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/utils/fileutils.h>
#include <QClipboard>
#include <QMimeData>
#include <QImage>
#include <QDateTime>
#include <QApplication>

#include <dfm-framework/dpf.h>

#include <functional>

DFMGLOBAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace ddplugin_canvas;

#define KEY_SCREENNUMBER "screenNumber"
#define KEY_POINT "point"

class FileBusinessGlobal : public FileOperatorProxy
{
};
Q_GLOBAL_STATIC(FileBusinessGlobal, fileBusinessGlobal)

FileOperatorProxyPrivate::FileOperatorProxyPrivate(FileOperatorProxy *q_ptr)
    : QObject(q_ptr), q(q_ptr)
{
}

void FileOperatorProxyPrivate::callBackTouchFile(const QUrl &target, const QVariantMap &customData)
{
    fmInfo() << "Touch file callback initiated for:" << target;

    q->clearTouchFileData();

    QString path = target.toString();
    int screenNum = customData.value(KEY_SCREENNUMBER).toInt();
    QPoint pos = customData.value(KEY_POINT).value<QPoint>();

    fmDebug() << "Touch file parameters - screen:" << screenNum << "position:" << pos << "path:" << path;

    // befor call back,recive file created signal
    QPair<int, QPoint> oriPoint;
    if (GridIns->point(path, oriPoint)) {
        fmInfo() << "note:file existed!must check code!" << path << oriPoint << pos;
        if (CanvasGrid::Mode::Align == GridIns->mode()) {
            fmDebug() << "Canvas in align mode - skipping position adjustment";
            return;
        }

        if (oriPoint.first == screenNum && oriPoint.second == pos) {
            fmDebug() << "File already at target position - no move needed";
            return;
        }

        // move it
        bool moved = GridIns->move(screenNum, pos, path, { path });
        fmDebug() << "item:" << path << " move:" << moved << " ori:" << oriPoint.first << oriPoint.second << "   target:" << screenNum << pos;
    } else if (Q_UNLIKELY(GridIns->overloadItems(-1).contains(path))) {
        fmDebug() << "item:" << path << " is overload";
    } else {
        // record the location and move the file after the real file is created
        fmDebug() << "Recording touch file data for later positioning:" << path << "at" << screenNum << pos;
        touchFileData = qMakePair(path, qMakePair(screenNum, pos));
    }

    emit q->fileTouchedCallback();
    fmDebug() << "Touch file callback completed";
}

void FileOperatorProxyPrivate::callBackPasteFiles(const JobInfoPointer info)
{
    fmInfo() << "Paste files callback initiated";

    if (info->keys().contains(AbstractJobHandler::NotifyInfoKey::kCompleteTargetFilesKey)) {
        QList<QUrl> files = info->value(AbstractJobHandler::NotifyInfoKey::kCompleteTargetFilesKey).value<QList<QUrl>>();

        fmInfo() << "Paste operation completed with" << files.size() << "files";

        q->clearPasteFileData();

        // clear all selection.
        auto sel = CanvasIns->selectionModel();
        if (sel) {
            fmDebug() << "Clearing current selection before selecting pasted files";
            sel->clear();
        }

        auto model = CanvasIns->model();
        if (model && sel) {
            int existingCount = 0;
            int pendingCount = 0;

            for (const QUrl &url : files) {
                auto idx = model->index(url);
                if (idx.isValid()) {
                    // selecting this file that is existed in model.
                    sel->select(idx, QItemSelectionModel::Select);
                    existingCount++;
                } else {
                    // record the file is not existed and selecting it when it is inserted.
                    pasteFileData.insert(url);
                    pendingCount++;
                }
            }

            fmDebug() << "File selection status - existing:" << existingCount << "pending:" << pendingCount;
        } else {
            fmWarning() << "there were no model and selection model.";
            pasteFileData = QSet<QUrl>(files.begin(), files.end());
        }
        emit q->filePastedCallback();
        fmDebug() << "Paste files callback completed";
    } else {
        fmWarning() << "Paste callback received without complete target files key";
    }
}

void FileOperatorProxyPrivate::callBackRenameFiles(const QList<QUrl> &sources, const QList<QUrl> &targets)
{
    fmInfo() << "Rename files callback initiated - processing" << sources.size() << "files";

    q->clearRenameFileData();

    // clear selected and current
    auto view = CanvasIns->views().first();
    if (Q_UNLIKELY(nullptr == view)) {
        fmWarning() << "No canvas view available for rename operation";
        return;
    }
    view->selectionModel()->clear();

    Q_ASSERT(sources.count() == targets.count());

    for (int i = 0; i < targets.count(); ++i) {
        renameFileData.insert(sources.at(i), targets.at(i));
    }
    emit q->fileRenamedCallback();
    fmInfo() << "Rename files callback completed successfully";
}

void FileOperatorProxyPrivate::filterDesktopFile(QList<QUrl> &urls)
{
    // computer and trash desktop files cannot be copied or cut.
    // filter the URL of these files copied and cut through shortcut keys here
    urls.removeAll(DesktopAppUrl::computerDesktopFileUrl());
    urls.removeAll(DesktopAppUrl::trashDesktopFileUrl());
    urls.removeAll(DesktopAppUrl::homeDesktopFileUrl());
}

FileOperatorProxy::FileOperatorProxy(QObject *parent)
    : QObject(parent), d(new FileOperatorProxyPrivate(this))
{
    d->callBack = std::bind(&FileOperatorProxy::callBackFunction, this, std::placeholders::_1);
}

FileOperatorProxy *FileOperatorProxy::instance()
{
    return fileBusinessGlobal;
}

void FileOperatorProxy::touchFile(const CanvasView *view, const QPoint pos, const DFMBASE_NAMESPACE::Global::CreateFileType type, QString suffix)
{
    QVariantMap data;
    data.insert(KEY_SCREENNUMBER, view->screenNum());
    data.insert(KEY_POINT, pos);
    QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant> funcData(FileOperatorProxyPrivate::kCallBackTouchFile, data);
    QVariant custom = QVariant::fromValue(funcData);

    dpfSignalDispatcher->publish(GlobalEventType::kTouchFile, view->winId(), view->model()->rootUrl(), type, suffix, custom, d->callBack);
}

void FileOperatorProxy::touchFile(const CanvasView *view, const QPoint pos, const QUrl &source)
{
    QVariantMap data;
    data.insert(KEY_SCREENNUMBER, view->screenNum());
    data.insert(KEY_POINT, pos);
    QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant> funcData(FileOperatorProxyPrivate::kCallBackTouchFile, data);
    QVariant custom = QVariant::fromValue(funcData);

    dpfSignalDispatcher->publish(GlobalEventType::kTouchFile, view->winId(), view->model()->rootUrl(), source, QString(), custom, d->callBack);
}

void FileOperatorProxy::touchFolder(const CanvasView *view, const QPoint pos)
{
    QVariantMap data;
    data.insert(KEY_SCREENNUMBER, view->screenNum());
    data.insert(KEY_POINT, pos);
    QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant> funcData(FileOperatorProxyPrivate::kCallBackTouchFolder, data);
    QVariant custom = QVariant::fromValue(funcData);

    dpfSignalDispatcher->publish(GlobalEventType::kMkdir, view->winId(), view->model()->rootUrl(), custom, d->callBack);
}

void FileOperatorProxy::copyFiles(const CanvasView *view)
{
    auto urls = view->selectionModel()->selectedUrls();
    fmInfo() << "Copy files request - initial selection:" << urls.size() << "files on screen:" << view->screenNum();

    d->filterDesktopFile(urls);
    if (urls.isEmpty()) {
        fmDebug() << "No files to copy after filtering";
        return;
    }

    fmInfo() << "Copying" << urls.size() << "files to clipboard";
    dpfSignalDispatcher->publish(GlobalEventType::kWriteUrlsToClipboard, view->winId(), ClipBoard::ClipboardAction::kCopyAction, urls);
}

void FileOperatorProxy::copyFilePath(const CanvasView *view)
{
    auto urls = view->selectionModel()->selectedUrls();
    dpfSignalDispatcher->publish(GlobalEventType::kCopyFilePath, urls);
}

void FileOperatorProxy::cutFiles(const CanvasView *view)
{
    auto urls = view->selectionModel()->selectedUrls();
    fmInfo() << "Cut files request - initial selection:" << urls.size() << "files on screen:" << view->screenNum();

    d->filterDesktopFile(urls);
    if (urls.isEmpty()) {
        fmDebug() << "No files to cut after filtering";
        return;
    }

    fmInfo() << "Cutting" << urls.size() << "files to clipboard";
    dpfSignalDispatcher->publish(GlobalEventType::kWriteUrlsToClipboard, view->winId(), ClipBoard::ClipboardAction::kCutAction, urls);
}

void FileOperatorProxy::pasteFiles(const CanvasView *view, const QPoint pos)
{
    // feature:paste at pos
    Q_UNUSED(pos)

    auto urls = ClipBoard::instance()->clipboardFileUrlList();
    ClipBoard::ClipboardAction action = ClipBoard::instance()->clipboardAction();

    fmInfo() << "Paste files request - clipboard action:" << static_cast<int>(action) << "URLs:" << urls.size() << "on screen:" << view->screenNum();

    // 深信服和云桌面的远程拷贝获取的clipboardFileUrlList都是空
    if (ClipBoard::kRemoteCopiedAction == action) {   // 远程协助
        fmInfo() << "Remote Assistance Copy: set Current Url to Clipboard";
        ClipBoard::setCurUrlToClipboardForRemote(view->model()->rootUrl());
        return;
    }

    if (ClipBoard::kRemoteAction == action) {
        fmDebug() << "Remote clipboard action detected - initiating remote copy with" << urls.size() << "URLs";
        dpfSignalDispatcher->publish(GlobalEventType::kCopy, view->winId(), urls, view->model()->rootUrl(),
                                     AbstractJobHandler::JobFlag::kCopyRemote, nullptr);
        return;
    }

    if (urls.isEmpty()) {
        // Check for clipboard image
        const QMimeData *mimeData = QApplication::clipboard()->mimeData();
        if (mimeData && mimeData->hasImage()) {
            fmInfo() << "Detected clipboard image, creating file via kTouchFile event";

            // Prepare custom data to identify this is a clipboard image
            QVariantMap clipboardData;
            clipboardData.insert("clipboardImage", true);

            // Define callback to select newly created file
            AbstractJobHandler::OperatorCallback callback = [this](const AbstractJobHandler::CallbackArgus args) {
                if (args->value(AbstractJobHandler::CallbackKey::kSuccessed).toBool() != false) {
                    auto targets = args->value(AbstractJobHandler::CallbackKey::kTargets)
                                      .value<QList<QUrl>>();
                    if (!targets.isEmpty()) {
                        fmDebug() << "Canvas: Requesting selection for created image file:" << targets;
                        // Add to pasteFileData to trigger selection via existing mechanism
                        d->pasteFileData.insert(targets.first());
                        emit filePastedCallback();
                    }
                }
            };

            // Publish kTouchFile event with png suffix
            dpfSignalDispatcher->publish(
                GlobalEventType::kTouchFile,
                view->winId(),
                view->model()->rootUrl(),
                CreateFileType::kCreateFileTypeDefault,
                QString("png"),  // Image suffix
                QVariant::fromValue(clipboardData),
                callback);

            return;
        }

        fmDebug() << "No URLs or image data in clipboard to paste";
        return;
    }

    QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant> funcData(FileOperatorProxyPrivate::kCallBackPasteFiles, QVariant());
    QVariant custom = QVariant::fromValue(funcData);

    if (ClipBoard::kCopyAction == action) {
        fmDebug() << "Executing copy paste operation for" << urls.size() << "files";
        dpfSignalDispatcher->publish(GlobalEventType::kCopy, view->winId(), urls, view->model()->rootUrl(),
                                     AbstractJobHandler::JobFlag::kNoHint, nullptr, custom, d->callBack);
    } else if (ClipBoard::kCutAction == action) {
        fmDebug() << "Executing cut paste operation for" << urls.size() << "files";
        dpfSignalDispatcher->publish(GlobalEventType::kCutFile, view->winId(), urls, view->model()->rootUrl(),
                                     AbstractJobHandler::JobFlag::kNoHint, nullptr, custom, d->callBack);
        //! todo bug#63441 如果是剪切操作，则禁止跨用户的粘贴操作, 讨论是否应该由下层统一处理?

        // clear clipboard after cutting files from clipboard
        fmDebug() << "Clearing clipboard after cut operation";
        ClipBoard::instance()->clearClipboard();
    } else {
        fmWarning() << "clipboard action:" << action << "    urls:" << urls;
    }
}

void FileOperatorProxy::openFiles(const CanvasView *view)
{
    auto urls = view->selectionModel()->selectedUrls();
    if (!urls.isEmpty())
        openFiles(view, urls);
    else
        fmDebug() << "No files selected to open";
}

void FileOperatorProxy::openFiles(const CanvasView *view, const QList<QUrl> &urls)
{
    fmInfo() << "Opening" << urls.size() << "files from screen:" << view->screenNum();
    dpfSignalDispatcher->publish(GlobalEventType::kOpenFiles, view->winId(), urls);
}

void FileOperatorProxy::renameFile(int wid, const QUrl &oldUrl, const QUrl &newUrl)
{
    fmInfo() << "Single file rename request - from:" << oldUrl << "to:" << newUrl << "window ID:" << wid;
    dpfSignalDispatcher->publish(GlobalEventType::kRenameFile, wid, oldUrl, newUrl, DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kNoHint);
}

void FileOperatorProxy::renameFiles(const CanvasView *view, const QList<QUrl> &urls, const QPair<QString, QString> &pair, const bool replace)
{
    fmInfo() << "Batch rename files request -" << urls.size() << "files, pattern:" << pair.first << "->" << pair.second << "replace:" << replace;

    QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant> funcData(FileOperatorProxyPrivate::kCallBackRenameFiles, QVariant());
    QVariant custom = QVariant::fromValue(funcData);

    dpfSignalDispatcher->publish(GlobalEventType::kRenameFiles, view->winId(), urls, pair, replace, custom, d->callBack);
}

void FileOperatorProxy::renameFiles(const CanvasView *view, const QList<QUrl> &urls, const QPair<QString, AbstractJobHandler::FileNameAddFlag> pair)
{
    fmInfo() << "Batch rename files with flag request -" << urls.size() << "files, pattern:" << pair.first << "flag:" << static_cast<int>(pair.second);

    QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant> funcData(FileOperatorProxyPrivate::kCallBackRenameFiles, QVariant());
    QVariant custom = QVariant::fromValue(funcData);

    dpfSignalDispatcher->publish(GlobalEventType::kRenameFiles, view->winId(), urls, pair, custom, d->callBack);
}

void FileOperatorProxy::openFilesByApp(const CanvasView *view)
{
    Q_UNUSED(view)
    // todo(wangcl):dependent right-click menu
}

void FileOperatorProxy::moveToTrash(const CanvasView *view)
{
    dpfSignalDispatcher->publish(GlobalEventType::kMoveToTrash, view->winId(), view->selectionModel()->selectedUrls(), AbstractJobHandler::JobFlag::kNoHint, nullptr);
}

void FileOperatorProxy::deleteFiles(const CanvasView *view)
{
    dpfSignalDispatcher->publish(GlobalEventType::kDeleteFiles, view->winId(), view->selectionModel()->selectedUrls(), AbstractJobHandler::JobFlag::kNoHint, nullptr);
}

void FileOperatorProxy::showFilesProperty(const CanvasView *view)
{
    dpfSlotChannel->push("dfmplugin_propertydialog", "slot_PropertyDialog_Show", view->selectionModel()->selectedUrls(), QVariantHash());
}

void FileOperatorProxy::sendFilesToBluetooth(const CanvasView *view)
{
    QList<QUrl> urls = view->selectionModel()->selectedUrls();
    fmInfo() << "Send to Bluetooth request -" << urls.size() << "files from screen:" << view->screenNum();

    if (!urls.isEmpty()) {
        QStringList paths;
        for (const auto &u : urls)
            paths << u.path();
        fmDebug() << "Converting" << urls.size() << "URLs to paths for Bluetooth transfer";
        dpfSlotChannel->push("dfmplugin_utils", "slot_Bluetooth_SendFiles", paths, "");
    } else {
        fmDebug() << "No files selected for Bluetooth transfer";
    }
}

void FileOperatorProxy::undoFiles(const CanvasView *view)
{
    dpfSignalDispatcher->publish(GlobalEventType::kRevocation,
                                 view->winId(), nullptr);
}

void FileOperatorProxy::redoFiles(const CanvasView *view)
{
    dpfSignalDispatcher->publish(GlobalEventType::kRedo,
                                 view->winId(), nullptr);
}

void FileOperatorProxy::dropFiles(const Qt::DropAction &action, const QUrl &targetUrl, const QList<QUrl> &urls)
{
    QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant> funcData(FileOperatorProxyPrivate::kCallBackPasteFiles, QVariant());
    QVariant custom = QVariant::fromValue(funcData);

    // drop files from other app will auto append,independent of the view
    auto view = CanvasIns->views().first();
    if (Q_UNLIKELY(nullptr == view)) {
        fmWarning() << "No canvas view available for drop operation";
        return;
    }

    if (action == Qt::MoveAction) {
        fmInfo() << "Executing drop move operation for" << urls.size() << "files";
        dpfSignalDispatcher->publish(GlobalEventType::kCutFile, view->winId(), urls, targetUrl, AbstractJobHandler::JobFlag::kNoHint, nullptr, custom, d->callBack);
    } else {
        // default is copy file
        fmInfo() << "Executing drop copy operation for" << urls.size() << "files";
        dpfSignalDispatcher->publish(GlobalEventType::kCopy, view->winId(), urls, targetUrl, AbstractJobHandler::JobFlag::kNoHint, nullptr, custom, d->callBack);
    }
}

void FileOperatorProxy::dropToTrash(const QList<QUrl> &urls)
{
    auto view = CanvasIns->views().first();
    dpfSignalDispatcher->publish(GlobalEventType::kMoveToTrash, view->winId(), urls, AbstractJobHandler::JobFlag::kNoHint, nullptr);
}

void FileOperatorProxy::dropToApp(const QList<QUrl> &urls, const QString &app)
{
    fmInfo() << "Drop to app operation -" << urls.size() << "files to app:" << app;

    auto view = CanvasIns->views().first();
    QList<QString> apps { app };
    dpfSignalDispatcher->publish(GlobalEventType::kOpenFilesByApp, view->winId(), urls, apps);
}

QPair<QString, QPair<int, QPoint>> FileOperatorProxy::touchFileData() const
{
    return d->touchFileData;
}

void FileOperatorProxy::clearTouchFileData()
{
    d->touchFileData = qMakePair(QString(), qMakePair(-1, QPoint(-1, -1)));
}

QHash<QUrl, QUrl> FileOperatorProxy::renameFileData() const
{
    return d->renameFileData;
}

void FileOperatorProxy::removeRenameFileData(const QUrl &oldUrl)
{
    d->renameFileData.remove(oldUrl);
}

void FileOperatorProxy::clearRenameFileData()
{
    d->renameFileData.clear();
}

QSet<QUrl> FileOperatorProxy::pasteFileData() const
{
    return d->pasteFileData;
}

void FileOperatorProxy::removePasteFileData(const QUrl &oldUrl)
{
    d->pasteFileData.remove(oldUrl);
}

void FileOperatorProxy::clearPasteFileData()
{
    d->pasteFileData.clear();
}

void FileOperatorProxy::callBackFunction(const AbstractJobHandler::CallbackArgus args)
{
    const QVariant &customValue = args->value(AbstractJobHandler::CallbackKey::kCustom);
    const QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant> &custom = customValue.value<QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant>>();
    const FileOperatorProxyPrivate::CallBackFunc funcKey = custom.first;

    switch (funcKey) {
    case FileOperatorProxyPrivate::CallBackFunc::kCallBackTouchFile:
    case FileOperatorProxyPrivate::CallBackFunc::kCallBackTouchFolder: {
        // Folder also belong to files
        // touch file is sync operation

        auto targets = args->value(AbstractJobHandler::CallbackKey::kTargets).value<QList<QUrl>>();
        if (Q_UNLIKELY(targets.count() != 1)) {
            fmWarning() << "unknow error.touch file successed,target urls is:" << targets;
        }

        d->callBackTouchFile(targets.first(), custom.second.toMap());
    } break;
    case FileOperatorProxyPrivate::CallBackFunc::kCallBackPasteFiles: {
        // paste files is async operation
        JobHandlePointer jobHandle = args->value(AbstractJobHandler::CallbackKey::kJobHandle).value<JobHandlePointer>();
        if (jobHandle) {
            if (jobHandle->currentState() != AbstractJobHandler::JobState::kStopState) {
                connect(jobHandle.get(), &AbstractJobHandler::finishedNotify, d.get(), &FileOperatorProxyPrivate::callBackPasteFiles);
            } else {
                JobInfoPointer infoPointer = jobHandle->getTaskInfoByNotifyType(AbstractJobHandler::NotifyType::kNotifyFinishedKey);
                d->callBackPasteFiles(infoPointer);
            }
        }
    } break;
    case FileOperatorProxyPrivate::CallBackFunc::kCallBackRenameFiles: {
        auto sources = args->value(AbstractJobHandler::CallbackKey::kSourceUrls).value<QList<QUrl>>();
        auto targets = args->value(AbstractJobHandler::CallbackKey::kTargets).value<QList<QUrl>>();
        d->callBackRenameFiles(sources, targets);
    } break;
    default:
        break;
    }
}
