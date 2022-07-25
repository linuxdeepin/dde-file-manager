/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "corehelper.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"

#include <dfm-framework/event/event.h>

#include <QDir>
#include <QProcess>

Q_DECLARE_METATYPE(QList<QUrl> *)

DPCORE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

void CoreHelper::cd(quint64 windowId, const QUrl &url)
{
    Q_ASSERT(url.isValid());
    auto window = FMWindowsIns.findWindowById(windowId);

    if (!window) {
        qWarning() << "Invalid window id: " << windowId;
        return;
    }

    qInfo() << "cd to " << url;
    window->cd(url);

    QUrl titleUrl { url };
    QList<QUrl> urls {};
    bool ok = dpfHookSequence->run("dfmplugin_utils", "hook_UrlsTransform", titleUrl, &urls);

    if (ok && !urls.isEmpty())
        titleUrl = urls.first();

    auto fileInfo = InfoFactory::create<AbstractFileInfo>(titleUrl);
    if (fileInfo) {
        QUrl url { fileInfo->url() };
        window->setWindowTitle(fileInfo->fileDisplayName());
    }
}

void CoreHelper::openNewWindow(const QUrl &url)
{
    FMWindowsIns.showWindow(url, true);
}
