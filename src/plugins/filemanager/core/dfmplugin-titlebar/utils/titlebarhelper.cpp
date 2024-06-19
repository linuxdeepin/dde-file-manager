// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "titlebarhelper.h"
#include "events/titlebareventcaller.h"
#include "dialogs/connecttoserverdialog.h"
#include "dialogs/diskpasswordchangingdialog.h"
#include "views/titlebarwidget.h"

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

#include <dfm-gui/windowmanager.h>

#include <dfm-framework/dpf.h>

#include <DTitlebar>

// #include <QGSettings>
#include <QStandardPaths>
#include <QSettings>

using namespace dfmplugin_titlebar;
DFMBASE_USE_NAMESPACE

QMap<quint64, TitleBarWidget *> TitleBarHelper::kTitleBarMap {};

bool TitleBarHelper::newWindowAndTabEnabled { true };

TitleBarWidget *TitleBarHelper::findTileBarByWindowId(quint64 windowId)
{
    if (!kTitleBarMap.contains(windowId))
        return nullptr;

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

quint64 TitleBarHelper::windowId(dfmgui::Applet *applet)
{
    return FMQuickWindowIns->findWindowIdFromApplet(applet);
}

void TitleBarHelper::createSettingsMenu(quint64 id)
{
    QMenu *menu = new QMenu();

    QAction *action { new QAction(QObject::tr("New window"), menu) };
    action->setData(MenuAction::kNewWindow);
    menu->addAction(action);

    action = new QAction(QObject::tr("Open in new tab"), menu);
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

    auto window = FMWindowsIns.findWindowById(id);
    Q_ASSERT_X(window, "TitleBar", "Cannot find window by id");
    auto defaultMenu = window->titlebar()->menu();
    if (defaultMenu && !defaultMenu->isEmpty()) {
        for (auto *act : defaultMenu->actions()) {
            act->setParent(menu);
            menu->addAction(act);
        }
    }

    window->titlebar()->setMenu(menu);
}

QList<CrumbData> TitleBarHelper::crumbSeprateUrl(const QUrl &url)
{
    static const QString kHomePath { QStandardPaths::standardLocations(QStandardPaths::HomeLocation).last() };

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
        QString localFile = oneUrl.toLocalFile();
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

bool TitleBarHelper::displayIcon()
{
    //! FIXME: ? qgsetting 不再使用
    // QGSettings settings("com.deepin.dde.filemanager.general", "/com/deepin/dde/filemanager/general/");
    // return settings.get("contextMenuIcons").toBool();
    return false;
}

void TitleBarHelper::handlePressed(QWidget *sender, const QString &text, bool *isSearch)
{
    const auto &currentDir = QDir::currentPath();
    QUrl currentUrl;
    auto curTitleBar = findTileBarByWindowId(windowId(sender));
    if (curTitleBar)
        currentUrl = curTitleBar->currentUrl();

    if (dfmbase::FileUtils::isLocalFile(currentUrl))
        QDir::setCurrent(currentUrl.toLocalFile());

    QString inputStr = text;
    TitleBarEventCaller::sendCheckAddressInputStr(sender, &inputStr);

    bool search { false };
    FinallyUtil finally([&]() {if (isSearch) *isSearch = search; });

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
        if (currentUrl.isValid()) {
            bool isDisableSearch = dpfSlotChannel->push("dfmplugin_search", "slot_Custom_IsDisableSearch", currentUrl).toBool();
            if (isDisableSearch) {
                fmInfo() << "search : current directory disable to search! " << currentUrl;
                return;
            }
        }

        search = true;
        fmInfo() << "search :" << text;
        TitleBarEventCaller::sendSearch(sender, text);
    }
}

void TitleBarHelper::openCurrentUrlInNewTab(quint64 windowId)
{
    FileManagerWindowsManager::FMWindow *window = FMWindowsIns.findWindowById(windowId);
    if (!window)
        return;

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
    QObject::connect(dialog, &ConnectToServerDialog::finished, dialog, &ConnectToServerDialog::onButtonClicked);
    window->setProperty("ConnectToServerDialogShown", true);
    QObject::connect(dialog, &ConnectToServerDialog::closed, [window] {
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
    QObject::connect(dialog, &UserSharePasswordSettingDialog::finished, dialog, &UserSharePasswordSettingDialog::onButtonClicked);
    QObject::connect(dialog, &UserSharePasswordSettingDialog::inputPassword, [=](const QString &password) {
        dpfSignalDispatcher->publish("dfmplugin_titlebar", "signal_Share_SetPassword", password);
    });
    window->setProperty("UserSharePwdSettingDialogShown", true);
    QObject::connect(dialog, &UserSharePasswordSettingDialog::closed, [=] {
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
    QObject::connect(dialog, &DiskPasswordChangingDialog::closed, [=] {
        window->setProperty("DiskPwdChangingDialogShown", false);
    });
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
