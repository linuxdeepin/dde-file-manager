/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             liuyangming<liuyangming@uniontech.com>
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
#include "recentfileshelper.h"
#include "recentmanager.h"
#include "events/recenteventcaller.h"

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/base/schemefactory.h"

#include <DDialog>
#include <DRecentManager>

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DPRECENT_USE_NAMESPACE

bool RecentFilesHelper::openFilesHandle(quint64 windowId, const QList<QUrl> urls, const QString *error)
{
    Q_UNUSED(error)

    QList<QUrl> redirectedFileUrls;
    for (QUrl url : urls) {
        url.setScheme(Global::kFile);
        redirectedFileUrls << url;
    }
    RecentEventCaller::sendOpenFiles(windowId, redirectedFileUrls);
    return true;
}

bool RecentFilesHelper::writeUrlToClipboardHandle(const quint64 windowId, const ClipBoard::ClipboardAction action, const QList<QUrl> urls)
{
    if (action == ClipBoard::ClipboardAction::kCutAction)
        return true;

    QList<QUrl> redirectedFileUrls;
    for (QUrl url : urls) {
        url.setScheme(Global::kFile);
        redirectedFileUrls << url;
    }
    RecentEventCaller::sendWriteToClipboard(windowId, action, redirectedFileUrls);
    return true;
}

JobHandlePointer RecentFilesHelper::deleteFilesHandle(const quint64 windowId, const QList<QUrl> sources, const AbstractJobHandler::JobFlags flags)
{
    Q_UNUSED(windowId)
    Q_UNUSED(flags)

    RecentFilesHelper::removeRecent(sources);

    return {};
}

void RecentFilesHelper::removeRecent(const QList<QUrl> &urls)
{
    DDialog dlg;
    dlg.setIcon(QIcon::fromTheme("dialog-warning"));
    dlg.addButton(QObject::tr("Cancel", "button"));
    dlg.addButton(QObject::tr("Remove", "button"), true, DDialog::ButtonRecommend);

    if (urls.size() == 1)
        dlg.setTitle(QObject::tr("Do you want to remove this item?"));
    else
        dlg.setTitle(QObject::tr("Do yout want to remove %1 items?").arg(urls.size()));
    dlg.setMessage(QObject::tr("It does not delete the original files"));

    int code = dlg.exec();
    if (code == 1) {
        QStringList list;
        for (const QUrl &url : urls) {
            //list << DUrl::fromLocalFile(url.path()).toString();
            //通过durl转换path会出现编码问题，这里直接用字符串拼出正确的path;
            QUrl newUrl = url;
            newUrl.setScheme(Global::kFile);
            list << newUrl.toString();
        }

        DRecentManager::removeItems(list);
    }
}
