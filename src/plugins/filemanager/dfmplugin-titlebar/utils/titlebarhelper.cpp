/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "titlebarhelper.h"
#include "events/titlebareventcaller.h"
#include "dialogs/connecttoserverdialog.h"
#include "dialogs/usersharepasswordsettingdialog.h"

#include "services/filemanager/titlebar/titlebar_defines.h"
#include "services/filemanager/windows/windowsservice.h"
#include "services/filemanager/workspace/workspaceservice.h"

#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/device/devicecontroller.h"
#include "dfm-base/file/local/localfileinfo.h"
#include "dfm-base/utils/systempathutil.h"
#include "dfm-base/utils/finallyutil.h"
#include "dfm-base/utils/dialogmanager.h"

#include <dfm-framework/framework.h>

#include <QGSettings>

DPTITLEBAR_USE_NAMESPACE
DSB_FM_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

QMap<quint64, TitleBarWidget *> TitleBarHelper::kTitleBarMap {};

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
    auto &ctx = dpfInstance.serviceContext();
    auto windowService = ctx.service<WindowsService>(WindowsService::name());
    return windowService->findWindowId(sender);
}

QMenu *TitleBarHelper::createSettingsMenu(quint64 id)
{
    QMenu *menu = new QMenu();

    QAction *action { new QAction(QObject::tr("New window")) };
    action->setData(TitleBar::MenuAction::kNewWindow);
    menu->addAction(action);

    menu->addSeparator();

    action = new QAction(QObject::tr("Connect to Server"));
    action->setData(TitleBar::MenuAction::kConnectToServer);
    menu->addAction(action);

    action = new QAction(QObject::tr("Set share password"));
    action->setData(TitleBar::MenuAction::kSetUserSharePassword);
    menu->addAction(action);

    action = new QAction(QObject::tr("Settings"));
    action->setData(TitleBar::MenuAction::kSettings);
    menu->addAction(action);

    QObject::connect(menu, &QMenu::triggered, [id](QAction *act) {
        bool ok { false };
        int val { act->data().toInt(&ok) };
        if (ok)
            handleSettingMenuTriggered(id, val);
    });

    return menu;
}

bool TitleBarHelper::crumbSupportedUrl(const QUrl &url)
{
    return url.scheme() == SchemeTypes::kFile;
}

QList<CrumbData> TitleBarHelper::crumbSeprateUrl(const QUrl &url)
{
    static const QString kHomePath { QStandardPaths::standardLocations(QStandardPaths::HomeLocation).last() };

    QList<CrumbData> list;
    const QString &path = url.toLocalFile();
    if (path.isEmpty())
        return list;

    QString prefixPath { "/" };
    if (path.startsWith(kHomePath)) {
        prefixPath = kHomePath;
        QString iconName { SystemPathUtil::instance()->systemPathIconName("Home") };
        CrumbData data { QUrl::fromLocalFile(kHomePath), getDisplayName("Home"), iconName };
        list.append(data);
    } else {
        QStorageInfo storageInfo(path);
        if (storageInfo.isValid()) {
            QString iconName = QStringLiteral("drive-harddisk-symbolic");
            prefixPath = storageInfo.rootPath();
            // TODO(zhangs): device info  (ref DFMFileCrumbController::seprateUrl)

            if (prefixPath == "/") {
                CrumbData data(UrlRoute::rootUrl(SchemeTypes::kFile), getDisplayName("System Disk"), "drive-harddisk-root-symbolic");
                list.append(data);
            } else {
                CrumbData data(QUrl::fromLocalFile(prefixPath), QString(), iconName);
                list.append(data);
            }
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
            auto infoPointer = InfoFactory::create<DFMBASE_NAMESPACE::LocalFileInfo>(oneUrl);
            if (infoPointer)
                displayText = infoPointer->fileDisplayName();
            CrumbData data(oneUrl, displayText);
            list.append(data);
        }
        ++iter;
    }

    return list;
}

bool TitleBarHelper::displayIcon()
{
    QGSettings settings("com.deepin.dde.filemanager.general", "/com/deepin/dde/filemanager/general/");
    return settings.get("context-menu-icons").toBool();
}

bool TitleBarHelper::tabAddable(quint64 windowId)
{

    auto &ctx = dpfInstance.serviceContext();
    auto workspaceService = ctx.service<WorkspaceService>(WorkspaceService::name());
    if (workspaceService)
        return workspaceService->tabAddable(windowId);

    return false;
}

