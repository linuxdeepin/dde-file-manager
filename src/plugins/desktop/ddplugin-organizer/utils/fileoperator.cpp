// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileoperator_p.h"
#include "view/collectionview.h"
#include "models/collectionmodel.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/utils/fileutils.h>

#include <dfm-framework/event/event.h>

#include <QUrl>
#include <QVariant>
#include <QItemSelectionModel>

using namespace ddplugin_organizer;
using namespace dfmbase;
using namespace Global;

static constexpr char const kCollectionKey[] = "CollectionKey";
static constexpr char const kDropFilesIndex[] = "DropFilesIndex";

class FileOperatorGlobal : public FileOperator
{
};
Q_GLOBAL_STATIC(FileOperatorGlobal, fileOperatorGlobal)

FileOperatorPrivate::FileOperatorPrivate(FileOperator *qq)
    : q(qq)
{
    canvasOperator = dpfSlotChannel->push("ddplugin_canvas", "slot_CanvasViewPrivate_FileOperator").value<QObject *>();
    if (!canvasOperator)
        fmWarning() << "fail to get canvas file operator";

    // the callback of pasting file on canvas.
    QObject::connect(canvasOperator, SIGNAL(filePastedCallback()), q, SLOT(onCanvasPastedFiles()));
}

void FileOperatorPrivate::callBackPasteFiles(const JobInfoPointer info, const QVariant &custom)
{
    if (info->keys().contains(AbstractJobHandler::NotifyInfoKey::kCompleteTargetFilesKey)) {
        QList<QUrl> files = info->value(AbstractJobHandler::NotifyInfoKey::kCompleteTargetFilesKey).value<QList<QUrl>>();

        // last data
        q->clearPasteFileData();
        q->clearDropFileData();

        emit q->requestClearSelection();

        //  move file to collection if on custom mode
        QVariantMap datas = custom.toMap();
        QString key = datas.value(kCollectionKey).toString();
        if (!key.isEmpty()) {
            auto toDrop = files;
            emit q->requestDropFile(key, toDrop);

            // if file is not existed, record it and move in inserting event.
            for (const QUrl &url : toDrop)
                dropFileData.insert(url, key);
        }

        // then try to select file that is existed
        emit q->requestSelectFile(files, QItemSelectionModel::Select);

        // record the file is not existed and selecting it when it is inserted.
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        pasteFileData = files.toSet();
#else
        pasteFileData = QSet<QUrl>(files.begin(), files.end());
#endif
    }
}

void FileOperatorPrivate::callBackRenameFiles(const QList<QUrl> &sources, const QList<QUrl> &targets)
{
    q->clearRenameFileData();

    // clear selected and current
    emit q->requestClearSelection();

    Q_ASSERT(sources.count() == targets.count());

    for (int i = 0; i < targets.count(); ++i) {
        renameFileData.insert(sources.at(i), targets.at(i));
    }
}

QList<QUrl> FileOperatorPrivate::getSelectedUrls(const CollectionView *view) const
{
    auto indexs = view->selectionModel()->selectedIndexes();
    QList<QUrl> urls;
    for (auto index : indexs) {
        urls << view->model()->fileUrl(index);
    }

    return urls;
}

void FileOperatorPrivate::filterDesktopFile(QList<QUrl> &urls)
{
    // computer and trash desktop files cannot be copied or cut.
    // filter the URL of these files copied and cut through shortcut keys here
    urls.removeAll(DesktopAppUrl::computerDesktopFileUrl());
    urls.removeAll(DesktopAppUrl::trashDesktopFileUrl());
    urls.removeAll(DesktopAppUrl::homeDesktopFileUrl());
}

FileOperator::FileOperator(QObject *parent)
    : QObject(parent), d(new FileOperatorPrivate(this))
{
    d->callBack = std::bind(&FileOperator::callBackFunction, this, std::placeholders::_1);
}

FileOperator::~FileOperator()
{
}

FileOperator *FileOperator::instance()
{
    return fileOperatorGlobal;
}

void FileOperator::setDataProvider(CollectionDataProvider *provider)
{
    d->provider = provider;
}

