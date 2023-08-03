// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tag.h"
#include "files/tagfileinfo.h"
#include "files/tagfilewatcher.h"
#include "files/tagdiriterator.h"
#include "utils/taghelper.h"
#include "utils/tagfilehelper.h"
#include "utils/tagmanager.h"
#include "utils/filetagcache.h"
#include "widgets/tagwidget.h"
#include "menu/tagmenuscene.h"
#include "menu/tagdirmenuscene.h"
#include "events/tageventreceiver.h"
#include "data/tagproxyhandle.h"

#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <QRectF>
#include <QDBusConnection>
#include <QCoreApplication>

using CustomViewExtensionView = std::function<QWidget *(const QUrl &url)>;

Q_DECLARE_METATYPE(QRectF *)
Q_DECLARE_METATYPE(QList<QVariantMap> *)
Q_DECLARE_METATYPE(QList<QUrl> *)
Q_DECLARE_METATYPE(CustomViewExtensionView)
Q_DECLARE_METATYPE(QByteArray *)
Q_DECLARE_METATYPE(Qt::DropAction *)
Q_DECLARE_METATYPE(dfmbase::ElideTextLayout *)

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_tag;

void Tag::initialize()
{
    UrlRoute::regScheme(TagManager::scheme(), "/", {}, true, tr("Tag"));

    InfoFactory::regClass<TagFileInfo>(TagManager::scheme());
    WatcherFactory::regClass<TagFileWatcher>(TagManager::scheme());
    DirIteratorFactory::regClass<TagDirIterator>(TagManager::scheme());

    if (DPF_NAMESPACE::LifeCycle::isAllPluginsStarted())
        onAllPluginsStarted();
    else
        connect(dpfListener, &DPF_NAMESPACE::Listener::pluginsStarted, this, &Tag::onAllPluginsStarted, Qt::DirectConnection);

    connect(TagProxyHandleIns, &TagProxyHandle::tagServiceRegistered, [] {
        emit FileTagCacheController::instance().initLoadTagInfos();
    });

    if (!TagProxyHandleIns->connectToService()) {
        qWarning() << "Cannot connect to TagManagerDBus!";
    }

    bindEvents();
    followEvents();
    bindWindows();
}

bool Tag::start()
{
    emit FileTagCacheController::instance().initLoadTagInfos();

    CustomViewExtensionView func { Tag::createTagWidget };
    dpfSlotChannel->push("dfmplugin_detailspace", "slot_ViewExtension_Register", func, -1);
    dpfSlotChannel->push("dfmplugin_propertydialog", "slot_ViewExtension_Register", func, "Tag", 0);

    QStringList &&filtes { "kFileSizeField", "kFileChangeTimeField", "kFileInterviewTimeField" };
    dpfSlotChannel->push("dfmplugin_detailspace", "slot_BasicFiledFilter_Add",
                         TagManager::scheme(), filtes);

    return true;
}

void Tag::onWindowOpened(quint64 windId)
{
    auto window = FMWindowsIns.findWindowById(windId);
    Q_ASSERT_X(window, "Tag", "Cannot find window by id");

    if (window->titleBar())
        regTagCrumbToTitleBar();
    else
        connect(window, &FileManagerWindow::titleBarInstallFinished, this, &Tag::regTagCrumbToTitleBar, Qt::DirectConnection);

    if (window->sideBar())
        installToSideBar();
    else
        connect(window, &FileManagerWindow::sideBarInstallFinished, this, &Tag::installToSideBar, Qt::DirectConnection);
}

void Tag::regTagCrumbToTitleBar()
{
    dpfSlotChannel->push("dfmplugin_titlebar", "slot_Custom_Register", TagManager::scheme(), QVariantMap {});
}

void Tag::onAllPluginsStarted()
{
    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterFileView", TagManager::scheme());

    dfmplugin_menu_util::menuSceneRegisterScene(TagMenuCreator::name(), new TagMenuCreator);
    bindScene("FileOperatorMenu");

    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterMenuScene", TagManager::scheme(), TagDirMenuCreator::name());
    dfmplugin_menu_util::menuSceneRegisterScene(TagDirMenuCreator::name(), new TagDirMenuCreator);
}

QWidget *Tag::createTagWidget(const QUrl &url)
{
    if (!TagManager::instance()->canTagFile(url))
        return nullptr;

    return new TagWidget(url);
}

