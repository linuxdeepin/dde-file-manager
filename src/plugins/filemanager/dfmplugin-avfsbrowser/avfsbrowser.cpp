/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#include "avfsbrowser.h"
#include "files/avfsfileinfo.h"
#include "files/avfsfilewatcher.h"
#include "files/avfsfileiterator.h"
#include "utils/avfsutils.h"
#include "menu/avfsmenuscene.h"
#include "events/avfseventhandler.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include "services/filemanager/workspace/workspaceservice.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/base/application/settings.h"

#include <QDebug>

Q_DECLARE_METATYPE(QList<QUrl> *)

using namespace dfmplugin_avfsbrowser;
Q_DECLARE_METATYPE(QList<QVariantMap> *);

DFMBASE_USE_NAMESPACE

void AvfsBrowser::initialize()
{
    UrlRoute::regScheme(AvfsUtils::scheme(), "/", {}, true);

    InfoFactory::regClass<AvfsFileInfo>(AvfsUtils::scheme());
    WatcherFactory::regClass<AvfsFileWatcher>(AvfsUtils::scheme());
    DirIteratorFactory::regClass<AvfsFileIterator>(AvfsUtils::scheme());

    followEvents();
}

bool AvfsBrowser::start()
{
    qDebug() << "avfs mounted? " << AvfsUtils::isAvfsMounted() << ", archive preview enabled? " << AvfsUtils::archivePreviewEnabled();
    if (AvfsUtils::archivePreviewEnabled())
        AvfsUtils::mountAvfs();
    connect(Application::instance(), &Application::previewCompressFileChanged,
            this, [](bool enable) { enable ? AvfsUtils::mountAvfs() : AvfsUtils::unmountAvfs(); }, Qt::DirectConnection);

    dfmplugin_menu_util::menuSceneRegisterScene(AvfsMenuSceneCreator::name(), new AvfsMenuSceneCreator());

    DSB_FM_USE_NAMESPACE
    WorkspaceService::service()->addScheme(AvfsUtils::scheme());
    WorkspaceService::service()->setWorkspaceMenuScene(AvfsUtils::scheme(), AvfsMenuSceneCreator::name());

    // follow event
    dpfHookSequence->follow("dfmplugin_utils", "hook_UrlsTransform", AvfsUtils::instance(), &AvfsUtils::urlsToLocal);

    regCrumb();
    claimSubScene("SortAndDisplayMenu");   //  yours last second but it's mine now
    claimSubScene("OpenWithMenu");   //  yours last second but it's mine now

    return true;
}

dpf::Plugin::ShutdownFlag AvfsBrowser::stop()
{
    return kSync;
}

void AvfsBrowser::followEvents()
{
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_OpenLocalFiles", AvfsEventHandler::instance(), &AvfsEventHandler::hookOpenFiles);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_ShortCut_EnterPressed", AvfsEventHandler::instance(), &AvfsEventHandler::hookEnterPressed);
    dpfHookSequence->follow("dfmplugin_titlebar", "hook_Crumb_Seprate", AvfsEventHandler::instance(), &AvfsEventHandler::sepateTitlebarCrumb);
}

void AvfsBrowser::regCrumb()
{
    DSB_FM_USE_NAMESPACE

    dpfSlotChannel->push("dfmplugin_titlebar", "slot_Custom_Register", AvfsUtils::scheme(), QVariantMap {});
}

void AvfsBrowser::claimSubScene(const QString &subScene)
{
    if (dfmplugin_menu_util::menuSceneContains(subScene)) {
        dfmplugin_menu_util::menuSceneBind(subScene, AvfsMenuSceneCreator::name());
    } else {
        //todo(xst) menu
        //        connect(MenuService::service(), &MenuService::sceneAdded, this, [=](const QString &addedScene) {
        //            if (subScene == addedScene) {
        //                MenuService::service()->bind(subScene, AvfsMenuSceneCreator::name());
        //                MenuService::service()->disconnect(this);
        //            }
        //        },
        //                Qt::DirectConnection);
    }
}
