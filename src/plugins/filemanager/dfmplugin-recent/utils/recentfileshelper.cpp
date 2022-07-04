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
#include "dfm-base/utils/sysinfoutils.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/file/local/localfilehandler.h"

#include <DDialog>
#include <DRecentManager>
#include <DDesktopServices>

#include <dfm-io/dfmio_utils.h>

#include <QProcess>

Q_DECLARE_METATYPE(QList<QUrl> *)

DFMBASE_USE_NAMESPACE
DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace dfmplugin_recent;

RecentFilesHelper *RecentFilesHelper::instance()
{
    static RecentFilesHelper instance;
    return &instance;
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
            newUrl.setScheme(Global::Scheme::kFile);
            list << newUrl.toString();
        }

        DRecentManager::removeItems(list);
    }
}

bool RecentFilesHelper::openFileLocation(const QUrl &url)
{
    if (SysInfoUtils::isRootUser()) {
        QStringList urls { QStringList() << url.toLocalFile() };
        // call by platform 'mips', but file-manager.sh not ready, todo:max
        //        if (QProcess::startDetached("file-manager.sh", QStringList() << "--show-item" << urls << "--raw"))
        //            return true;

        return QProcess::startDetached("dde-file-manager", QStringList() << "--show-item" << urls << "--raw");
    }

    QUrl localUrl = url;
    QList<QUrl> urls {};
    bool ok = dpfHookSequence->run("dfmplugin_utils", "hook_UrlsTransform", QList<QUrl>() << localUrl, &urls);
    if (ok && !urls.isEmpty())
        localUrl = urls.first();

    return DDesktopServices::showFileItem(localUrl);
}

void RecentFilesHelper::openFileLocation(const QList<QUrl> &urls)
{
    foreach (const QUrl &url, urls) {
        if (!openFileLocation(url))
            qWarning() << "failed to open: " << url.path();
    }
}

bool RecentFilesHelper::setPermissionHandle(const quint64 windowId, const QUrl url, const QFileDevice::Permissions permissions, bool *ok, QString *error)
{
    if (Global::Scheme::kRecent != url.scheme())
        return false;

    Q_UNUSED(windowId)

    const QUrl &localUrl = RecentManager::urlTransform(url);
    DFMBASE_NAMESPACE::LocalFileHandler fileHandler;

    bool succ = fileHandler.setPermissions(localUrl, permissions);
    if (!succ && error)
        *error = fileHandler.errorString();

    if (ok)
        *ok = succ;

    return true;
}

RecentFilesHelper::RecentFilesHelper(QObject *parent)
    : QObject(parent)
{
}