void Tag::installToSideBar()
{
    auto tagsMap = TagManager::instance()->getAllTags();
    auto tagNames = tagsMap.keys();
    auto orders = Application::genericSetting()->value(kSidebarOrder, kTagOrderKey).toStringList();
    for (const auto &item : orders) {
        QUrl u(item);
#if (QT_VERSION <= QT_VERSION_CHECK(5, 15, 0))
        auto query = u.query().split("=", QString::SkipEmptyParts);
#else
        auto query = u.query().split("=", Qt::SkipEmptyParts);
#endif
        if (query.count() == 2 && tagNames.contains(query[1])) {
            auto &&map { TagHelper::instance()->createSidebarItemInfo(query[1]) };
            dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Add", u, map);
            tagNames.removeAll(query[1]);
        }
    }
    for (const auto &tag : tagNames) {   // if tag order is not complete.
        auto &&url { TagHelper::instance()->makeTagUrlByTagName(tag) };
        auto &&map { TagHelper::instance()->createSidebarItemInfo(tag) };
        dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Add", url, map);
    }
}

void Tag::followEvents()
{
    // todo 优化接口
    dpfHookSequence->follow("dfmplugin_workspace", "hook_Delegate_PaintListItem", TagManager::instance(), &TagManager::paintListTagsHandle);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_Delegate_LayoutText", TagManager::instance(), &TagManager::addIconTagsHandle);

    // canvas
    dpfHookSequence->follow("ddplugin_canvas", "hook_CanvasItemDelegate_LayoutText", TagManager::instance(), &TagManager::addIconTagsHandle);

    // paste
    dpfHookSequence->follow("dfmplugin_workspace", "hook_ShortCut_PasteFiles", TagManager::instance(), &TagManager::pasteHandle);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_DragDrop_FileDrop", TagManager::instance(), &TagManager::fileDropHandle);

    // titlebar crumb
    dpfHookSequence->follow("dfmplugin_titlebar", "hook_Crumb_Seprate", TagManager::instance(), &TagManager::sepateTitlebarCrumb);

    dpfHookSequence->follow("dfmplugin_sidebar", "hook_Item_DropData", TagManager::instance(), &TagManager::fileDropHandleWithAction);

    // file operation
    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_OpenFileInPlugin", TagFileHelper::instance(), &TagFileHelper::openFileInPlugin);
}

void Tag::bindScene(const QString &parentScene)
{
    if (dfmplugin_menu_util::menuSceneContains(parentScene)) {
        dfmplugin_menu_util::menuSceneBind(TagMenuCreator::name(), parentScene);
    } else {
        menuScenes << parentScene;
        if (!subscribedEvent)
            subscribedEvent = dpfSignalDispatcher->subscribe("dfmplugin_menu", "signal_MenuScene_SceneAdded", this, &Tag::onMenuSceneAdded);
    }
}

void Tag::onMenuSceneAdded(const QString &scene)
{
    if (menuScenes.contains(scene)) {
        menuScenes.remove(scene);
        dfmplugin_menu_util::menuSceneBind(TagMenuCreator::name(), scene);

        if (menuScenes.isEmpty()) {
            dpfSignalDispatcher->unsubscribe("dfmplugin_menu", "signal_MenuScene_SceneAdded", this, &Tag::onMenuSceneAdded);
            subscribedEvent = false;
        }
    }
}

void Tag::bindEvents()
{
    dpfSignalDispatcher->subscribe(GlobalEventType::kHideFilesResult, TagEventReceiver::instance(), &TagEventReceiver::handleHideFilesResult);
    dpfSignalDispatcher->subscribe(GlobalEventType::kCutFileResult, TagEventReceiver::instance(), &TagEventReceiver::handleFileCutResult);
    dpfSignalDispatcher->subscribe(GlobalEventType::kMoveToTrashResult, TagEventReceiver::instance(), &TagEventReceiver::handleFileRemoveResult);
    dpfSignalDispatcher->subscribe(GlobalEventType::kDeleteFilesResult, TagEventReceiver::instance(), &TagEventReceiver::handleFileRemoveResult);
    dpfSignalDispatcher->subscribe(GlobalEventType::kRenameFileResult, TagEventReceiver::instance(), &TagEventReceiver::handleFileRenameResult);
    dpfSignalDispatcher->subscribe(GlobalEventType::kRestoreFromTrashResult, TagEventReceiver::instance(), &TagEventReceiver::handleRestoreFromTrashResult);
    dpfSignalDispatcher->subscribe(GlobalEventType::kChangeCurrentUrl, TagEventReceiver::instance(), &TagEventReceiver::handleWindowUrlChanged);
    dpfSignalDispatcher->subscribe("dfmplugin_sidebar", "signal_Sidebar_Sorted", TagEventReceiver::instance(), &TagEventReceiver::handleSidebarOrderChanged);

    dpfSlotChannel->connect("dfmplugin_tag", "slot_GetTags", TagEventReceiver::instance(), &TagEventReceiver::handleGetTags);
}

void Tag::bindWindows()
{
    const auto &winIdList { FMWindowsIns.windowIdList() };
    std::for_each(winIdList.begin(), winIdList.end(), [this](quint64 id) {
        onWindowOpened(id);
    });
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowOpened, this, &Tag::onWindowOpened, Qt::DirectConnection);
}
