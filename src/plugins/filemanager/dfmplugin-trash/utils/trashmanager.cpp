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

#include "trashmanager.h"
#include "trashfileinfo.h"
#include "trashfilewatcher.h"
#include "events/trasheventcaller.h"

#include "services/filemanager/workspace/workspaceservice.h"
#include "services/common/propertydialog/propertydialogservice.h"
#include "services/common/dialog/dialogservice.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/standardpaths.h"

#include <QFileInfo>
#include <QFile>
#include <QMenu>

DPTRASH_USE_NAMESPACE
DSB_FM_USE_NAMESPACE
DSC_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

TrashManager *TrashManager::instance()
{
    static TrashManager instance;
    return &instance;
}

QUrl TrashManager::rootUrl()
{
    QUrl url;
    url.setScheme(scheme());
    url.setPath("/");
    return url;
}

void TrashManager::contenxtMenuHandle(quint64 windowId, const QUrl &url, const QPoint &globalPos)
{
    QMenu *menu = new QMenu;
    menu->addAction(QObject::tr("Open in new window"), [url]() {
        TrashEventCaller::sendOpenWindow(url);
    });

    auto newTabAct = menu->addAction(QObject::tr("Open in new tab"), [windowId, url]() {
        TrashEventCaller::sendOpenTab(windowId, url);
    });

    auto &ctx = dpfInstance.serviceContext();
    auto workspaceService = ctx.service<WorkspaceService>(WorkspaceService::name());
    if (!workspaceService) {
        qCritical() << "Failed, trashManager contenxtMenuHandle \"WorkspaceService\" is empty";
        abort();
    }

    newTabAct->setDisabled(!workspaceService->tabAddable(windowId));

    menu->addSeparator();
    menu->addAction(QObject::tr("Empty Trash"), [windowId, url]() {
        QUrl url = TrashManager::toLocalFile(url);
        TrashEventCaller::sendEmptyTrash(windowId, { url });
    });

    menu->addSeparator();
    menu->addAction(QObject::tr("Properties"), [url]() {
        auto &ctx = dpfInstance.serviceContext();
        QString errStr;
        if (!ctx.load(PropertyDialogService::name(), &errStr)) {
            qCritical() << errStr;
            abort();
        }
        PropertyDialogService *service = ctx.service<PropertyDialogService>(PropertyDialogService::name());
        service->showTrashProperty(url);
    });
    menu->exec(globalPos);
    delete menu;
}

bool TrashManager::openFilesHandle(quint64 windowId, const QList<QUrl> urls, const QString *error)
{
    Q_UNUSED(error)
    auto &ctx = dpfInstance.serviceContext();
    QString errStr;
    if (!ctx.load(DialogService::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }
    DialogService *service = ctx.service<DialogService>(DialogService::name());

    bool isOpenFile = false;
    QList<QUrl> redirectedFileUrls;
    for (const QUrl &url : urls) {
        QUrl redirectedFileUrl = TrashManager::toLocalFile(url);
        QFileInfo fileInfo(redirectedFileUrl.path());
        if (fileInfo.isFile()) {
            isOpenFile = true;
            continue;
        }
        redirectedFileUrls << url;
    }
    if (!redirectedFileUrls.isEmpty())
        TrashEventCaller::sendOpenFiles(windowId, redirectedFileUrls);

    // Todo(yanghao)
    if (isOpenFile) {
        QString strMsg = tr("Unable to open items in the trash, please restore it first");
        service->showMessageDialog(DialogService::kMsgWarn, strMsg);
    }
    return true;
}

bool TrashManager::writeToClipBoardHandle(const quint64 windowId, const ClipBoard::ClipboardAction action, const QList<QUrl> urls)
{
    QList<QUrl> redirectedFileUrls;
    for (QUrl url : urls) {
        url.setScheme(SchemeTypes::kFile);
        redirectedFileUrls << url;
    }
    // Todo(yanghao)
    return true;
}

QUrl TrashManager::toLocalFile(const QUrl &url)
{
    return StandardPaths::location(StandardPaths::kTrashFilesPath) + url.path();
}

TrashManager::TrashManager(QObject *parent)
    : QObject(parent)
{
    init();
}

TrashManager::~TrashManager()
{
}

void TrashManager::init()
{
}
