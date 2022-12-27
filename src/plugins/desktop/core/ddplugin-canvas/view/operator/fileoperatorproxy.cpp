/*
 * Copyright (C) 2022 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#include "fileoperatorproxy.h"
#include "fileoperatorproxy_p.h"
#include "canvasmanager.h"
#include "view/canvasview.h"
#include "model/canvasproxymodel.h"
#include "grid/canvasgrid.h"
#include "model/canvasselectionmodel.h"

#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/utils/clipboard.h"
#include "dfm-base/utils/fileutils.h"

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
    q->clearTouchFileData();

    QString path = target.toString();
    int screenNum = customData.value(KEY_SCREENNUMBER).toInt();
    QPoint pos = customData.value(KEY_POINT).value<QPoint>();

    // befor call back,recive file created signal
    QPair<int, QPoint> oriPoint;
    if (Q_UNLIKELY(GridIns->point(path, oriPoint))) {
        qInfo() << "note:file existed!must check code!" << path << oriPoint << pos;
        if (CanvasGrid::Mode::Align == GridIns->mode())
            return;

        if (oriPoint.first == screenNum && oriPoint.second == pos)
            return;

        // move it
        bool moved = GridIns->move(screenNum, pos, path, { path });
        qDebug() << "item:" << path << " move:" << moved << " ori:" << oriPoint.first << oriPoint.second << "   target:" << screenNum << pos;
    } else if (Q_UNLIKELY(GridIns->overloadItems(-1).contains(path))) {
        qDebug() << "item:" << path << " is overload";
    } else {
        // record the location and move the file after the real file is created
        touchFileData = qMakePair(path, qMakePair(screenNum, pos));
    }
}

void FileOperatorProxyPrivate::callBackPasteFiles(const JobInfoPointer info)
{
    if (info->keys().contains(AbstractJobHandler::NotifyInfoKey::kCompleteTargetFilesKey)) {
        QList<QUrl> files = info->value(AbstractJobHandler::NotifyInfoKey::kCompleteTargetFilesKey).value<QList<QUrl>>();
        delaySelectUrls(files);
    }
}

void FileOperatorProxyPrivate::callBackRenameFiles(const QList<QUrl> &sources, const QList<QUrl> &targets)
{
    q->clearRenameFileData();

    // clear selected and current
    auto view = CanvasIns->views().first();
    if (Q_UNLIKELY(nullptr == view))
        return;
    view->selectionModel()->clearSelection();
    view->selectionModel()->clearCurrentIndex();

    Q_ASSERT(sources.count() == targets.count());

    for (int i = 0; i < targets.count(); ++i) {
        renameFileData.insert(sources.at(i), targets.at(i));
    }
}

void FileOperatorProxyPrivate::delaySelectUrls(const QList<QUrl> &urls, int ms)
{
    if (nullptr != selectTimer.get())
        selectTimer->stop();

    if (ms < 1) {
        doSelectUrls(urls);
    } else {
        selectTimer.reset(new QTimer);
        selectTimer->setSingleShot(true);
        connect(selectTimer.get(), &QTimer::timeout, this, [&, urls]() {
            this->doSelectUrls(urls);
        });
        selectTimer->start(ms);
    }
}

void FileOperatorProxyPrivate::doSelectUrls(const QList<QUrl> &urls)
{
    auto view = CanvasIns->views().first();
    if (Q_UNLIKELY(nullptr == view))
        return;

    if (urls.isEmpty()) {
        // clear
        view->selectionModel()->clearSelection();
        view->selectionModel()->clearCurrentIndex();
        return;
    }

    QModelIndexList indexs;
    for (auto url : urls) {
        QModelIndex index = view->model()->index(url);
        indexs << index;
    }

    if (indexs.isEmpty()) {
        qWarning() << "select url failed,file does not exist?files:" << urls;
        return;
    }

    /*!
     * File pasting is an asynchronous operation.
     * The actual order of creating files (index) is inconsistent with the last file in the callback function.
     * Topleft and bottomright cannot be directly selected from the last file.
     * Only can empty the original selection and then select new files one by one
    */

    view->selectionModel()->clearSelection();
    for (auto index : indexs) {
        view->selectionModel()->select(index, QItemSelectionModel::Select);
    }
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
    d->filterDesktopFile(urls);
    if (urls.isEmpty())
        return;

    dpfSignalDispatcher->publish(GlobalEventType::kWriteUrlsToClipboard, view->winId(), ClipBoard::ClipboardAction::kCopyAction, urls);
}

void FileOperatorProxy::cutFiles(const CanvasView *view)
{
    auto urls = view->selectionModel()->selectedUrls();
    d->filterDesktopFile(urls);
    if (urls.isEmpty())
        return;

    dpfSignalDispatcher->publish(GlobalEventType::kWriteUrlsToClipboard, view->winId(), ClipBoard::ClipboardAction::kCutAction, urls);
}

