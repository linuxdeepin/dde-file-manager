/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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

#include "smbbrowser.h"
#include "events/smbbrowsereventcaller.h"
#include "events/smbbrowsereventreceiver.h"
#include "utils/smbbrowserutils.h"
#include "fileinfo/smbsharefileinfo.h"
#include "iterator/smbshareiterator.h"
#include "menu/smbbrowsermenuscene.h"
#include "menu/smbintcomputermenuscene.h"
#include "menu/smbintcomputermenuscene_p.h"
#include "smbintegration/smbintegrationentity.h"
#include "smbintegration/smbintegrationmanager.h"

#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/base/application/settings.h"
#include "dfm-base/base/device/devicemanager.h"
#include "dfm-base/base/device/deviceutils.h"
#include "dfm-base/utils/dialogmanager.h"

#include <QMenu>
#include <QTimer>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QHostAddress>
#include <QNetworkInterface>

using namespace dfmplugin_smbbrowser;

DFMBASE_USE_NAMESPACE

static constexpr char kSmbInteg[] = { "smbinteg" };
static constexpr char kSmbIntegPath[] = { "/.smbinteg" };
static constexpr char kProtodevstashed[] = { "protodevstashed" };
static constexpr char kStashedSmbDevices[] = { "StashedSmbDevices" };
static constexpr char kSmbIntegrations[] = { "SmbIntegrations" };

Q_DECLARE_METATYPE(QString *)

void SmbBrowser::initialize()
{
    UrlRoute::regScheme(Global::Scheme::kSmb, "/", SmbBrowserUtils::icon(), true);
    UrlRoute::regScheme(Global::Scheme::kFtp, "/", SmbBrowserUtils::icon(), true);
    UrlRoute::regScheme(Global::Scheme::kSFtp, "/", SmbBrowserUtils::icon(), true);
    UrlRoute::regScheme(SmbBrowserUtils::networkScheme(), "/", SmbBrowserUtils::icon(), true);

    InfoFactory::regClass<SmbShareFileInfo>(Global::Scheme::kSmb);
    DirIteratorFactory::regClass<SmbShareIterator>(Global::Scheme::kSmb);

    InfoFactory::regClass<SmbShareFileInfo>(Global::Scheme::kFtp);
    DirIteratorFactory::regClass<SmbShareIterator>(Global::Scheme::kFtp);

    InfoFactory::regClass<SmbShareFileInfo>(Global::Scheme::kSFtp);
    DirIteratorFactory::regClass<SmbShareIterator>(Global::Scheme::kSFtp);

    InfoFactory::regClass<SmbShareFileInfo>(SmbBrowserUtils::networkScheme());
    DirIteratorFactory::regClass<SmbShareIterator>(SmbBrowserUtils::networkScheme());

    EntryEntityFactor::registCreator<SmbIntegrationEntity>(kSmbInteg);

    connect(SmbIntegrationManager::instance(), &SmbIntegrationManager::refreshToSmbIntegrationMode, this, &SmbBrowser::onWindowOpened);
    connect(SmbIntegrationManager::instance(), &SmbIntegrationManager::refreshToSmbSeperatedMode, this, &SmbBrowser::onRefreshToSmbSeperatedMode);
    dfmplugin_menu_util::menuSceneRegisterScene(SmbBrowserMenuCreator::name(), new SmbBrowserMenuCreator());
    bindWindows();
    registerNetworkAccessPrehandler();

    dfmplugin_menu_util::menuSceneRegisterScene(SmbIntComputerMenuCreator::name(), new SmbIntComputerMenuCreator());
    bindScene("ComputerMenu");
    SmbIntegrationManager::instance();

    followEvents();
    dpfSlotChannel->push("dfmplugin_computer", "slot_ComputerView_Refresh");
    if (SmbIntegrationManager::instance()->isSmbIntegrationEnabled()) {
        QTimer::singleShot(0, this, [=]() {
            QStringList smbRootUrls = Application::genericSetting()->value(kStashedSmbDevices, kSmbIntegrations).toStringList();
            for (const QString &smbRootUrl : smbRootUrls)
                SmbIntegrationManager::instance()->addIntegrationItemToComputer(smbRootUrl);
        });
    }
}

bool SmbBrowser::start()
{
    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterFileView", QString(Global::Scheme::kSmb));
    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterMenuScene", QString(Global::Scheme::kSmb), SmbBrowserMenuCreator::name());

    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterFileView", SmbBrowserUtils::networkScheme());
    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterMenuScene", SmbBrowserUtils::networkScheme(), SmbBrowserMenuCreator::name());

    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterFileView", QString(Global::Scheme::kFtp));
    dpfSlotChannel->push("dfmplugin_workspace", "slot_RegisterFileView", QString(Global::Scheme::kSFtp));
    dpfSignalDispatcher->subscribe("dfmplugin_computer", "signal_Operation_OpenItem", SmbIntegrationManager::instance(), &SmbIntegrationManager::computerOpenItem);

    return true;
}

