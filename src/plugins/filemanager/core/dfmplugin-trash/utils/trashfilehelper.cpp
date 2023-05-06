// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
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

    if (sources.isEmpty())
        return true;

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
        if (fileinfo && fileinfo->isAttributes(OptInfoType::kIsFile)) {
            isOpenFile = true;
            break;
        }
    }
    if (isOpenFile) {
        const QString &strMsg = QObject::tr("Unable to open items in the trash, please restore it first");
        DialogManagerInstance->showMessageDialog(DFMBASE_NAMESPACE::DialogManager::kMsgWarn, strMsg);
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
        qDebug() << "The trash directory does not support paste!";
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