void TitleBarHelper::handlePressed(QWidget *sender, const QString &text, bool *isSearch)
{
    bool search { false };
    FinallyUtil finally([&]() {if (isSearch) *isSearch = search; });
    QUrl url(UrlRoute::fromUserInput(text));
    QString scheme { url.scheme() };
    if (!url.scheme().isEmpty() && UrlRoute::hasScheme(scheme)) {
        if (url.path().isEmpty())
            url = UrlRoute::fromUserInput(text + "/");
        qInfo() << "jump :" << text;
        TitleBarEventCaller::sendCd(sender, url);
    } else {
        search = true;
        qInfo() << "search :" << text;
        TitleBarEventCaller::sendSearch(sender, text);
    }
}

bool TitleBarHelper::handleConnection(QWidget *sender, const QUrl &url)
{
    QString &&scheme = url.scheme();
    if (scheme != SchemeTypes::kSmb && scheme != SchemeTypes::kFtp && scheme != SchemeTypes::kSFtp)
        return false;

    // TODO(xust) see if i can find any other way to handle the choise (browse the smb shares and mount the samba directly)
    if (scheme == SchemeTypes::kSmb && url.path() == "/")
        return false;

    if (url.host().isEmpty()) {
        DialogManagerInstance->showErrorDialog("", QObject::tr("Mounting device error"));
        return true;
    }

    DeviceController::instance()->mountNetworkDevice(url.toString(), [sender](bool ok, DFMMOUNT::DeviceError err, const QString &mntPath) {
        if (!ok && err != DFMMOUNT::DeviceError::GIOErrorAlreadyMounted) {
            DialogManagerInstance->showErrorDialogWhenMountDeviceFailed(err);
        } else {
            QUrl u;
            u.setScheme(SchemeTypes::kFile);
            u.setPath(mntPath);
            TitleBarEventCaller::sendCd(sender, u);
        }
    });

    return true;
}

void TitleBarHelper::showSettingsDialog(quint64 windowId)
{
    auto &ctx = dpfInstance.serviceContext();
    auto windowService = ctx.service<WindowsService>(WindowsService::name());
    auto window = windowService->findWindowById(windowId);

    if (!window) {
        qWarning() << "Invalid window id: " << windowId;
        return;
    }

    DialogManagerInstance->showSetingsDialog(window);
}

void TitleBarHelper::showConnectToServerDialog(quint64 windowId)
{
    auto &ctx = dpfInstance.serviceContext();
    auto windowService = ctx.service<WindowsService>(WindowsService::name());
    auto window = windowService->findWindowById(windowId);

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
    auto &ctx = dpfInstance.serviceContext();
    auto windowService = ctx.service<WindowsService>(WindowsService::name());
    auto window = windowService->findWindowById(windowId);
    if (!window || window->property("UserSharePwdSettingDialogShown").toBool()) {
        return;
    }

    UserSharePasswordSettingDialog *dialog = new UserSharePasswordSettingDialog(window);
    dialog->show();
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    QObject::connect(dialog, &UserSharePasswordSettingDialog::finished, dialog, &UserSharePasswordSettingDialog::onButtonClicked);
    window->setProperty("UserSharePwdSettingDialogShown", true);
    QObject::connect(dialog, &UserSharePasswordSettingDialog::closed, [=] {
        window->setProperty("UserSharePwdSettingDialogShown", false);
    });
}

QMutex &TitleBarHelper::mutex()
{
    static QMutex m;
    return m;
}

void TitleBarHelper::handleSettingMenuTriggered(quint64 windowId, int action)
{
    switch (static_cast<TitleBar::MenuAction>(action)) {
    case TitleBar::MenuAction::kNewWindow:
        TitleBarEventCaller::sendOpenWindow(QUrl());
        break;
    case TitleBar::MenuAction::kSettings:
        TitleBarHelper::showSettingsDialog(windowId);
        break;
    case TitleBar::MenuAction::kConnectToServer:
        TitleBarHelper::showConnectToServerDialog(windowId);
        break;
    case TitleBar::MenuAction::kSetUserSharePassword:
        TitleBarHelper::showUserSharePasswordSettingDialog(windowId);
        break;
    }
}

QString TitleBarHelper::getDisplayName(const QString &name)
{
    QString displayName { SystemPathUtil::instance()->systemPathDisplayName(name) };
    displayName = displayName.isEmpty() ? name : displayName;
    return displayName;
}