void SmbBrowser::contenxtMenuHandle(quint64 windowId, const QUrl &url, const QPoint &globalPos)
{
    QFileInfo info(url.path());
    bool bEnabled = info.exists();

    QMenu *menu = new QMenu;
    if (url.scheme() == Global::Scheme::kNetwork) {
        auto newWindowAct = menu->addAction(QObject::tr("Open in new window"), [url]() { SmbBrowserEventCaller::sendOpenWindow(url); });
        newWindowAct->setEnabled(bEnabled);

        auto newTabAct = menu->addAction(QObject::tr("Open in new tab"), [windowId, url]() {
            SmbBrowserEventCaller::sendOpenTab(windowId, url);
        });
        newTabAct->setEnabled(bEnabled && SmbBrowserEventCaller::sendCheckTabAddable(windowId));
    } else if (SmbIntegrationManager::instance()->isSmbIntegrationEnabled() && url.path() == kSmbIntegPath) {
        auto newWindowAct = menu->addAction(QObject::tr("Unmount"), [windowId, url]() {
            SmbIntegrationManager::instance()->umountAllProtocolDevice(windowId, url, false);
            // format of url is : entry://x.x.x.x/.smbinteg
            QUrl smbUrl;
            smbUrl.setScheme(Global::Scheme::kSmb);
            smbUrl.setHost(url.host());
            SmbIntegrationManager::instance()->removeStashedIntegrationFromConfig(smbUrl);
        });
        newWindowAct->setEnabled(true);

        auto newTabAct = menu->addAction(QObject::tr("Clear saved password and unmount"), [windowId, url]() {
            SmbIntegrationManager::instance()->umountAllProtocolDevice(windowId, url, true);
            QUrl smbUrl;
            smbUrl.setScheme(Global::Scheme::kSmb);
            smbUrl.setHost(url.host());
            SmbIntegrationManager::instance()->removeStashedIntegrationFromConfig(smbUrl);
        });
        newTabAct->setEnabled(true);
    } else if (!SmbIntegrationManager::instance()->isSmbIntegrationEnabled() && url.path().endsWith(kProtodevstashed)) {
        auto newWindowAct1 = menu->addAction(QObject::tr("Mount"), [windowId, url]() {
            QString suffix = QString(".%1").arg(kProtodevstashed);
            QString encodecId = url.path().remove(suffix);
            QString id = QByteArray::fromBase64(encodecId.toUtf8());
            dpfSignalDispatcher->publish(GlobalEventType::kChangeCurrentUrl, windowId, QUrl(id));
        });

        auto newWindowAct2 = menu->addAction(QObject::tr("Remove"), [url]() {
            QString suffix = QString(".%1").arg(kProtodevstashed);
            QString encodecId = url.path().remove(suffix);
            QString id = QByteArray::fromBase64(encodecId.toUtf8());
            SmbIntegrationManager::instance()->removeStashedSeperatedItem(QUrl(id));
        });
        newWindowAct1->setEnabled(true);
        newWindowAct2->setEnabled(true);
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
    SmbIntegrationManager::instance()->setWindowId(winId);

    auto window = FMWindowsIns.findWindowById(winId);
    ContextMenuCallback ctxMenuHandle = { SmbBrowser::contenxtMenuHandle };
    if (window->sideBar()) {
        addNeighborToSidebar();
        SmbIntegrationManager::instance()->addSmbIntegrationFromConfig(ctxMenuHandle);
    } else {
        connect(
                window, &FileManagerWindow::sideBarInstallFinished,
                this, [this, ctxMenuHandle] {
                    addNeighborToSidebar();
                    SmbIntegrationManager::instance()->addSmbIntegrationFromConfig(ctxMenuHandle);
                },
                Qt::DirectConnection);
    }

    if (window->workSpace()) {
        SmbIntegrationManager::instance()->addSmbIntegrationFromConfig(ctxMenuHandle, false);
    } else {
        connect(
                window, &FileManagerWindow::workspaceInstallFinished,
                this, [this, ctxMenuHandle] {
                    addNeighborToSidebar();
                    SmbIntegrationManager::instance()->addSmbIntegrationFromConfig(ctxMenuHandle, false);
                },
                Qt::DirectConnection);
    }

    auto searchPlugin { DPF_NAMESPACE::LifeCycle::pluginMetaObj("dfmplugin-search") };
    if (searchPlugin && searchPlugin->pluginState() == DPF_NAMESPACE::PluginMetaObject::kStarted) {
        registerNetworkToSearch();
    } else {
        connect(
                DPF_NAMESPACE::Listener::instance(), &DPF_NAMESPACE::Listener::pluginStarted, this, [this](const QString &iid, const QString &name) {
                    Q_UNUSED(iid)
                    if (name == "dfmplugin-search")
                        registerNetworkToSearch();
                },
                Qt::DirectConnection);
    }
}

void SmbBrowser::onRefreshToSmbSeperatedMode(const QVariantMap &stashedSeperatedData, const QList<QUrl> &urls)
{
    // add items to sidebar
    ContextMenuCallback ctxMenuHandle = { SmbBrowser::contenxtMenuHandle };
    SmbIntegrationManager::instance()->addStashedSeperatedItemToSidebar(stashedSeperatedData, ctxMenuHandle);

    // add items to computer view
    SmbIntegrationManager::instance()->addStashedSeperatedItemToComputer(urls);
}

void SmbBrowser::bindScene(const QString &parentScene)
{
    if (dfmplugin_menu_util::menuSceneContains(parentScene)) {
        dfmplugin_menu_util::menuSceneBind(SmbIntComputerMenuCreator::name(), parentScene);
    } else {
        waitToBind << parentScene;
        if (!eventSubscribed)
            eventSubscribed = dpfSignalDispatcher->subscribe("dfmplugin_menu", "signal_MenuScene_SceneAdded", this, &SmbBrowser::bindSceneOnAdded);
    }
}

void SmbBrowser::bindSceneOnAdded(const QString &newScene)
{
    if (waitToBind.contains(newScene)) {
        waitToBind.remove(newScene);
        if (waitToBind.isEmpty())
            eventSubscribed = !dpfSignalDispatcher->unsubscribe("dfmplugin_menu", "signal_MenuScene_SceneAdded", this, &SmbBrowser::bindSceneOnAdded);
        bindScene(newScene);
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
    dpfHookSequence->follow("dfmplugin_computer", "hook_ComputerView_ItemListFilter", SmbIntegrationManager::instance(), &SmbIntegrationManager::handleItemListFilter);
    dpfHookSequence->follow("dfmplugin_computer", "hook_ComputerView_ItemFilterOnAdd", SmbIntegrationManager::instance(), &SmbIntegrationManager::handleItemFilterOnAdd);
    dpfHookSequence->follow("dfmplugin_detailspace", "hook_Icon_Fetch", SmbBrowserEventReceiver::instance(), &SmbBrowserEventReceiver::detailViewIcon);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_ShortCut_DeleteFiles", SmbBrowserEventReceiver::instance(), &SmbBrowserEventReceiver::cancelDelete);
    dpfHookSequence->follow("dfmplugin_computer", "hook_ComputerView_ItemFilterOnRemove", SmbIntegrationManager::instance(), &SmbIntegrationManager::handleItemFilterOnRemove);
}

void SmbBrowser::addNeighborToSidebar()
{
    ContextMenuCallback contextMenuCb { SmbBrowser::contenxtMenuHandle };
    Qt::ItemFlags flags { Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren };
    QVariantMap map {
        { "Property_Key_Group", "Group_Network" },
        { "Property_Key_DisplayName", tr("Computers in LAN") },
        { "Property_Key_Icon", SmbBrowserUtils::icon() },
        { "Property_Key_QtItemFlags", QVariant::fromValue(flags) },
        { "Property_Key_VisiableControl", "computers_in_lan" },
        { "Property_Key_ReportName", "Network" },
        { "Property_Key_CallbackContextMenu", QVariant::fromValue(contextMenuCb) }
    };

    dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Insert", 0, SmbBrowserUtils::netNeighborRootUrl(), map);
}

void SmbBrowser::registerNetworkAccessPrehandler()
{
    Prehandler handler { SmbBrowser::networkAccessPrehandler };
    Prehandler smbHanlder { SmbBrowser::smbAccessPrehandler };
    if (!dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_RegisterRoutePrehandle", QString(Global::Scheme::kSmb), smbHanlder).toBool())
        qWarning() << "smb's prehandler has been registered";
    if (!dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_RegisterRoutePrehandle", QString(Global::Scheme::kSFtp), handler).toBool())
        qWarning() << "sftp's prehandler has been registered";
    if (!dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_RegisterRoutePrehandle", QString(Global::Scheme::kFtp), handler).toBool())
        qWarning() << "ftp's prehandler has been registered";
}

void SmbBrowser::registerNetworkToSearch()
{
    QVariantMap property;
    property["Property_Key_DisableSearch"] = true;
    dpfSlotChannel->push("dfmplugin_search", "slot_Custom_Register", QString(Global::Scheme::kSmb), property);
    dpfSlotChannel->push("dfmplugin_search", "slot_Custom_Register", QString(Global::Scheme::kNetwork), property);
}

void SmbBrowser::networkAccessPrehandler(quint64 winId, const QUrl &url, std::function<void()> after)
{
    const auto &&scheme = url.scheme();
    const QStringList kSupportedSchemes { Global::Scheme::kSmb, Global::Scheme::kFtp, Global::Scheme::kSFtp };
    if (!kSupportedSchemes.contains(scheme))
        return;

    auto makeSmbRootUrl = [](const QUrl &url) {
        QUrl smbRootUrl;
        smbRootUrl.setScheme(url.scheme());
        smbRootUrl.setHost(url.host());
        return smbRootUrl;
    };
    auto callBackMnt = [after, winId, url, makeSmbRootUrl](bool ok, DFMMOUNT::DeviceError err, const QString &mpt) {
        if (!mpt.isEmpty()) {
            dpfSignalDispatcher->publish(GlobalEventType::kChangeCurrentUrl, winId, QUrl::fromLocalFile(mpt));

            if (SmbIntegrationManager::instance()->isSmbIntegrationEnabled()) {
                ContextMenuCallback ctxMenuHandle = { SmbBrowser::contenxtMenuHandle };
                SmbIntegrationManager::instance()->addSmbIntegrationItem(makeSmbRootUrl(url), ctxMenuHandle);
            }
        } else if ((ok || err == DFMMOUNT::DeviceError::kGIOErrorAlreadyMounted) && after) {
            after();

            if (SmbIntegrationManager::instance()->isSmbIntegrationEnabled()) {
                ContextMenuCallback ctxMenuHandle = { SmbBrowser::contenxtMenuHandle };
                SmbIntegrationManager::instance()->addSmbIntegrationItem(makeSmbRootUrl(url), ctxMenuHandle);
            }
        } else {
            if (err == DFMMOUNT::DeviceError::kUserErrorNetworkWrongPasswd)   // fix bug:#180869
                err = DFMMOUNT::DeviceError::kUnhandledError;
            DialogManager::instance()->showErrorDialogWhenOperateDeviceFailed(DialogManager::kMount, err);
            qDebug() << DeviceUtils::errMessage(err);
            // dont save failed access to history
            dpfSlotChannel->push("dfmplugin_titlebar", "slot_ServerDialog_RemoveHistory", url.toString());
        }
    };
    DevMngIns->mountNetworkDeviceAsync(url.toString(), callBackMnt /*,10000*/);
}

void SmbBrowser::smbAccessPrehandler(quint64 winId, const QUrl &url, std::function<void()> after)
{
    bool enableRequired = false;
    auto checkAndStartService = [winId, &enableRequired](const QString &serv) {
        if (!SmbBrowserUtils::instance()->isServiceRuning(serv)) {
            if (!SmbBrowserUtils::instance()->startService(serv)) {
                dpfSlotChannel->push("dfmplugin_titlebar", "slot_Navigator_Backward", winId);   // if failed/cancelled, back to previous page.
                return false;
            }
            enableRequired = true;
        }
        return true;
    };

    QStringList hostList;
    auto addrs = QNetworkInterface::allAddresses();
    for (const auto &addr : addrs)
        hostList << addr.toString();
    hostList << "localhost";

    QString targetHost = url.host();
    if (hostList.contains(targetHost)) {   // only check service when access local shares
        QStringList validateService { "smb", "nmb" };
        for (const auto &serv : validateService) {
            if (!checkAndStartService(serv))
                return;
        }
    }

    QTimer::singleShot(100, qApp, [=] {
        networkAccessPrehandler(winId, url, after);
    });

    // set smbd auto start ASYNC
    if (enableRequired) {
        QDBusInterface iface("com.deepin.filemanager.daemon",
                             "/com/deepin/filemanager/daemon/UserShareManager",
                             "com.deepin.filemanager.daemon.UserShareManager",
                             QDBusConnection::systemBus());
        iface.asyncCall("EnableSmbServices");
    }
}

QDebug operator<<(QDebug dbg, const DPSMBBROWSER_NAMESPACE::SmbShareNode &node)
{
    dbg.nospace() << "SmbShareNode: {"
                  << "\n\turl: " << node.url
                  << "\n\tname: " << node.displayName
                  << "\n\ticon: " << node.iconType
                  << "\n}";
    return dbg;
}
