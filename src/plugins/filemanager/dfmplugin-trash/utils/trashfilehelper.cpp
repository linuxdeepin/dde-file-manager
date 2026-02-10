// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "trashfilehelper.h"
#include "trashhelper.h"
#include "events/trasheventcaller.h"

#include <dfm-base/utils/fileutils.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/utils/universalutils.h>

#include <dfm-framework/event/event.h>
#include <dfm-io/dfmio_utils.h>

#include <QUrl>
#include <QFileInfo>

DPTRASH_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
TrashFileHelper *TrashFileHelper::instance()
{
    static TrashFileHelper ins;
    return &ins;
}

TrashFileHelper::TrashFileHelper(QObject *parent)
    : QObject(parent)
{
}

bool TrashFileHelper::cutFile(const quint64 windowId, const QList<QUrl> sources, const QUrl target, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags)
{
    if (target.scheme() != scheme())
        return false;

    if (sources.isEmpty()) {
        fmDebug() << "Trash: No source files provided for cut operation";
        return true;
    }

    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kMoveToTrash,
                                 windowId,
                                 sources, flags, nullptr);
    return true;
}

bool TrashFileHelper::copyFile(const quint64 windowId, const QList<QUrl> sources, const QUrl target, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags)
{
    if (target.scheme() != scheme())
        return false;

    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kMoveToTrash,
                                 windowId,
                                 sources, flags, nullptr);
    return true;
}

bool TrashFileHelper::moveToTrash(const quint64 windowId, const QList<QUrl> sources, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags)
{
    Q_UNUSED(flags)

    if (sources.isEmpty())
        return false;
    if (sources.first().scheme() != scheme())
        return false;
    // trash sub dir file do not run
    bool isTrashRoot = FileUtils::isTrashRootFile(sources.first());
    bool isParentTrashRoot = FileUtils::isTrashRootFile(UrlRoute::urlParent(sources.first()));
    if (!isTrashRoot && !isParentTrashRoot) {
        fmDebug() << "Trash: Files are not in trash root directory, operation skipped";
        return true;
    }

    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kCleanTrash,
                                 windowId,
                                 sources,
                                 DFMBASE_NAMESPACE::AbstractJobHandler::DeleteDialogNoticeType::kDeleteTashFiles, nullptr);
    return true;
}

bool TrashFileHelper::deleteFile(const quint64 windowId, const QList<QUrl> sources, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags)
{
    Q_UNUSED(flags)

    if (sources.isEmpty())
        return false;
    if (sources.first().scheme() != scheme())
        return false;
    // trash sub dir file do not run
    bool isTrashRoot = FileUtils::isTrashRootFile(sources.first());
    bool isParentTrashRoot = FileUtils::isTrashRootFile(UrlRoute::urlParent(sources.first()));
    if (!isTrashRoot && !isParentTrashRoot) {
        fmDebug() << "Trash: Files are not in trash root directory, delete operation skipped";
        return true;
    }

    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kCleanTrash,
                                 windowId,
                                 sources,
                                 DFMBASE_NAMESPACE::AbstractJobHandler::DeleteDialogNoticeType::kDeleteTashFiles, nullptr);
    return true;
}

bool TrashFileHelper::openFileInPlugin(quint64 windowId, const QList<QUrl> urls)
{
    Q_UNUSED(windowId);
    if (urls.isEmpty())
        return false;
    if (urls.first().scheme() != scheme())
        return false;

    bool isOpenFile = false;
    for (const QUrl &url : urls) {
        auto fileinfo = DFMBASE_NAMESPACE::InfoFactory::create<DFMBASE_NAMESPACE::FileInfo>(url);
        if (fileinfo && fileinfo->fileType() == FileInfo::FileType::kRegularFile) {
            isOpenFile = true;
            break;
        }
    }
    if (isOpenFile) {
        fmWarning() << "Trash: Attempting to open files in trash, showing warning dialog";
        const QString &strMsg = QObject::tr("Unable to open items in the trash, please restore it first");
        DialogManagerInstance->showMessageDialog(strMsg);
    }
    return isOpenFile;
}

bool TrashFileHelper::blockPaste(quint64 winId, const QList<QUrl> &fromUrls, const QUrl &to)
{
    Q_UNUSED(winId)
    if (fromUrls.isEmpty())
        return false;

    if (fromUrls.first().scheme() == scheme() && to.scheme() == scheme()) {
        DFMBASE_NAMESPACE::ClipBoard::clearClipboard();
        fmDebug() << "Trash: Blocking paste operation within trash directory";
        return true;
    }
    return false;
}

bool TrashFileHelper::disableOpenWidgetWidget(const QUrl &url, bool *result)
{
    if (url.scheme() != scheme())
        return false;

    *result = true;

    return true;
}

bool TrashFileHelper::handleCanTag(const QUrl &url, bool *canTag)
{
    if (url.scheme() == scheme() || FileUtils::isTrashFile(url)) {
        if (canTag)
            *canTag = false;
        return true;
    }

    return false;
}

bool TrashFileHelper::handleIsSubFile(const QUrl &parent, const QUrl &sub)
{
    if (parent.scheme() != scheme())
        return false;
    if (!FileUtils::isTrashFile(sub))
        return false;
    if (UniversalUtils::urlEquals(FileUtils::trashRootUrl(), parent))
        return true;

    return sub.path().contains(parent.path());
}

bool TrashFileHelper::handleNotCdComputer(const QUrl &url, QUrl *cdUrl)
{
    if (url.scheme() != scheme())
        return false;

    if (!cdUrl)
        return false;

    // 回收站中只能最上层目录可以删除和还原，那么它们的父目录都是回收站
    *cdUrl = FileUtils::trashRootUrl();
    return true;
}
