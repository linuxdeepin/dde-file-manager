// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
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
static constexpr char const kViewObject[] = "ViewObject";

class FileOperatorGlobal : public FileOperator
{
};
Q_GLOBAL_STATIC(FileOperatorGlobal, fileOperatorGlobal)

FileOperatorPrivate::FileOperatorPrivate(FileOperator *qq)
    : q(qq)
{
}

void FileOperatorPrivate::callBackTouchFile(const QUrl &target, const QVariantMap &customData)
{
    /*
     * 1.在自动集合模式下，无需关注该流程。即菜单响应不会被拦截，直接由canvas插件响应，
     *   被新建的文件根据需求，将直接被分类器分配到对应的集合中，并置顶显示。
     *
     * 2.在自定义集合模式下，需要拦截新建菜单的响应，调用FileOperator::touchFile和
     *   FileOperator::touchFolder（待添加的函数，具体实现参考canvas插件中的FileOperatorProxy类）
     *   ，以在本回调函数中记录底层返回的即将新建的文件名称。记录之后，在真正的文件被创建时，
     *   集合才能第一时间从canvas中过滤出该文件，防止该文件被首先显示到canvas中。
     *   另外，此场景新建文件的位置，是置顶显示还是鼠标最近的可用位置需要与产品确认。
     *
    */
}

void FileOperatorPrivate::callBackPasteFiles(const JobInfoPointer info)
{
    // todo(wangcl) 文件粘贴、拖拽释放的回调响应流程。流程无法满足需求，待商榷方案。
    if (info->keys().contains(AbstractJobHandler::NotifyInfoKey::kCompleteTargetFilesKey)) {
        //QList<QUrl> files = info->value(AbstractJobHandler::NotifyInfoKey::kCompleteTargetFilesKey).value<QList<QUrl>>();

        // todo(wangcl)
        // 如 dropFilesToCollection 的备注，此时文件是否全部创建，状态未知。
        // 即使通过延迟，让文件全部创建，依然存在文件先显示在桌面，再显示到集合的问题。
        // 另外，如果桌面本身存在同名文件，用户从文管拖拽文件到集合时选择了替换，此时文件显示到集合中？还是保持在桌面的原有位置？
    }
}

void FileOperatorPrivate::callBackRenameFiles(const QList<QUrl> &sources, const QList<QUrl> &targets, const CollectionView *view)
{
    q->clearRenameFileData();

    // clear selected and current
    view->selectionModel()->clearSelection();
    view->selectionModel()->clearCurrentIndex();

    Q_ASSERT(sources.count() == targets.count());

    for (int i = 0; i < targets.count(); ++i) {
        renameFileData.insert(sources.at(i), targets.at(i));
    }
}

QList<QUrl> FileOperatorPrivate::getSelectedUrls(const CollectionView *view) const
{
    auto indexs = view->selectedIndexes();
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
    if (urls.isEmpty())
        return;

    dpfSignalDispatcher->publish(GlobalEventType::kWriteUrlsToClipboard, view->winId(), ClipBoard::ClipboardAction::kCopyAction, urls);
}

void FileOperator::cutFiles(const CollectionView *view)
{
    auto &&urls = d->getSelectedUrls(view);
    d->filterDesktopFile(urls);
    if (urls.isEmpty())
        return;

    dpfSignalDispatcher->publish(GlobalEventType::kWriteUrlsToClipboard, view->winId(), ClipBoard::ClipboardAction::kCutAction, urls);
}

void FileOperator::pasteFiles(const CollectionView *view)
{
    auto urls = ClipBoard::instance()->clipboardFileUrlList();
    ClipBoard::ClipboardAction action = ClipBoard::instance()->clipboardAction();
    // 深信服和云桌面的远程拷贝获取的clipboardFileUrlList都是空
    if (action == ClipBoard::kRemoteCopiedAction) {   // 远程协助
        qInfo() << "Remote Assistance Copy: set Current Url to Clipboard";
        ClipBoard::setCurUrlToClipboardForRemote(view->model()->rootUrl());
        return;
    }

    if (ClipBoard::kRemoteAction == action) {
        dpfSignalDispatcher->publish(GlobalEventType::kCopy, view->winId(), urls, view->model()->rootUrl(),
                                     AbstractJobHandler::JobFlag::kCopyRemote, nullptr, nullptr, QVariant(), nullptr);
        return;
    }

    if (urls.isEmpty())
        return;

    if (ClipBoard::kCopyAction == action) {
        dpfSignalDispatcher->publish(GlobalEventType::kCopy, view->winId(), urls, view->model()->rootUrl(), AbstractJobHandler::JobFlag::kNoHint, nullptr);
    } else if (ClipBoard::kCutAction == action) {
        dpfSignalDispatcher->publish(GlobalEventType::kCutFile, view->winId(), urls, view->model()->rootUrl(), AbstractJobHandler::JobFlag::kNoHint, nullptr);
        // clear clipboard after cutting files from clipboard
        ClipBoard::instance()->clearClipboard();
    } else {
        qWarning() << "clipboard action:" << action << "    urls:" << urls;
    }
}

