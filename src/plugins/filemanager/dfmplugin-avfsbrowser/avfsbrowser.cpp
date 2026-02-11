// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "avfsbrowser.h"
#include "files/avfsfileinfo.h"
#include "files/avfsfilewatcher.h"
#include "files/avfsfileiterator.h"
#include "utils/avfsutils.h"
#include "menu/avfsmenuscene.h"
#include "events/avfseventhandler.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>

#include <QDebug>

Q_DECLARE_METATYPE(QList<QUrl> *)
Q_DECLARE_METATYPE(QList<QVariantMap> *);

namespace dfmplugin_avfsbrowser {
DFM_LOG_REGISTER_CATEGORY(DPAVFSBROWSER_NAMESPACE)

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
    fmDebug() << "avfs mounted? " << AvfsUtils::isAvfsMounted() << ", archive preview enabled? " << AvfsUtils::archivePreviewEnabled();
    if (AvfsUtils::archivePreviewEnabled())
        AvfsUtils::mountAvfs();
    connect(Application::instance(), &Application::previewCompressFileChanged,
            this, [](bool enable) { enable ? AvfsUtils::mountAvfs() : AvfsUtils::unmountAvfs(); }, Qt::DirectConnection);

    dfmplugin_menu_util::menuSceneRegisterScene(AvfsMenuSceneCreator::name(), new AvfsMenuSceneCreator());

    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterFileView", AvfsUtils::scheme());
    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterMenuScene", AvfsUtils::scheme(), AvfsMenuSceneCreator::name());

    regCrumb();
    beMySubScene("SortAndDisplayMenu");   //  yours last second but it's mine now
    beMySubScene("OpenWithMenu");   //  yours last second but it's mine now

    return true;
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
}   // namespace dfmplugin_avfsbrowser
