// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "previewdialogmanager.h"
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>

#include <dfm-framework/dpf.h>

#include <DPlatformWindowHandle>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_filepreview;

PreviewDialogManager *PreviewDialogManager::instance()
{
    static PreviewDialogManager previewManager;
    return &previewManager;
}

void PreviewDialogManager::onPreviewDialogClose()
{
    exitTimer->start(60000);
}

PreviewDialogManager::PreviewDialogManager(QObject *parent)
    : QObject(parent)
{
    exitTimer = new QTimer(this);
    exitTimer->setSingleShot(true);
    connect(exitTimer, &QTimer::timeout, this, []() {
        QCoreApplication::quit();
    });
}

void PreviewDialogManager::showPreviewDialog(const quint64 winId, const QList<QUrl> &selecturls, const QList<QUrl> &dirUrl)
{
    Q_UNUSED(winId)

    if (selecturls.isEmpty())
        return;

    bool hasInvalidSymlink = false;
    for (const QUrl &url : selecturls) {
        const FileInfoPointer &info = InfoFactory::create<FileInfo>(url);

        if (info && (url.isLocalFile() || info->exists())) {
            // 判断链接文件的源文件是否存在
            if (info->isAttributes(OptInfoType::kIsSymLink)) {
                QUrl targetUrl = QUrl::fromLocalFile(info->pathOf(PathInfoType::kSymLinkTarget));
                if (!targetUrl.isValid()) {
                    hasInvalidSymlink = true;
                    continue;
                }

                dfmio::DFile file(targetUrl);
                if (!file.exists()) {
                    hasInvalidSymlink = true;
                    continue;
                }

                const FileInfoPointer linkInfo = InfoFactory::create<FileInfo>(targetUrl);
                if (linkInfo && !linkInfo->exists() && linkInfo->timeOf(TimeInfoType::kCreateTimeSecond) == 0) {
                    info->refresh();
                    linkInfo->refresh();
                }
            }
        }
    }

    // 链接文件源文件不存在或找不到的情况，弹错误提示窗
    if (hasInvalidSymlink) {
        DialogManager::instance()->showErrorDialog(tr("Unable to find the original file"), QString());
        return;
    }

    exitTimer->stop();

    if (filePreviewDialog) {
        filePreviewDialog->close();
        filePreviewDialog = nullptr;
    }

    if (!filePreviewDialog) {
        filePreviewDialog = new FilePreviewDialog(selecturls, nullptr);
        DPlatformWindowHandle::enableDXcbForWindow(filePreviewDialog, true);
        filePreviewDialog->setCurrentWinID(winId);
        connect(filePreviewDialog, &FilePreviewDialog::signalCloseEvent, this, &PreviewDialogManager::onPreviewDialogClose);
    }

    if (selecturls.count() == 1)
        filePreviewDialog->setEntryUrlList(dirUrl);

    filePreviewDialog->show();
    filePreviewDialog->raise();
    filePreviewDialog->activateWindow();

    filePreviewDialog->moveToCenter();
}
