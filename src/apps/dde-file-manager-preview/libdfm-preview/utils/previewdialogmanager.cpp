// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "previewdialogmanager.h"
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>

#include <dfm-framework/dpf.h>

#include <DPlatformWindowHandle>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_filepreview;

Q_DECLARE_LOGGING_CATEGORY(logLibFilePreview)

PreviewDialogManager *PreviewDialogManager::instance()
{
    static PreviewDialogManager previewManager;
    return &previewManager;
}

void PreviewDialogManager::onPreviewDialogClose()
{
    if (filePreviewDialog) {
        // Disconnect all signals to prevent any further interactions
        filePreviewDialog->disconnect();
        
        // Use single shot timer to ensure proper cleanup order
        // This allows the dialog's closeEvent to complete its cleanup first
        QTimer::singleShot(0, this, [this]() {
            if (filePreviewDialog) {
                filePreviewDialog->deleteLater();
                filePreviewDialog = nullptr;
            }
        });
    }
    
    qCInfo(logLibFilePreview) << "PreviewDialogManager: preview dialog closed, starting exit timer";
    exitTimer->start(60000);
}

PreviewDialogManager::PreviewDialogManager(QObject *parent)
    : QObject(parent)
{
    exitTimer = new QTimer(this);
    exitTimer->setSingleShot(true);
    connect(exitTimer, &QTimer::timeout, this, []() {
        qCInfo(logLibFilePreview) << "PreviewDialogManager: exit timer expired, quitting application";
        QCoreApplication::quit();
    });
    qCDebug(logLibFilePreview) << "PreviewDialogManager: initialized with 60s exit timer";
}

void PreviewDialogManager::showPreviewDialog(const quint64 winId, const QList<QUrl> &selecturls, const QList<QUrl> &dirUrl)
{
    Q_UNUSED(winId)

    if (selecturls.isEmpty()) {
        qCWarning(logLibFilePreview) << "PreviewDialogManager: cannot show preview dialog - no URLs provided";
        return;
    }

    qCInfo(logLibFilePreview) << "PreviewDialogManager: showing preview dialog for" << selecturls.size() << "files";

    bool hasInvalidSymlink = false;
    int invalidCount = 0;
    for (const QUrl &url : selecturls) {
        const FileInfoPointer &info = InfoFactory::create<FileInfo>(url);

        if (info && (url.isLocalFile() || info->exists())) {
            // 判断链接文件的源文件是否存在
            if (info->isAttributes(OptInfoType::kIsSymLink)) {
                QUrl targetUrl = QUrl::fromLocalFile(info->pathOf(PathInfoType::kSymLinkTarget));
                if (!targetUrl.isValid()) {
                    hasInvalidSymlink = true;
                    invalidCount++;
                    qCWarning(logLibFilePreview) << "PreviewDialogManager: invalid symlink target URL for:" << url.toString();
                    continue;
                }

                dfmio::DFile file(targetUrl);
                if (!file.exists()) {
                    hasInvalidSymlink = true;
                    invalidCount++;
                    qCWarning(logLibFilePreview) << "PreviewDialogManager: symlink target does not exist:" << targetUrl.toString() << "for symlink:" << url.toString();
                    continue;
                }

                const FileInfoPointer linkInfo = InfoFactory::create<FileInfo>(targetUrl);
                if (linkInfo && !linkInfo->exists() && linkInfo->timeOf(TimeInfoType::kCreateTimeSecond) == 0) {
                    qCDebug(logLibFilePreview) << "PreviewDialogManager: refreshing file info for symlink target:" << targetUrl.toString();
                    info->refresh();
                    linkInfo->refresh();
                }
            }
        } else {
            qCWarning(logLibFilePreview) << "PreviewDialogManager: file does not exist or is not accessible:" << url.toString();
        }
    }

    // 链接文件源文件不存在或找不到的情况，弹错误提示窗
    if (hasInvalidSymlink) {
        qCWarning(logLibFilePreview) << "PreviewDialogManager: found" << invalidCount << "invalid symlinks, showing error dialog";
        DialogManager::instance()->showErrorDialog(tr("Unable to find the original file"), QString());
        return;
    }

    qCDebug(logLibFilePreview) << "PreviewDialogManager: stopping exit timer";
    exitTimer->stop();

    bool updatePos = true;
    if (filePreviewDialog) {
        updatePos = false;
        qCDebug(logLibFilePreview) << "PreviewDialogManager: closing existing preview dialog";
        filePreviewDialog->updatePreviewList(selecturls);
    } else {
        qCInfo(logLibFilePreview) << "PreviewDialogManager: creating new preview dialog";
        filePreviewDialog = new FilePreviewDialog(selecturls, nullptr);
        DPlatformWindowHandle::enableDXcbForWindow(filePreviewDialog, true);
        connect(filePreviewDialog, &FilePreviewDialog::signalCloseEvent, this, &PreviewDialogManager::onPreviewDialogClose);
    }

    filePreviewDialog->setCurrentWinID(winId);
    if (selecturls.count() == 1) {
        qCDebug(logLibFilePreview) << "PreviewDialogManager: single file preview, setting entry URL list with" << dirUrl.size() << "directory URLs";
        filePreviewDialog->setEntryUrlList(dirUrl);
    }

    if (!filePreviewDialog->hasValidPreview()) {
        qCWarning(logLibFilePreview) << "PreviewDialogManager: preview creation failed, not showing dialog";
        filePreviewDialog->deleteLater();
        filePreviewDialog = nullptr;
        return;
    }

    qCInfo(logLibFilePreview) << "PreviewDialogManager: displaying preview dialog";
    if (updatePos) {
        filePreviewDialog->show();
        filePreviewDialog->moveToCenter();
    }
    filePreviewDialog->raise();
    filePreviewDialog->activateWindow();
}
