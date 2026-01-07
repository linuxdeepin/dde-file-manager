// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "titlebarhelper.h"
#include "events/titlebareventcaller.h"
#include "dialogs/connecttoserverdialog.h"
#include "dialogs/diskpasswordchangingdialog.h"
#include "views/titlebarwidget.h"
#include "utils/optionbuttonmanager.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/dialogs/smbsharepasswddialog/usersharepasswordsettingdialog.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/utils/finallyutil.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <dfm-framework/dpf.h>

#include <DTitlebar>

#include <QStandardPaths>
#include <QSettings>

using namespace dfmplugin_titlebar;
DFMBASE_USE_NAMESPACE
using namespace GlobalDConfDefines::ConfigPath;
using namespace GlobalDConfDefines::BaseConfig;

QMap<quint64, TitleBarWidget *> TitleBarHelper::kTitleBarMap {};
QList<QString> TitleBarHelper::kKeepTitleStatusSchemeList {};
QMap<QString, ViewModeUrlCallback> TitleBarHelper::kViewModeUrlCallbackMap {};

bool TitleBarHelper::newWindowAndTabEnabled { true };
bool TitleBarHelper::searchEnabled { false };

QList<TitleBarWidget *> TitleBarHelper::titlebars()
{
    return kTitleBarMap.values();
}

TitleBarWidget *TitleBarHelper::findTileBarByWindowId(quint64 windowId)
{
    if (!kTitleBarMap.contains(windowId)) {
        fmWarning() << "Cannot find titlebar widget for window id:" << windowId;
        return nullptr;
    }

    return kTitleBarMap[windowId];
}

void TitleBarHelper::addTileBar(quint64 windowId, TitleBarWidget *titleBar)
{
    QMutexLocker locker(&TitleBarHelper::mutex());
    if (!kTitleBarMap.contains(windowId))
        kTitleBarMap.insert(windowId, titleBar);
}

void TitleBarHelper::removeTitleBar(quint64 windowId)
{
    QMutexLocker locker(&TitleBarHelper::mutex());
    if (kTitleBarMap.contains(windowId))
        kTitleBarMap.remove(windowId);
}

quint64 TitleBarHelper::windowId(QWidget *sender)
{
    return FMWindowsIns.findWindowId(sender);
}

void TitleBarHelper::createSettingsMenu(quint64 id)
{
    auto window = FMWindowsIns.findWindowById(id);
    Q_ASSERT_X(window, "TitleBar", "Cannot find window by id");
    auto titleBarWidget = dynamic_cast<TitleBarWidget *>(window->titleBar());
    if (!titleBarWidget || !titleBarWidget->titleBar()) {
        fmWarning() << "Cannot create settings menu: invalid titlebar widget for window id" << id;
        return;
    }

    if (window->property("WINDOW_DISABLE_TITLEBAR_MENU").toBool()) {
        titleBarWidget->titleBar()->setDisableFlags(Qt::WindowSystemMenuHint);
        titleBarWidget->titleBar()->setMenuVisible(false);
        return;
    }

    QMenu *menu = new QMenu();

    QAction *action { new QAction(QObject::tr("New window"), menu) };
    action->setData(MenuAction::kNewWindow);
    menu->addAction(action);

    action = new QAction(QObject::tr("New tab"), menu);
    action->setData(MenuAction::kOpenInNewTab);
    menu->addAction(action);

    menu->addSeparator();

    action = new QAction(QObject::tr("Connect to Server"), menu);
    action->setData(MenuAction::kConnectToServer);
    menu->addAction(action);

    action = new QAction(QObject::tr("Set share password"), menu);
    action->setData(MenuAction::kSetUserSharePassword);
    menu->addAction(action);

    if (DeviceUtils::checkDiskEncrypted()) {
        action = new QAction(QObject::tr("Change disk password"), menu);
        action->setData(MenuAction::kChangeDiskPassword);
        menu->addAction(action);
    }

    action = new QAction(QObject::tr("Settings"), menu);
    action->setData(MenuAction::kSettings);
    menu->addAction(action);

    QObject::connect(menu, &QMenu::triggered, [id](QAction *act) {
        bool ok { false };
        int val { act->data().toInt(&ok) };
        if (ok)
            handleSettingMenuTriggered(id, val);
    });

    auto defaultMenu = titleBarWidget->titleBar()->menu();
    if (defaultMenu && !defaultMenu->isEmpty()) {
        for (auto *act : defaultMenu->actions()) {
            act->setParent(menu);
            menu->addAction(act);
        }
    }
    titleBarWidget->titleBar()->setMenu(menu);
}

