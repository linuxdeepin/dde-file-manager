// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "smbbrowser.h"
#include "utils/smbbrowserutils.h"
#include "events/smbbrowsereventcaller.h"
#include "events/smbbrowsereventreceiver.h"
#include "events/traversprehandler.h"
#include "fileinfo/smbsharefileinfo.h"
#include "iterator/smbshareiterator.h"
#include "menu/smbbrowsermenuscene.h"
#include "displaycontrol/protocoldevicedisplaymanager.h"

#include <plugins/common/dfmplugin-menu/menu_eventinterface_helper.h>

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>

#include <QMenu>

Q_DECLARE_METATYPE(QString *)
Q_DECLARE_METATYPE(QUrl *)

namespace dfmplugin_smbbrowser {
DFM_LOG_REGISTER_CATEGORY(DPSMBBROWSER_NAMESPACE)

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE

void registScheme(const QString &scheme)
{
    UrlRoute::regScheme(scheme, "/", smb_browser_utils::icon(), true);
    InfoFactory::regClass<SmbShareFileInfo>(scheme);
    DirIteratorFactory::regClass<SmbShareIterator>(scheme);
    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterFileView", scheme);
}

void registSchemeHandler(const QString &scheme, PrehandlerFunc handler)
{
    auto ok = dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_RegisterRoutePrehandle", scheme, handler).toBool();
    fmInfo() << scheme << "'s handler regist result:" << ok;
}

void SmbBrowser::initialize()
{
    dfmplugin_menu_util::menuSceneRegisterScene(SmbBrowserMenuCreator::name(), new SmbBrowserMenuCreator());
    smb_browser_utils::initSettingPane();
    smb_browser_utils::bindSetting();
    bindWindows();
}

bool SmbBrowser::start()
{
    registScheme(Global::Scheme::kSmb);
    registScheme(Global::Scheme::kFtp);
    registScheme(Global::Scheme::kSFtp);
    registScheme(Global::Scheme::kNetwork);
    registScheme(Global::Scheme::kDav);
    registScheme(Global::Scheme::kDavs);
    registScheme(Global::Scheme::kNfs);

    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterMenuScene", QString(Global::Scheme::kSmb), SmbBrowserMenuCreator::name());
    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterMenuScene", QString(Global::Scheme::kNetwork), SmbBrowserMenuCreator::name());

    QVariantMap property;
    property[ViewCustomKeys::kSupportTreeMode] = false;
    dpfSlotChannel->push("dfmplugin_workspace", "slot_View_SetCustomViewProperty", QString(Global::Scheme::kNetwork), property);
    dpfSlotChannel->push("dfmplugin_workspace", "slot_View_SetCustomViewProperty", QString(Global::Scheme::kSmb), property);

    ProtocolDeviceDisplayManager::instance();
    registerNetworkAccessPrehandler();
    followEvents();

    return true;
}

void SmbBrowser::contextMenuHandle(quint64 windowId, const QUrl &url, const QPoint &globalPos)
{
    QFileInfo info(url.path());
    bool bEnabled = info.exists();

    QMenu *menu = new QMenu;
    if (url.scheme() == Global::Scheme::kNetwork) {
        auto newWindowAct = menu->addAction(QObject::tr("Open in new window"), [url]() { SmbBrowserEventCaller::sendOpenWindow(url); });
        newWindowAct->setEnabled(bEnabled);

        auto newTabAct = menu->addAction(QObject::tr("Open in new tab"), [windowId, url]() { SmbBrowserEventCaller::sendOpenTab(windowId, url); });
        newTabAct->setEnabled(bEnabled && SmbBrowserEventCaller::sendCheckTabAddable(windowId));
    }

    QAction *act = menu->exec(globalPos);
    if (act) {
        QList<QUrl> urls { url };
        dpfSignalDispatcher->publish("dfmplugin_smbbrowser", "signal_ReportLog_MenuData", act->text(), urls);
    }
    delete menu;
}

void SmbBrowser::onWindowOpened(quint64 winId)
{
    auto window = FMWindowsIns.findWindowById(winId);
    if (!window)
        return;

    ContextMenuCallback ctxMenuHandle = { SmbBrowser::contextMenuHandle };
    if (window->sideBar()) {
        updateNeighborToSidebar();
    } else {
        connect(
                window, &FileManagerWindow::sideBarInstallFinished,
                this, [this] { updateNeighborToSidebar(); },
                Qt::DirectConnection);
    }

    if (window->titleBar())
        registerNetworkToTitleBar();
    else
        connect(window, &FileManagerWindow::titleBarInstallFinished, this, &SmbBrowser::registerNetworkToTitleBar, Qt::DirectConnection);

    auto searchPlugin { DPF_NAMESPACE::LifeCycle::pluginMetaObj("dfmplugin-search") };
    if (searchPlugin && searchPlugin->pluginState() == DPF_NAMESPACE::PluginMetaObject::kStarted) {
        registerNetworkToSearch();
    } else {
        connect(
                DPF_NAMESPACE::Listener::instance(), &DPF_NAMESPACE::Listener::pluginStarted,
                this, [this](const QString &, const QString &name) {
                    if (name == "dfmplugin-search") registerNetworkToSearch();
                },
                Qt::DirectConnection);
    }
}

void SmbBrowser::bindWindows()
{
    const auto &winIdList { FMWindowsIns.windowIdList() };
    std::for_each(winIdList.begin(), winIdList.end(), [this](quint64 id) {
        onWindowOpened(id);
    });

    connect(&FMWindowsIns, &FileManagerWindowsManager::windowOpened, this, &SmbBrowser::onWindowOpened, Qt::DirectConnection);
}

void SmbBrowser::followEvents()
{
    dpfHookSequence->follow("dfmplugin_detailspace", "hook_Icon_Fetch", SmbBrowserEventReceiver::instance(), &SmbBrowserEventReceiver::detailViewIcon);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_ShortCut_DeleteFiles", SmbBrowserEventReceiver::instance(), &SmbBrowserEventReceiver::cancelDelete);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_ShortCut_MoveToTrash", SmbBrowserEventReceiver::instance(), &SmbBrowserEventReceiver::cancelMoveToTrash);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_ShortCut_PasteFiles", SmbBrowserEventReceiver::instance(), &SmbBrowserEventReceiver::cancelMoveToTrash);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_ShortCut_CopyFiles", SmbBrowserEventReceiver::instance(), &SmbBrowserEventReceiver::cancelMoveToTrash);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_ShortCut_CutFiles", SmbBrowserEventReceiver::instance(), &SmbBrowserEventReceiver::cancelMoveToTrash);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_ShortCut_PreViewFiles", SmbBrowserEventReceiver::instance(), &SmbBrowserEventReceiver::cancelMoveToTrash);
    dpfHookSequence->follow("dfmplugin_titlebar", "hook_Tab_SetTabName", SmbBrowserEventReceiver::instance(), &SmbBrowserEventReceiver::hookSetTabName);
    dpfHookSequence->follow("dfmplugin_titlebar", "hook_Show_Addr", SmbBrowserEventReceiver::instance(), &SmbBrowserEventReceiver::hookTitleBarAddrHandle);
    dpfHookSequence->follow("dfmplugin_titlebar", "hook_Copy_Addr", SmbBrowserEventReceiver::instance(), &SmbBrowserEventReceiver::hookTitleBarAddrHandle);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_Allow_Repeat_Url", SmbBrowserEventReceiver::instance(), &SmbBrowserEventReceiver::hookAllowRepeatUrl);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_ShortCut_CopyFilePath", SmbBrowserEventReceiver::instance(), &SmbBrowserEventReceiver::hookCopyFilePath);
}