void FileOperator::copyFiles(const CollectionView *view)
{
    auto &&urls = d->getSelectedUrls(view);
    d->filterDesktopFile(urls);
    if (urls.isEmpty()) {
        fmDebug() << "No files to copy";
        return;
    }

    dpfSignalDispatcher->publish(GlobalEventType::kWriteUrlsToClipboard, view->winId(), ClipBoard::ClipboardAction::kCopyAction, urls);
}

void FileOperator::copyFilePath(const CollectionView *view)
{
    auto &&urls = d->getSelectedUrls(view);
    dpfSignalDispatcher->publish(GlobalEventType::kCopyFilePath, urls);
}

void FileOperator::cutFiles(const CollectionView *view)
{
    auto &&urls = d->getSelectedUrls(view);
    d->filterDesktopFile(urls);
    if (urls.isEmpty()) {
        fmDebug() << "No files to cut";
        return;
    }

    dpfSignalDispatcher->publish(GlobalEventType::kWriteUrlsToClipboard, view->winId(), ClipBoard::ClipboardAction::kCutAction, urls);
}

void FileOperator::pasteFiles(const CollectionView *view, const QString &targetColletion)
{
    auto urls = ClipBoard::instance()->clipboardFileUrlList();
    ClipBoard::ClipboardAction action = ClipBoard::instance()->clipboardAction();
    // 深信服和云桌面的远程拷贝获取的clipboardFileUrlList都是空
    if (action == ClipBoard::kRemoteCopiedAction) {   // 远程协助
        fmInfo() << "Remote Assistance Copy: set Current Url to Clipboard";
        ClipBoard::setCurUrlToClipboardForRemote(view->model()->rootUrl());
        return;
    }

    if (ClipBoard::kRemoteAction == action) {
        fmInfo() << "Processing remote clipboard action";
        dpfSignalDispatcher->publish(GlobalEventType::kCopy, view->winId(), urls, view->model()->rootUrl(),
                                     AbstractJobHandler::JobFlag::kCopyRemote, nullptr);
        return;
    }

    if (urls.isEmpty()) {
        fmDebug() << "No files in clipboard to paste";
        return;
    }

    QVariantMap data;
    // the taget collection that pasted file will be move to.
    data.insert(kCollectionKey, targetColletion);
    QPair<FileOperatorPrivate::CallBackFunc, QVariant> funcData(FileOperatorPrivate::kCallBackPasteFiles, data);
    QVariant custom = QVariant::fromValue(funcData);

    if (ClipBoard::kCopyAction == action) {
        dpfSignalDispatcher->publish(GlobalEventType::kCopy, view->winId(), urls, view->model()->rootUrl(),
                                     AbstractJobHandler::JobFlag::kNoHint, nullptr, custom, d->callBack);
    } else if (ClipBoard::kCutAction == action) {
        dpfSignalDispatcher->publish(GlobalEventType::kCutFile, view->winId(), urls, view->model()->rootUrl(),
                                     AbstractJobHandler::JobFlag::kNoHint, nullptr, custom, d->callBack);
        // clear clipboard after cutting files from clipboard
        ClipBoard::instance()->clearClipboard();
    } else {
        fmWarning() << "clipboard action:" << action << "    urls:" << urls;
    }
}

void FileOperator::openFiles(const CollectionView *view)
{
    auto &&urls = d->getSelectedUrls(view);
    if (!urls.isEmpty())
        openFiles(view, urls);
}

void FileOperator::openFiles(const CollectionView *view, const QList<QUrl> &urls)
{
    dpfSignalDispatcher->publish(GlobalEventType::kOpenFiles, view->winId(), urls);
}

void FileOperator::renameFile(int wid, const QUrl &oldUrl, const QUrl &newUrl)
{
    dpfSignalDispatcher->publish(GlobalEventType::kRenameFile, wid, oldUrl, newUrl, DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kNoHint);
}