QList<CrumbData> TitleBarHelper::crumbSeprateUrl(const QUrl &url)
{
    static const QString kHomePath { QStandardPaths::standardLocations(QStandardPaths::HomeLocation).constLast() };

    QList<CrumbData> list;
    const QString &path = url.toLocalFile();
    if (path.isEmpty())
        return list;

    QString prefixPath { "/" };

    // for gvfs files do not construct QStorageInfo object which costs a lots time.
    const QString &&iconName = QStringLiteral("drive-harddisk-symbolic");

    // for root user, $HOME is /root and the mount path of gvfsd is /root/.gvfs, so promote it to the prior condition
    QRegularExpression rex(Global::Regex::kGvfsRoot);
    auto match = rex.match(path);
    if (match.hasMatch()) {
        prefixPath = match.captured();
        CrumbData data { QUrl::fromLocalFile(prefixPath), "", iconName };
        list.append(data);
    } else if (path.startsWith(kHomePath)) {
        prefixPath = kHomePath;
        QString iconName { SystemPathUtil::instance()->systemPathIconName("Home") };
        CrumbData data { QUrl::fromLocalFile(kHomePath), getDisplayName("Home"), iconName };
        list.append(data);
    } else {
        prefixPath = DeviceUtils::getLongestMountRootPath(path);
        // TODO(zhangs): device info  (ref DFMFileCrumbController::seprateUrl)

        if (prefixPath == "/") {
            CrumbData data(UrlRoute::rootUrl(Global::Scheme::kFile), getDisplayName("System Disk"), "drive-harddisk-root-symbolic");
            list.append(data);
        } else {
            CrumbData data(QUrl::fromLocalFile(prefixPath), QString(), iconName);
            list.append(data);
        }
    }

    QList<QUrl> urls;
    urls.push_back(url);
    UrlRoute::urlParentList(url, &urls);

    // Push urls into crumb list (without prefix url)
    QList<QUrl>::const_reverse_iterator iter = urls.crbegin();
    while (iter != urls.crend()) {
        const QUrl &oneUrl = *iter;
        if (!prefixPath.startsWith(oneUrl.toLocalFile())) {
            QString displayText = oneUrl.fileName();
            // Check for possible display text.
            auto infoPointer = InfoFactory::create<DFMBASE_NAMESPACE::FileInfo>(oneUrl);
            if (infoPointer) {
                const QString &displayName = infoPointer->displayOf(DisPlayInfoType::kFileDisplayName);
                if (!displayName.isEmpty())
                    displayText = displayName;
            }
            CrumbData data(oneUrl, displayText);
            list.append(data);
        }
        ++iter;
    }

    return list;
}

QList<CrumbData> TitleBarHelper::tansToCrumbDataList(const QList<QVariantMap> &mapGroup)
{
    QList<CrumbData> group;
    for (auto &&map : mapGroup) {
        const auto &url { map[CustomKey::kUrl].toUrl() };
        const auto &text { map[CustomKey::kDisplayText].toString() };
        const auto &icon { map[CustomKey::kIconName].toString() };
        group.push_back(CrumbData { url, text, icon });
    }
    return group;
}

