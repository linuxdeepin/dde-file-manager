/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#include "previewdialogmanager.h"
#include "dfm-base/base/schemefactory.h"

#include <dfm-framework/dpf.h>

#include <DPlatformWindowHandle>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_filepreview;

PreviewDialogManager *PreviewDialogManager::instance()
{
    static PreviewDialogManager previewManager;
    return &previewManager;
}

PreviewDialogManager::PreviewDialogManager(QObject *parent)
    : QObject(parent)
{
}

void PreviewDialogManager::showPreviewDialog(const quint64 winId, const QList<QUrl> &selecturls, const QList<QUrl> dirUrl)
{
    Q_UNUSED(winId)

    if (selecturls.isEmpty())
        return;

    bool hasInvalidSymlink = false;
    for (const QUrl &url : selecturls) {
        const AbstractFileInfoPointer &info = InfoFactory::create<AbstractFileInfo>(url);

        if (info && (info->url().isLocalFile() || info->exists())) {
            //判断链接文件的源文件是否存在
            if (info->isSymLink()) {
                QUrl targetUrl = QUrl::fromLocalFile(info->symLinkTarget());
                if (!targetUrl.isValid()) {
                    hasInvalidSymlink = true;
                    continue;
                }

                const AbstractFileInfoPointer linkInfo = InfoFactory::create<AbstractFileInfo>(targetUrl);
                if (!linkInfo || !linkInfo->exists()) {
                    hasInvalidSymlink = true;
                    continue;
                }
            }
        }
    }

    //链接文件源文件不存在或找不到的情况，弹错误提示窗
    if (hasInvalidSymlink) {
        DialogManager::instance()->showErrorDialog(tr("Unable to find the original file"), QString());
        return;
    }

    if (filePreviewDialog) {
        filePreviewDialog->close();
        filePreviewDialog = nullptr;
    }

    if (!filePreviewDialog) {
        filePreviewDialog = new FilePreviewDialog(selecturls, nullptr);
        DPlatformWindowHandle::enableDXcbForWindow(filePreviewDialog, true);
        filePreviewDialog->setCurrentWinID(winId);
    }

    if (selecturls.count() == 1)
        filePreviewDialog->setEntryUrlList(dirUrl);

    filePreviewDialog->show();
    filePreviewDialog->raise();
    filePreviewDialog->activateWindow();

    filePreviewDialog->moveToCenter();
}