void FileOperator::renameFiles(const CollectionView *view, const QList<QUrl> &urls, const QPair<QString, QString> &pair, const bool replace)
{
    QVariantMap data;
    data.insert(kCollectionKey, view->id());

    QPair<FileOperatorPrivate::CallBackFunc, QVariant> funcData(FileOperatorPrivate::kCallBackRenameFiles, data);
    QVariant custom = QVariant::fromValue(funcData);

    dpfSignalDispatcher->publish(GlobalEventType::kRenameFiles, view->winId(), urls, pair, replace, custom, d->callBack);
}

void FileOperator::renameFiles(const CollectionView *view, const QList<QUrl> &urls, const QPair<QString, AbstractJobHandler::FileNameAddFlag> pair)
{
    QVariantMap data;
    data.insert(kCollectionKey, view->id());

    QPair<FileOperatorPrivate::CallBackFunc, QVariant> funcData(FileOperatorPrivate::kCallBackRenameFiles, data);
    QVariant custom = QVariant::fromValue(funcData);

    dpfSignalDispatcher->publish(GlobalEventType::kRenameFiles, view->winId(), urls, pair, custom, d->callBack);
}

void FileOperator::moveToTrash(const CollectionView *view)
{
    auto &&urls = d->getSelectedUrls(view);
    if (urls.isEmpty()) {
        fmDebug() << "No files selected to move to trash";
        return;
    }

    fmInfo() << "Moving" << urls.size() << "files to trash";
    dpfSignalDispatcher->publish(GlobalEventType::kMoveToTrash, view->winId(), urls, AbstractJobHandler::JobFlag::kNoHint, nullptr);
}

void FileOperator::deleteFiles(const CollectionView *view)
{
    auto &&urls = d->getSelectedUrls(view);
    if (urls.isEmpty()) {
        fmDebug() << "No files selected to delete";
        return;
    }

    fmWarning() << "Permanently deleting" << urls.size() << "files";
    dpfSignalDispatcher->publish(GlobalEventType::kDeleteFiles, view->winId(), urls, AbstractJobHandler::JobFlag::kNoHint, nullptr);
}

void FileOperator::undoFiles(const CollectionView *view)
{
    dpfSignalDispatcher->publish(GlobalEventType::kRevocation,
                                 view->winId(), nullptr);
}

void FileOperator::previewFiles(const CollectionView *view)
{
    auto selectUrls = d->getSelectedUrls(view);
    if (selectUrls.isEmpty()) {
        fmDebug() << "No files selected for preview";
        return;
    }

    QList<QUrl> currentDirUrls = view->dataProvider()->items(view->id());
    fmInfo() << "Previewing" << selectUrls.size() << "files";
    dpfSlotChannel->push("dfmplugin_fileoperations", "slot_Operation_FilesPreview", view->topLevelWidget()->winId(), selectUrls, currentDirUrls);
}

void FileOperator::showFilesProperty(const CollectionView *view)
{
    auto &&urls = d->getSelectedUrls(view);
    if (urls.isEmpty()) {
        fmDebug() << "No files selected to show properties";
        return;
    }

    fmInfo() << "Showing properties for" << urls.size() << "files";
    dpfSlotChannel->push("dfmplugin_propertydialog", "slot_PropertyDialog_Show", urls, QVariantHash());
}

