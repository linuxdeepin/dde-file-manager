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
#include "recentfilehelper.h"
#include "recentmanager.h"
#include "events/recenteventcaller.h"

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/utils/sysinfoutils.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/file/local/localfilehandler.h"

#include <dfm-framework/event/event.h>

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

RecentFileHelper *RecentFileHelper::instance()
{
    static RecentFileHelper instance;
    return &instance;
}

void RecentFileHelper::removeRecent(const QList<QUrl> &urls)
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
        auto originPath = RecentManager::instance()->getRecentOriginPaths();
        for (const QUrl &url : urls) {
            if (originPath.contains(url)) {
                list << originPath[url];
                continue;
            }
            //list << DUrl::fromLocalFile(url.path()).toString();
            //通过durl转换path会出现编码问题，这里直接用字符串拼出正确的path;
            QUrl newUrl = url;
            newUrl.setScheme(Global::Scheme::kFile);
            list << newUrl.toString();
        }

        DRecentManager::removeItems(list);
    }
}

bool RecentFileHelper::openFileLocation(const QUrl &url)
{

    QUrl localUrl = url;
    QList<QUrl> urls {};
    bool ok = dpfHookSequence->run("dfmplugin_utils", "hook_UrlsTransform", QList<QUrl>() << localUrl, &urls);
    if (ok && !urls.isEmpty())
        localUrl = urls.first();

    const auto &fileInfo { InfoFactory::create<AbstractFileInfo>(localUrl) };
    QUrl parentUrl { fileInfo->urlOf(UrlInfoType::kParentUrl) };
    parentUrl.setQuery("selectUrl=" + localUrl.toString());

    return dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, parentUrl);
}

void RecentFileHelper::openFileLocation(const QList<QUrl> &urls)
{
    foreach (const QUrl &url, urls) {
        if (!openFileLocation(url))
            qWarning() << "failed to open: " << url.path();
    }
}

bool RecentFileHelper::setPermissionHandle(const quint64 windowId, const QUrl url, const QFileDevice::Permissions permissions, bool *ok, QString *error)
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

bool RecentFileHelper::cutFile(const quint64 windowId, const QList<QUrl> sources, const QUrl target, const AbstractJobHandler::JobFlags flags)
{
    Q_UNUSED(windowId)
    Q_UNUSED(sources)
    Q_UNUSED(flags)

    return target.scheme() == scheme();
}

bool RecentFileHelper::copyFile(const quint64, const QList<QUrl>, const QUrl target, const AbstractJobHandler::JobFlags)
{
    return target.scheme() == scheme();
}

bool RecentFileHelper::moveToTrash(const quint64 windowId, const QList<QUrl> sources, const AbstractJobHandler::JobFlags flags)
{
    if (sources.isEmpty())
        return false;
    if (sources.first().scheme() != scheme())
        return false;

    Q_UNUSED(windowId)
    Q_UNUSED(flags)

    removeRecent(sources);

    return true;
}

bool RecentFileHelper::openFileInPlugin(quint64 winId, QList<QUrl> urls)
{
    if (urls.isEmpty())
        return false;
    if (urls.first().scheme() != scheme())
        return false;

    QList<QUrl> redirectedFileUrls;
    for (QUrl url : urls) {
        url.setScheme(Global::Scheme::kFile);
        redirectedFileUrls << url;
    }
    RecentEventCaller::sendOpenFiles(winId, redirectedFileUrls);
    return true;
}

bool RecentFileHelper::linkFile(const quint64 windowId, const QUrl url, const QUrl link, const bool force, const bool silence)
{
    if (url.scheme() != scheme())
        return false;

    Q_UNUSED(windowId)

    if (force) {
        const AbstractFileInfoPointer &toInfo = InfoFactory::create<AbstractFileInfo>(link);
        if (toInfo && toInfo->exists()) {
            DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
            fileHandler.deleteFile(link);
        }
    }

    auto checkTargetUrl = [](const QUrl &url) -> QUrl {
        const QUrl &urlParent = DFMIO::DFMUtils::directParentUrl(url);
        if (!urlParent.isValid())
            return url;

        const QString &nameValid = FileUtils::nonExistSymlinkFileName(url, urlParent);
        if (!nameValid.isEmpty())
            return DFMIO::DFMUtils::buildFilePath(urlParent.toString().toStdString().c_str(),
                                                  nameValid.toStdString().c_str(), nullptr);

        return url;
    };

    QUrl urlValid { link };
    if (silence)
        urlValid = checkTargetUrl(link);

    DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
    const QUrl &localUrl = RecentManager::urlTransform(url);
    fileHandler.createSystemLink(localUrl, urlValid);

    return true;
}

bool RecentFileHelper::writeUrlsToClipboard(const quint64 windowId, const ClipBoard::ClipboardAction action, const QList<QUrl> urls)
{
    if (urls.isEmpty())
        return false;
    if (urls.first().scheme() != scheme())
        return false;
    if (action == ClipBoard::ClipboardAction::kCutAction)
        return true;

    QList<QUrl> redirectedFileUrls;
    for (QUrl url : urls) {
        url.setScheme(Global::Scheme::kFile);
        redirectedFileUrls << url;
    }
    RecentEventCaller::sendWriteToClipboard(windowId, action, redirectedFileUrls);
    return true;
}

bool RecentFileHelper::openFileInTerminal(const quint64 windowId, const QList<QUrl> urls)
{
    if (urls.isEmpty())
        return false;
    if (urls.first().scheme() != scheme())
        return false;

    Q_UNUSED(windowId);

    return true;
}

RecentFileHelper::RecentFileHelper(QObject *parent)
    : QObject(parent)
{
}
