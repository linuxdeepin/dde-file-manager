// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
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

#include <plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h>

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>

#include <QMenu>

Q_DECLARE_METATYPE(QString *)
Q_DECLARE_METATYPE(QUrl *)

namespace dfmplugin_smbbrowser {
DFM_LOG_REISGER_CATEGORY(DPSMBBROWSER_NAMESPACE)

DFMBASE_USE_NAMESPACE

void SmbBrowser::initialize()
{
    UrlRoute::regScheme(Global::Scheme::kSmb, "/", smb_browser_utils::icon(), true);
    UrlRoute::regScheme(Global::Scheme::kFtp, "/", smb_browser_utils::icon(), true);
    UrlRoute::regScheme(Global::Scheme::kSFtp, "/", smb_browser_utils::icon(), true);
    UrlRoute::regScheme(smb_browser_utils::networkScheme(), "/", smb_browser_utils::icon(), true);

    InfoFactory::regClass<SmbShareFileInfo>(Global::Scheme::kSmb);
    DirIteratorFactory::regClass<SmbShareIterator>(Global::Scheme::kSmb);

    InfoFactory::regClass<SmbShareFileInfo>(Global::Scheme::kFtp);
    DirIteratorFactory::regClass<SmbShareIterator>(Global::Scheme::kFtp);

    InfoFactory::regClass<SmbShareFileInfo>(Global::Scheme::kSFtp);
    DirIteratorFactory::regClass<SmbShareIterator>(Global::Scheme::kSFtp);

    InfoFactory::regClass<SmbShareFileInfo>(smb_browser_utils::networkScheme());
    DirIteratorFactory::regClass<SmbShareIterator>(smb_browser_utils::networkScheme());

    dfmplugin_menu_util::menuSceneRegisterScene(SmbBrowserMenuCreator::name(), new SmbBrowserMenuCreator());
    bindWindows();

    smb_browser_utils::initSettingPane();
    smb_browser_utils::bindSetting();

    followEvents();
}

bool SmbBrowser::start()
{
    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterFileView", QString(Global::Scheme::kSmb));
    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterMenuScene", QString(Global::Scheme::kSmb), SmbBrowserMenuCreator::name());

    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterFileView", smb_browser_utils::networkScheme());
    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterMenuScene", smb_browser_utils::networkScheme(), SmbBrowserMenuCreator::name());

    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterFileView", QString(Global::Scheme::kFtp));
    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterFileView", QString(Global::Scheme::kSFtp));

    dpfSlotChannel->push("dfmplugin_workspace", "slot_NotSupportTreeView", smb_browser_utils::networkScheme());
    dpfSlotChannel->push("dfmplugin_workspace", "slot_NotSupportTreeView", QString(Global::Scheme::kSmb));

    ProtocolDeviceDisplayManager::instance();
    registerNetworkAccessPrehandler();

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
    PrehandlerFunc smbHanlder { travers_prehandler::smbAccessPrehandler };
    if (!dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_RegisterRoutePrehandle", QString(Global::Scheme::kSmb), smbHanlder).toBool())
        fmWarning() << "smb's prehandler has been registered";
    if (!dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_RegisterRoutePrehandle", QString(Global::Scheme::kSFtp), handler).toBool())
        fmWarning() << "sftp's prehandler has been registered";
    if (!dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_RegisterRoutePrehandle", QString(Global::Scheme::kFtp), handler).toBool())
        fmWarning() << "ftp's prehandler has been registered";
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
    property["Property_Key_HideTreeViewBtn"] = true;
    dpfSlotChannel->push("dfmplugin_titlebar", "slot_Custom_Register", QString(Global::Scheme::kSmb), property);
    dpfSlotChannel->push("dfmplugin_titlebar", "slot_Custom_Register", QString(Global::Scheme::kNetwork), property);
}
}   // namespace dfmplugin_smbbrowser