void TitleBarHelper::handleJumpToPressed(QWidget *sender, const QString &text)
{
    const auto &currentDir = QDir::currentPath();
    QUrl currentUrl;
    auto curTitleBar = findTileBarByWindowId(windowId(sender));
    if (curTitleBar)
        currentUrl = curTitleBar->currentUrl();

    if (currentUrl.isLocalFile())
        QDir::setCurrent(currentUrl.toLocalFile());

    QString inputStr = text;
    TitleBarEventCaller::sendCheckAddressInputStr(sender, &inputStr);

    // here, judge whether the text is a local file path.
    QUrl url(UrlRoute::fromUserInput(inputStr, false));
    QDir::setCurrent(currentDir);

    QString scheme { url.scheme() };
    if (!url.scheme().isEmpty() && UrlRoute::hasScheme(scheme)) {
        if (url.path().isEmpty())
            url.setPath("/");
        fmInfo() << "jump :" << inputStr;
        const FileInfoPointer &info = InfoFactory::create<FileInfo>(url);
        if (info && info->exists() && info->isAttributes(OptInfoType::kIsFile)) {
            TitleBarEventCaller::sendOpenFile(sender, url);
        } else {
            TitleBarEventCaller::sendCd(sender, url);
        }
    } else {
        fmWarning() << "jump to :" << inputStr << "is not a valid url";
    }
}

void TitleBarHelper::handleSearch(QWidget *sender, const QString &text)
{
    const auto &currentDir = QDir::currentPath();
    QUrl currentUrl;
    auto curTitleBar = findTileBarByWindowId(windowId(sender));
    if (curTitleBar)
        currentUrl = curTitleBar->currentUrl();

    if (currentUrl.isValid()) {
        bool isDisableSearch = dpfSlotChannel->push("dfmplugin_search", "slot_Custom_IsDisableSearch", currentUrl).toBool();
        if (isDisableSearch) {
            fmInfo() << "Search disabled for current directory:" << currentUrl.toString();
            return;
        }
    }

    fmInfo() << "Starting search with keyword:" << text;
    TitleBarEventCaller::sendSearch(sender, text);
}

void TitleBarHelper::openCurrentUrlInNewTab(quint64 windowId)
{
    FileManagerWindowsManager::FMWindow *window = FMWindowsIns.findWindowById(windowId);
    if (!window) {
        fmWarning() << "Cannot open new tab: window not found for id" << windowId;
        return;
    }

    TitleBarEventCaller::sendOpenTab(windowId, window->currentUrl());
}

void TitleBarHelper::showSettingsDialog(quint64 windowId)
{
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kShowSettingDialog, windowId);
}

void TitleBarHelper::showConnectToServerDialog(quint64 windowId)
{
    auto window = FMWindowsIns.findWindowById(windowId);

    if (!window || window->property("ConnectToServerDialogShown").toBool())
        return;

    ConnectToServerDialog *dialog = new ConnectToServerDialog(window->currentUrl(), window);
    dialog->show();
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    QObject::connect(dialog, &DDialog::buttonClicked, dialog, &ConnectToServerDialog::onButtonClicked);
    window->setProperty("ConnectToServerDialogShown", true);
    QObject::connect(dialog, &ConnectToServerDialog::closed, window, [window] {
        window->setProperty("ConnectToServerDialogShown", false);
    });
}

void TitleBarHelper::showUserSharePasswordSettingDialog(quint64 windowId)
{
    auto window = FMWindowsIns.findWindowById(windowId);
    if (!window || window->property("UserSharePwdSettingDialogShown").toBool()) {
        return;
    }

    UserSharePasswordSettingDialog *dialog = new UserSharePasswordSettingDialog(window);
    dialog->show();
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    QObject::connect(dialog, &UserSharePasswordSettingDialog::buttonClicked, dialog, &UserSharePasswordSettingDialog::onButtonClicked);
    QObject::connect(dialog, &UserSharePasswordSettingDialog::inputPassword, [=](const QString &password) {
        dpfSignalDispatcher->publish("dfmplugin_titlebar", "signal_Share_SetPassword", password);
    });
    window->setProperty("UserSharePwdSettingDialogShown", true);
    QObject::connect(dialog, &UserSharePasswordSettingDialog::closed, window, [window] {
        window->setProperty("UserSharePwdSettingDialogShown", false);
    });
}

void TitleBarHelper::showDiskPasswordChangingDialog(quint64 windowId)
{
    auto window = FMWindowsIns.findWindowById(windowId);
    if (!window || window->property("DiskPwdChangingDialogShown").toBool()) {
        return;
    }

    DiskPasswordChangingDialog *dialog = new DiskPasswordChangingDialog(window);
    dialog->show();
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    window->setProperty("DiskPwdChangingDialogShown", true);
    QObject::connect(dialog, &DiskPasswordChangingDialog::closed, window, [window] {
        window->setProperty("DiskPwdChangingDialogShown", false);
    });
}

