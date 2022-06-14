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

#include "services/common/delegate/delegateservice.h"

#include <DDialog>
#include <DRecentManager>
#include <DDesktopServices>

#include <dfm-io/dfmio_utils.h>

#include <QProcess>

DFMBASE_USE_NAMESPACE
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

bool RecentFilesHelper::openFileLocation(const QUrl &url)
{
    if (SysInfoUtils::isRootUser()) {
        QStringList urls { QStringList() << url.toLocalFile() };
        // call by platform 'mips', but file-manager.sh not ready, todo:max
        //        if (QProcess::startDetached("file-manager.sh", QStringList() << "--show-item" << urls << "--raw"))
        //            return true;

        return QProcess::startDetached("dde-file-manager", QStringList() << "--show-item" << urls << "--raw");
    }

    return DDesktopServices::showFileItem(delegateServIns->urlTransform(url));
}

void RecentFilesHelper::openFileLocation(const QList<QUrl> &urls)
{
    foreach (const QUrl &url, urls) {
        if (!openFileLocation(url))
            qWarning() << "failed to open: " << url.path();
    }
}

bool RecentFilesHelper::setPermissionHandle(const quint64 windowId, const QUrl url, const QFileDevice::Permissions permissions, QString *error)
{
    Q_UNUSED(windowId)
    Q_UNUSED(error)

    const QUrl &localUrl = RecentManager::urlTransform(url);
    DFMBASE_NAMESPACE::LocalFileHandler fileHandler;

    return fileHandler.setPermissions(localUrl, permissions);
}

bool RecentFilesHelper::createLinkFileHandle(const quint64 windowId, const QUrl url, const QUrl link, const bool force, const bool silence, QString *error)
{
    Q_UNUSED(windowId)
    Q_UNUSED(error)

    if (force) {
        const AbstractFileInfoPointer &toInfo = InfoFactory::create<AbstractFileInfo>(link);
        if (toInfo && toInfo->exists()) {
            DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
            fileHandler.deleteFile(link);
        }
    }

    QUrl urlValid { link };
    if (silence)
        urlValid = checkTargetUrl(link);

    DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
    const QUrl &localUrl = RecentManager::urlTransform(url);
    return fileHandler.createSystemLink(localUrl, urlValid);
}

QUrl RecentFilesHelper::checkTargetUrl(const QUrl &url)
{
    const QUrl &urlParent = DFMIO::DFMUtils::directParentUrl(url);
    if (!urlParent.isValid())
        return url;

    const QString &nameValid = FileUtils::nonExistSymlinkFileName(url, urlParent);
    if (!nameValid.isEmpty())
        return urlParent.toString() + QDir::separator() + nameValid;

    return url;
}