void FileOperator::pasteFiles(const CollectionView *view, const QPoint pos)
{
    /*
     * 1.文件粘贴与通过右键菜单新建文件存在相似的逻辑，即自动整理模式下，
     *   直接由canvas插件响应，真实文件创建时，由分类器直接分配到对应的集合中即可，
     *   该功能由FileOperator::pasteFiles(const CollectionView *view)函数完成。
     *
     * 2.在自定义整理模式下，参考FileOperator::dropFilesToCollection中的备注信息。
     *
    */
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
    data.insert(kViewObject, reinterpret_cast<qlonglong>(view));

    QPair<FileOperatorPrivate::CallBackFunc, QVariant> funcData(FileOperatorPrivate::kCallBackRenameFiles, data);
    QVariant custom = QVariant::fromValue(funcData);

    dpfSignalDispatcher->publish(GlobalEventType::kRenameFiles, view->winId(), urls, pair, replace, custom, d->callBack);
}

void FileOperator::renameFiles(const CollectionView *view, const QList<QUrl> &urls, const QPair<QString, AbstractJobHandler::FileNameAddFlag> pair)
{
    QVariantMap data;
    data.insert(kViewObject, reinterpret_cast<qlonglong>(view));

    QPair<FileOperatorPrivate::CallBackFunc, QVariant> funcData(FileOperatorPrivate::kCallBackRenameFiles, data);
    QVariant custom = QVariant::fromValue(funcData);

    dpfSignalDispatcher->publish(GlobalEventType::kRenameFiles, view->winId(), urls, pair, custom, d->callBack);
}

void FileOperator::moveToTrash(const CollectionView *view)
{
    auto &&urls = d->getSelectedUrls(view);
    if (urls.isEmpty())
        return;

    dpfSignalDispatcher->publish(GlobalEventType::kMoveToTrash, view->winId(), urls, AbstractJobHandler::JobFlag::kNoHint, nullptr);
}

void FileOperator::deleteFiles(const CollectionView *view)
{
    auto &&urls = d->getSelectedUrls(view);
    if (urls.isEmpty())
        return;

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
    if (selectUrls.isEmpty())
        return;

    QList<QUrl> currentDirUrls = view->dataProvider()->items(view->id());
    dpfSlotChannel->push("dfmplugin_filepreview", "slot_PreviewDialog_Show", view->topLevelWidget()->winId(), selectUrls, currentDirUrls);
}

void FileOperator::showFilesProperty(const CollectionView *view)
{
    auto &&urls = d->getSelectedUrls(view);
    if (urls.isEmpty())
        return;

    dpfSlotChannel->push("dfmplugin_propertydialog", "slot_PropertyDialog_Show", urls, QVariantHash());
}

void FileOperator::dropFilesToCollection(const Qt::DropAction &action, const QUrl &targetUrl, const QList<QUrl> &urls, const QString &key, const int index)
{
    // todo(wangcl) 逻辑流程无法满足需求，现有方案会导致文件先出现在桌面，再移动动到集合中
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

void FileOperator::callBackFunction(const AbstractJobHandler::CallbackArgus args)
{
    const QVariant &customValue = args->value(AbstractJobHandler::CallbackKey::kCustom);
    const QPair<FileOperatorPrivate::CallBackFunc, QVariant> &custom = customValue.value<QPair<FileOperatorPrivate::CallBackFunc, QVariant>>();
    const FileOperatorPrivate::CallBackFunc funcKey = custom.first;

    switch (funcKey) {
    case FileOperatorPrivate::CallBackFunc::kCallBackTouchFile:
    case FileOperatorPrivate::CallBackFunc::kCallBackTouchFolder: {
        // Folder also belong to files
        // touch file is sync operation

        auto targets = args->value(AbstractJobHandler::CallbackKey::kTargets).value<QList<QUrl>>();
        if (Q_UNLIKELY(targets.count() != 1)) {
            qWarning() << "unknow error.touch file successed,target urls is:" << targets;
        }

        d->callBackTouchFile(targets.first(), custom.second.toMap());
    } break;
    case FileOperatorPrivate::CallBackFunc::kCallBackPasteFiles: {
        // paste files is async operation
        JobHandlePointer jobHandle = args->value(AbstractJobHandler::CallbackKey::kJobHandle).value<JobHandlePointer>();

        if (jobHandle->currentState() != AbstractJobHandler::JobState::kStopState) {
            connect(jobHandle.get(), &AbstractJobHandler::finishedNotify, d.get(), &FileOperatorPrivate::callBackPasteFiles);
        } else {
            JobInfoPointer infoPointer = jobHandle->getTaskInfoByNotifyType(AbstractJobHandler::NotifyType::kNotifyFinishedKey);
            d->callBackPasteFiles(infoPointer);
        }
    } break;
    case FileOperatorPrivate::CallBackFunc::kCallBackRenameFiles: {
        auto sources = args->value(AbstractJobHandler::CallbackKey::kSourceUrls).value<QList<QUrl>>();
        auto targets = args->value(AbstractJobHandler::CallbackKey::kTargets).value<QList<QUrl>>();
        auto viewData = custom.second.toMap();
        CollectionView *view = reinterpret_cast<CollectionView *>(viewData.value(kViewObject).toLongLong());
        if (Q_UNLIKELY(!view)) {
            qWarning() << "warning:can not get collection view.";
            break;
        }
        d->callBackRenameFiles(sources, targets, view);
    } break;
    default:
        break;
    }
}