void FileOperator::dropFilesToCollection(const Qt::DropAction &action, const QUrl &targetUrl, const QList<QUrl> &urls, const QString &key, const int index)
{
    /*!
     * 从文管drop文件到集合，只能是追加（效果与从文管拖拽到桌面一致，而原因，也与桌面一样)
     * 1.底层执行粘贴（拖拽释放也是粘贴）是异步执行，只有所有粘贴执行完成后，才会调用回调函数和发送事件，
     * 在此之前，watcher已经监测到文件的创建，并通知model创建了文件index，创建时由于没有回调函数提供的位置信息和新文件名信息，
     * 文件只能按顺序追加。
     * 2.另一方面，底层无法做到在执行粘贴之前，就返回所有的最终文件名，因为其是在执行的过程中，逐个获取最终文件名称的，
     * 因为存在同名文件时，需要弹窗让用户选择“共存”、“替换”或“跳过”选项。
     * 3.另一种情况，底层已经掉了回调和发送事件，而model还没有将所有文件创建完毕，此时在回调中直接调用选中文件，存在部分文件选中失败的可能。
     * 4.为了解决上述问题，可能的解决方案是在收到回调函数之后，延迟一定的时间，再去选中（原文管流程）。
     * 5.但是，对于文件存放集合的问题则无法通过延迟解决，且，在弹窗让用户选中共存、跳过等时，实际已经有部分文件被创建成功，
     * 由于缺少回调函数提供的位置信息，导致新创建的文件不会被集合劫持，从而会先显示在桌面上（按空位自动存放）。最后等回调的延迟超时后，
     * 才会从桌面移动到集合中！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！
     * 6.只能底层修改逻辑，让执行粘贴之前，返回所有最终文件名？？？包括重名的文件，也需要在开始粘贴之前，让用户完成所有选择后，再开始执行粘贴操作。
     * 7.另外，即使在粘贴之前回调告诉了所有文件名称与所属集合信息，但是在执行粘贴的过程中，由于其他原因（比如用户手动修改了某个文件的名称），
     * 导致又出现了文件重名，此时还是需要弹窗让用户选择？那么又需要通过一个事件向外通知该特殊情况？
     */

    QVariantMap data;
    data.insert(kCollectionKey, key);
    data.insert(kDropFilesIndex, index);
    QPair<FileOperatorPrivate::CallBackFunc, QVariant> funcData(FileOperatorPrivate::kCallBackPasteFiles, data);
    QVariant custom = QVariant::fromValue(funcData);

    // default is copy file
    auto type = action == Qt::MoveAction ? GlobalEventType::kCutFile : GlobalEventType::kCopy;

    dpfSignalDispatcher->publish(type, 0, urls, targetUrl, AbstractJobHandler::JobFlag::kNoHint, nullptr, custom, d->callBack);
}

void FileOperator::dropFilesToCanvas(const Qt::DropAction &action, const QUrl &targetUrl, const QList<QUrl> &urls)
{
    // default is copy file
    auto type = action == Qt::MoveAction ? GlobalEventType::kCutFile : GlobalEventType::kCopy;
    dpfSignalDispatcher->publish(type, 0, urls, targetUrl, AbstractJobHandler::JobFlag::kNoHint, nullptr);
}

void FileOperator::dropToTrash(const QList<QUrl> &urls)
{
    dpfSignalDispatcher->publish(GlobalEventType::kMoveToTrash, 0, urls, AbstractJobHandler::JobFlag::kNoHint, nullptr);
}

void FileOperator::dropToApp(const QList<QUrl> &urls, const QString &app)
{
    QList<QString> apps { app };
    dpfSignalDispatcher->publish(GlobalEventType::kOpenFilesByApp, 0, urls, apps);
}

QHash<QUrl, QUrl> FileOperator::renameFileData() const
{
    return d->renameFileData;
}

void FileOperator::removeRenameFileData(const QUrl &oldUrl)
{
    d->renameFileData.remove(oldUrl);
}

void FileOperator::clearRenameFileData()
{
    d->renameFileData.clear();
}

QUrl FileOperator::touchFileData() const
{
    // special calling
    QUrl ret;
    if (d->canvasOperator) {
        QPair<QString, QPair<int, QPoint>> touch;
        QMetaObject::invokeMethod(d->canvasOperator, "touchFileData", Qt::DirectConnection,
                                  QReturnArgument<QPair<QString, QPair<int, QPoint>>>("QPair<QString, QPair<int, QPoint>>", touch));
        ret = QUrl(touch.first);
    }
    return ret;
}

void FileOperator::clearTouchFileData()
{
    // special calling
    if (d->canvasOperator) {
        QMetaObject::invokeMethod(d->canvasOperator,
                                  "clearTouchFileData", Qt::DirectConnection);
    }
}

QSet<QUrl> FileOperator::pasteFileData() const
{
    return d->pasteFileData;
}

void FileOperator::removePasteFileData(const QUrl &oldUrl)
{
    if (d->canvasOperator) {
        QMetaObject::invokeMethod(d->canvasOperator,
                                  "removePasteFileData", Qt::DirectConnection, Q_ARG(QUrl, oldUrl));
    }
    d->pasteFileData.remove(oldUrl);
}