void FileOperatorProxy::pasteFiles(const CanvasView *view, const QPoint pos)
{
    // feature:paste at pos
    Q_UNUSED(pos)

    auto urls = ClipBoard::instance()->clipboardFileUrlList();
    if (urls.isEmpty())
        return;

    QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant> funcData(FileOperatorProxyPrivate::kCallBackPasteFiles, QVariant());
    QVariant custom = QVariant::fromValue(funcData);

    ClipBoard::ClipboardAction action = ClipBoard::instance()->clipboardAction();
    if (ClipBoard::kCopyAction == action) {
        dpfSignalDispatcher->publish(GlobalEventType::kCopy, view->winId(), urls, view->model()->rootUrl(), AbstractJobHandler::JobFlag::kNoHint, nullptr, custom, d->callBack);
    } else if (ClipBoard::kCutAction == action) {
        dpfSignalDispatcher->publish(GlobalEventType::kCutFile, view->winId(), urls, view->model()->rootUrl(), AbstractJobHandler::JobFlag::kNoHint, nullptr, custom, d->callBack);
        //! todo bug#63441 如果是剪切操作，则禁止跨用户的粘贴操作, 讨论是否应该由下层统一处理?

        // clear clipboard after cutting files from clipboard
        ClipBoard::instance()->clearClipboard();
    } else if (ClipBoard::kRemoteCopiedAction == action) {   // 远程协助
        qInfo() << "Remote Assistance Copy: set Current Url to Clipboard";
        ClipBoard::setCurUrlToClipboardForRemote(view->model()->rootUrl());
    } else {
        qWarning() << "clipboard action:" << action << "    urls:" << urls;
    }
}

void FileOperatorProxy::openFiles(const CanvasView *view)
{
    auto urls = view->selectionModel()->selectedUrls();
    if (!urls.isEmpty())
        openFiles(view, urls);
}

void FileOperatorProxy::openFiles(const CanvasView *view, const QList<QUrl> &urls)
{
    dpfSignalDispatcher->publish(GlobalEventType::kOpenFiles, view->winId(), urls);
}

void FileOperatorProxy::renameFile(int wid, const QUrl &oldUrl, const QUrl &newUrl)
{
    dpfSignalDispatcher->publish(GlobalEventType::kRenameFile, wid, oldUrl, newUrl, DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kNoHint);
}

void FileOperatorProxy::renameFiles(const CanvasView *view, const QList<QUrl> &urls, const QPair<QString, QString> &pair, const bool replace)
{
    QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant> funcData(FileOperatorProxyPrivate::kCallBackRenameFiles, QVariant());
    QVariant custom = QVariant::fromValue(funcData);

    dpfSignalDispatcher->publish(GlobalEventType::kRenameFiles, view->winId(), urls, pair, replace, custom, d->callBack);
}

void FileOperatorProxy::renameFiles(const CanvasView *view, const QList<QUrl> &urls, const QPair<QString, AbstractJobHandler::FileNameAddFlag> pair)
{
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
    if (!urls.isEmpty()) {
        QStringList paths;
        for (const auto &u : urls)
            paths << u.path();
        dpfSlotChannel->push("dfmplugin_utils", "slot_Bluetooth_SendFiles", paths, "");
    }
}

void FileOperatorProxy::undoFiles(const CanvasView *view)
{
    dpfSignalDispatcher->publish(GlobalEventType::kRevocation,
                                 view->winId(), nullptr);
}

void FileOperatorProxy::dropFiles(const Qt::DropAction &action, const QUrl &targetUrl, const QList<QUrl> &urls)
{
    QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant> funcData(FileOperatorProxyPrivate::kCallBackPasteFiles, QVariant());
    QVariant custom = QVariant::fromValue(funcData);

    // drop files from other app will auto append,independent of the view
    auto view = CanvasIns->views().first();
    if (Q_UNLIKELY(nullptr == view))
        return;

    if (action == Qt::MoveAction) {
        dpfSignalDispatcher->publish(GlobalEventType::kCutFile, view->winId(), urls, targetUrl, AbstractJobHandler::JobFlag::kNoHint, nullptr, custom, d->callBack);
    } else {
        // default is copy file
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

void FileOperatorProxy::callBackFunction(const CallbackArgus args)
{
    const QVariant &customValue = args->value(CallbackKey::kCustom);
    const QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant> &custom = customValue.value<QPair<FileOperatorProxyPrivate::CallBackFunc, QVariant>>();
    const FileOperatorProxyPrivate::CallBackFunc funcKey = custom.first;

    switch (funcKey) {
    case FileOperatorProxyPrivate::CallBackFunc::kCallBackTouchFile:
    case FileOperatorProxyPrivate::CallBackFunc::kCallBackTouchFolder: {
        // Folder also belong to files
        // touch file is sync operation

        auto targets = args->value(CallbackKey::kTargets).value<QList<QUrl>>();
        if (Q_UNLIKELY(targets.count() != 1)) {
            qWarning() << "unknow error.touch file successed,target urls is:" << targets;
        }

        d->callBackTouchFile(targets.first(), custom.second.toMap());
    } break;
    case FileOperatorProxyPrivate::CallBackFunc::kCallBackPasteFiles: {
        // paste files is async operation
        JobHandlePointer jobHandle = args->value(CallbackKey::kJobHandle).value<JobHandlePointer>();
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
        auto sources = args->value(CallbackKey::kSourceUrls).value<QList<QUrl>>();
        auto targets = args->value(CallbackKey::kTargets).value<QList<QUrl>>();
        d->callBackRenameFiles(sources, targets);
    } break;
    default:
        break;
    }
}