void SmbBrowser::updateNeighborToSidebar()
{
    static std::once_flag flag;
    std::call_once(flag, []() {
        ContextMenuCallback contextMenuCb { SmbBrowser::contextMenuHandle };
        Qt::ItemFlags flags { Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren };
        QVariantMap map {
            { "Property_Key_QtItemFlags", QVariant::fromValue(flags) },
            { "Property_Key_CallbackContextMenu", QVariant::fromValue(contextMenuCb) }
        };

        dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Update", smb_browser_utils::netNeighborRootUrl(), map);
    });
}

void SmbBrowser::registerNetworkAccessPrehandler()
{
    PrehandlerFunc handler { travers_prehandler::networkAccessPrehandler };
    PrehandlerFunc smbHandler { travers_prehandler::smbAccessPrehandler };
    registSchemeHandler(Global::Scheme::kSmb, smbHandler);
    registSchemeHandler(Global::Scheme::kFtp, handler);
    registSchemeHandler(Global::Scheme::kSFtp, handler);
    registSchemeHandler(Global::Scheme::kDav, handler);
    registSchemeHandler(Global::Scheme::kDavs, handler);
    registSchemeHandler(Global::Scheme::kNfs, handler);
}

void SmbBrowser::registerNetworkToSearch()
{
    QVariantMap property;
    property["Property_Key_DisableSearch"] = true;
    dpfSlotChannel->push("dfmplugin_search", "slot_Custom_Register", QString(Global::Scheme::kSmb), property);
    dpfSlotChannel->push("dfmplugin_search", "slot_Custom_Register", QString(Global::Scheme::kNetwork), property);
}

void SmbBrowser::registerNetworkToTitleBar()
{
    QVariantMap property;
    property[ViewCustomKeys::kSupportTreeMode] = false;
    dpfSlotChannel->push("dfmplugin_titlebar", "slot_Custom_Register", QString(Global::Scheme::kSmb), property);
    dpfSlotChannel->push("dfmplugin_titlebar", "slot_Custom_Register", QString(Global::Scheme::kNetwork), property);
}
}   // namespace dfmplugin_smbbrowser