void FileOperator::clearPasteFileData()
{
    // special calling
    if (d->canvasOperator) {
        QMetaObject::invokeMethod(d->canvasOperator,
                                  "clearPasteFileData", Qt::DirectConnection);
    }

    d->pasteFileData.clear();
}

QHash<QUrl, QString> FileOperator::dropFileData() const
{
    return d->dropFileData;
}

void FileOperator::removeDropFileData(const QUrl &oldUrl)
{
    d->dropFileData.remove(oldUrl);
}

void FileOperator::clearDropFileData()
{
    d->dropFileData.clear();
}

void FileOperator::onCanvasPastedFiles()
{
    QSet<QUrl> ret;
    // special calling
    if (d->canvasOperator) {
        QMetaObject::invokeMethod(d->canvasOperator,
                                  "pasteFileData", Qt::DirectConnection, QReturnArgument<QSet<QUrl>>("QSet<QUrl>", ret));
    }

    // just clear self
    d->pasteFileData.clear();

    // clear drop
    clearDropFileData();

    // no need to clear selecion since the canvas(FileOperatorProxyPrivate) has done it.
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    auto list = ret.toList();
#else
    auto list = QList<QUrl>(ret.begin(), ret.end());
#endif
    emit requestSelectFile(list, QItemSelectionModel::Select);

    // clear canvas
    for (const QUrl &url : ret) {
        if (!list.contains(url))
            removePasteFileData(url);
    }

    // record the file is not existed and selecting it when it is inserted.
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    d->pasteFileData = list.toSet();
#else
    d->pasteFileData = QSet<QUrl>(list.begin(), list.end());

#endif
}

void FileOperator::callBackFunction(const AbstractJobHandler::CallbackArgus args)
{
    const QVariant &customValue = args->value(AbstractJobHandler::CallbackKey::kCustom);
    const QPair<FileOperatorPrivate::CallBackFunc, QVariant> &custom = customValue.value<QPair<FileOperatorPrivate::CallBackFunc, QVariant>>();
    const FileOperatorPrivate::CallBackFunc funcKey = custom.first;

    switch (funcKey) {
#if 0   // touch file is processed by canvas. edit it at inserting by calling touchFileData with FileOperatorProxy
    case FileOperatorPrivate::CallBackFunc::kCallBackTouchFile:
    case FileOperatorPrivate::CallBackFunc::kCallBackTouchFolder: {
        // Folder also belong to files
        // touch file is sync operation

        auto targets = args->value(AbstractJobHandler::CallbackKey::kTargets).value<QList<QUrl>>();
        if (Q_UNLIKELY(targets.count() != 1)) {
            fmWarning() << "unknow error.touch file successed,target urls is:" << targets;
        }

        d->callBackTouchFile(targets.first(), custom.second.toMap());
    } break;
#endif
    case FileOperatorPrivate::CallBackFunc::kCallBackPasteFiles: {
        // paste files is async operation
        JobHandlePointer jobHandle = args->value(AbstractJobHandler::CallbackKey::kJobHandle).value<JobHandlePointer>();

        if (jobHandle->currentState() != AbstractJobHandler::JobState::kStopState) {
            connect(jobHandle.get(), &AbstractJobHandler::finishedNotify, d.get(), [this, custom](const JobInfoPointer &infoPointer) {
                d->callBackPasteFiles(infoPointer, custom.second);
            });
        } else {
            JobInfoPointer infoPointer = jobHandle->getTaskInfoByNotifyType(AbstractJobHandler::NotifyType::kNotifyFinishedKey);
            d->callBackPasteFiles(infoPointer, custom.second);
        }
    } break;
    case FileOperatorPrivate::CallBackFunc::kCallBackRenameFiles: {
        auto sources = args->value(AbstractJobHandler::CallbackKey::kSourceUrls).value<QList<QUrl>>();
        auto targets = args->value(AbstractJobHandler::CallbackKey::kTargets).value<QList<QUrl>>();
        d->callBackRenameFiles(sources, targets);
    } break;
    default:
        break;
    }
}
