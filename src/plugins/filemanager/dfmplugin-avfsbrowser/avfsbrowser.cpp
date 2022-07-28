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

#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"

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

    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterFileView", AvfsUtils::scheme());
    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterMenuScene", AvfsUtils::scheme(), AvfsMenuSceneCreator::name());

    // follow event
    dpfHookSequence->follow("dfmplugin_utils", "hook_UrlsTransform", AvfsUtils::instance(), &AvfsUtils::urlsToLocal);

    regCrumb();
    beMySubScene("SortAndDisplayMenu");   //  yours last second but it's mine now
    beMySubScene("OpenWithMenu");   //  yours last second but it's mine now

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
    dpfSlotChannel->push("dfmplugin_titlebar", "slot_Custom_Register", AvfsUtils::scheme(), QVariantMap {});
}

void AvfsBrowser::beMySubScene(const QString &subScene)
{
    if (dfmplugin_menu_util::menuSceneContains(subScene)) {
        dfmplugin_menu_util::menuSceneBind(subScene, AvfsMenuSceneCreator::name());
    } else {
        waitToBind << subScene;
        if (!eventSubscribed)
            eventSubscribed = dpfSignalDispatcher->subscribe("dfmplugin_menu", "signal_MenuScene_SceneAdded", this, &AvfsBrowser::beMySubOnAdded);
    }
}

void AvfsBrowser::beMySubOnAdded(const QString &newScene)
{
    if (waitToBind.contains(newScene)) {
        waitToBind.remove(newScene);
        if (waitToBind.isEmpty())
            eventSubscribed = !dpfSignalDispatcher->unsubscribe("dfmplugin_menu", "signal_MenuScene_SceneAdded", this, &AvfsBrowser::beMySubOnAdded);
        beMySubScene(newScene);
    }
}