void TitleBarHelper::registerKeepTitleStatusScheme(const QString &scheme)
{
    if (!kKeepTitleStatusSchemeList.contains(scheme))
        kKeepTitleStatusSchemeList.append(scheme);
}

bool TitleBarHelper::checkKeepTitleStatus(const QUrl &url)
{
    auto scheme = url.scheme();
    return kKeepTitleStatusSchemeList.contains(scheme);
}

void TitleBarHelper::registerViewModelUrlCallback(const QString &scheme, ViewModeUrlCallback callback)
{
    if (!kViewModeUrlCallbackMap.contains(scheme))
        kViewModeUrlCallbackMap.insert(scheme, callback);
}

ViewModeUrlCallback TitleBarHelper::viewModelUrlCallback(const QUrl &url)
{
    auto scheme = url.scheme();
    return kViewModeUrlCallbackMap.value(scheme);
}

QMutex &TitleBarHelper::mutex()
{
    static QMutex m;
    return m;
}

void TitleBarHelper::handleSettingMenuTriggered(quint64 windowId, int action)
{
    switch (static_cast<MenuAction>(action)) {
    case MenuAction::kNewWindow:
        TitleBarEventCaller::sendOpenWindow(QUrl());
        break;
    case MenuAction::kOpenInNewTab:
        TitleBarHelper::openCurrentUrlInNewTab(windowId);
        break;
    case MenuAction::kSettings:
        TitleBarHelper::showSettingsDialog(windowId);
        break;
    case MenuAction::kConnectToServer:
        TitleBarHelper::showConnectToServerDialog(windowId);
        break;
    case MenuAction::kSetUserSharePassword:
        TitleBarHelper::showUserSharePasswordSettingDialog(windowId);
        break;
    case MenuAction::kChangeDiskPassword:
        TitleBarHelper::showDiskPasswordChangingDialog(windowId);
        break;
    }
}

QString TitleBarHelper::getDisplayName(const QString &name)
{
    QString displayName { SystemPathUtil::instance()->systemPathDisplayName(name) };
    displayName = displayName.isEmpty() ? name : displayName;
    return displayName;
}

QUrl TitleBarHelper::transformViewModeUrl(const QUrl &url)
{
    auto callback = viewModelUrlCallback(url);
    return callback ? callback(url) : url;
}

QVariant TitleBarHelper::getFileViewStateValue(const QUrl &url, const QString &key, const QVariant &defaultValue)
{
    QUrl viewModeUrl = transformViewModeUrl(url);
    QMap<QString, QVariant> valueMap = Application::appObtuselySetting()->value("FileViewState", viewModeUrl).toMap();
    return valueMap.value(key, defaultValue);
}

void TitleBarHelper::setFileViewStateValue(const QUrl &url, const QString &key, const QVariant &value)
{
    QUrl viewModeUrl = transformViewModeUrl(url);
    QVariantMap map = Application::appObtuselySetting()->value("FileViewState", viewModeUrl).toMap();
    map[key] = value;
    Application::appObtuselySetting()->setValue("FileViewState", viewModeUrl, map);
}

bool TitleBarHelper::isTreeViewGloballyEnabled()
{
    return DConfigManager::instance()->value(kViewDConfName, kTreeViewEnable, true).toBool();
}

bool TitleBarHelper::isViewModeVisibleForScheme(int mode, const QString &scheme)
{
    // If no scheme restriction, all modes are visible
    if (!OptionButtonManager::instance()->hasVsibleState(scheme))
        return true;

    auto state = OptionButtonManager::instance()->optBtnVisibleState(scheme);

    switch (mode) {
    case 0:   // Icon mode
        return !(state & OptionButtonManager::kHideIconViewBtn);
    case 1:   // List mode
        return !(state & OptionButtonManager::kHideListViewBtn);
    case 2:   // Tree mode
        return !(state & OptionButtonManager::kHideTreeViewBtn);
    default:
        return false;
    }
}
