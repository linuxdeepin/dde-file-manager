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

#include "trashhelper.h"
#include "trashfileinfo.h"
#include "trashfilewatcher.h"
#include "events/trasheventcaller.h"
#include "views/emptyTrashWidget.h"
#include "dfm_event_defines.h"

#include "services/filemanager/windows/windowsservice.h"
#include "services/filemanager/workspace/workspaceservice.h"
#include "services/common/propertydialog/propertydialogservice.h"
#include "services/common/trash/trashservice.h"

#include "dfm-framework/framework.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/utils/dialogmanager.h"
#include "dfm-base/base/standardpaths.h"

#include <DHorizontalLine>
#include <DApplicationHelper>

#include <QFileInfo>
#include <QFile>
#include <QMenu>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

DPTRASH_USE_NAMESPACE
DSB_FM_USE_NAMESPACE
DSC_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

TrashHelper *TrashHelper::instance()
{
    static TrashHelper instance;
    return &instance;
}

QUrl TrashHelper::rootUrl()
{
    QUrl url;
    url.setScheme(scheme());
    url.setPath("/");
    return url;
}

quint64 TrashHelper::windowId(QWidget *sender)
{
    auto &ctx = dpfInstance.serviceContext();
    auto windowService = ctx.service<WindowsService>(WindowsService::name());
    return windowService->findWindowId(sender);
}

void TrashHelper::contenxtMenuHandle(const quint64 windowId, const QUrl &url, const QPoint &globalPos)
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
        qCritical() << "Failed, TrashHelper contenxtMenuHandle \"WorkspaceService\" is empty";
        abort();
    }

    newTabAct->setDisabled(!workspaceService->tabAddable(windowId));

    menu->addSeparator();

    auto emptyTrashAct = menu->addAction(QObject::tr("Empty Trash"), [windowId, url]() {
        QUrl localUrl = TrashHelper::toLocalFile(url);
        TrashEventCaller::sendEmptyTrash(windowId, { localUrl });
    });
    emptyTrashAct->setDisabled(TrashHelper::isEmpty());

    menu->addSeparator();

    menu->addAction(QObject::tr("Properties"), [url]() {
        TrashEventCaller::sendTrashPropertyDialog(url);
    });
    menu->exec(globalPos);
    delete menu;
}

QFrame *TrashHelper::createEmptyTrashTopWidget()
{
    EmptyTrashWidget *emptyTrashWidget = new EmptyTrashWidget;
    QObject::connect(emptyTrashWidget, &EmptyTrashWidget::emptyTrash,

                     TrashHelper::instance(), [emptyTrashWidget] {
                         auto windId = TrashHelper::instance()->windowId(emptyTrashWidget);
                         TrashHelper::emptyTrash(windId);
                     });
    return emptyTrashWidget;
}

bool TrashHelper::showTopWidget(QWidget *w, const QUrl &url)
{
    Q_UNUSED(w)

    if (url == TrashHelper::fromTrashFile("/") && !TrashHelper::isEmpty()) {
        return true;
    } else {
        return false;
    }
}

QUrl TrashHelper::fromTrashFile(const QString &filePath)
{
    QUrl url;

    url.setScheme(TrashHelper::scheme());
    url.setPath(filePath);

    return url;
}

QUrl TrashHelper::fromLocalFile(const QString &filePath)
{
    QString path = filePath;
    return TrashHelper::fromTrashFile(path.remove(StandardPaths::location(StandardPaths::kTrashFilesPath)));
}

QUrl TrashHelper::toLocalFile(const QUrl &url)
{
    return QUrl::fromLocalFile(StandardPaths::location(StandardPaths::kTrashFilesPath) + url.path());
}

bool TrashHelper::isEmpty()
{
    QDir dir(StandardPaths::location(StandardPaths::kTrashFilesPath));

    if (!dir.exists())
        return true;

    dir.setFilter(QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);

    QDirIterator iterator(dir);

    return !iterator.hasNext();
}

void TrashHelper::emptyTrash(const quint64 windowId)
{
    dpfInstance.eventDispatcher().publish(DSC_NAMESPACE::Trash::EventType::kEmptyTrash, windowId);
}

DSB_FM_NAMESPACE::WindowsService *TrashHelper::winServIns()
{
    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(DSB_FM_NAMESPACE::WindowsService::name()))
            abort();
    });

    return ctx.service<DSB_FM_NAMESPACE::WindowsService>(DSB_FM_NAMESPACE::WindowsService::name());
}

DSB_FM_NAMESPACE::TitleBarService *TrashHelper::titleServIns()
{
    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(DSB_FM_NAMESPACE::TitleBarService::name()))
            abort();
    });

    return ctx.service<DSB_FM_NAMESPACE::TitleBarService>(DSB_FM_NAMESPACE::TitleBarService::name());
}

SideBarService *TrashHelper::sideBarServIns()
{
    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(DSB_FM_NAMESPACE::SideBarService::name()))
            abort();
    });

    return ctx.service<DSB_FM_NAMESPACE::SideBarService>(DSB_FM_NAMESPACE::SideBarService::name());
}

DSB_FM_NAMESPACE::WorkspaceService *TrashHelper::workspaceServIns()
{
    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(DSB_FM_NAMESPACE::WorkspaceService::name()))
            abort();
    });

    return ctx.service<DSB_FM_NAMESPACE::WorkspaceService>(DSB_FM_NAMESPACE::WorkspaceService::name());
}

dfm_service_common::FileOperationsService *TrashHelper::fileOperationsServIns()
{
    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(DSC_NAMESPACE::FileOperationsService::name()))
            abort();
    });

    return ctx.service<DSC_NAMESPACE::FileOperationsService>(DSC_NAMESPACE::FileOperationsService::name());
}

TrashHelper::TrashHelper(QObject *parent)
    : QObject(parent)
{
}
