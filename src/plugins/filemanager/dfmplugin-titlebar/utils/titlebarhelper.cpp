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
#include "views/titlebarwidget.h"

#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/file/local/localfileinfo.h"
#include "dfm-base/utils/systempathutil.h"
#include "dfm-base/utils/finallyutil.h"
#include "dfm-base/utils/dialogmanager.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"

#include <dfm-framework/dpf.h>

#include <QGSettings>
#include <QStandardPaths>
#include <QStorageInfo>

using namespace dfmplugin_titlebar;
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
    return FMWindowsIns.findWindowId(sender);
}

QMenu *TitleBarHelper::createSettingsMenu(quint64 id)
{
    QMenu *menu = new QMenu();

    QAction *action { new QAction(QObject::tr("New window")) };
    action->setData(MenuAction::kNewWindow);
    menu->addAction(action);

    menu->addSeparator();

    action = new QAction(QObject::tr("Connect to Server"));
    action->setData(MenuAction::kConnectToServer);
    menu->addAction(action);

    action = new QAction(QObject::tr("Set share password"));
    action->setData(MenuAction::kSetUserSharePassword);
    menu->addAction(action);

    action = new QAction(QObject::tr("Settings"));
    action->setData(MenuAction::kSettings);
    menu->addAction(action);

    QObject::connect(menu, &QMenu::triggered, [id](QAction *act) {
        bool ok { false };
        int val { act->data().toInt(&ok) };
        if (ok)
            handleSettingMenuTriggered(id, val);
    });

    return menu;
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
        const QString &&iconName = QStringLiteral("drive-harddisk-symbolic");

        // for gvfs files do not construct QStorageInfo object which costs a lots time.
        QRegularExpression rex(Global::Regex::kGvfsRoot);
        auto match = rex.match(path);
        if (match.hasMatch()) {
            prefixPath = match.captured();
            CrumbData data { QUrl::fromLocalFile(prefixPath), "", iconName };
            list.append(data);
        } else {
            QStorageInfo storageInfo(path);
            if (storageInfo.isValid()) {

                prefixPath = storageInfo.rootPath();
                // TODO(zhangs): device info  (ref DFMFileCrumbController::seprateUrl)

                if (prefixPath == "/") {
                    CrumbData data(UrlRoute::rootUrl(Global::Scheme::kFile), getDisplayName("System Disk"), "drive-harddisk-root-symbolic");
                    list.append(data);
                } else {
                    CrumbData data(QUrl::fromLocalFile(prefixPath), QString(), iconName);
                    list.append(data);
                }
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
    QGSettings settings("com.deepin.dde.filemanager.general", "/com/deepin/dde/filemanager/general/");
    return settings.get("contextMenuIcons").toBool();
}

void TitleBarHelper::handlePressed(QWidget *sender, const QString &text, bool *isSearch)
{
    const auto &currentDir = QDir::currentPath();
    QUrl currentUrl;
    auto curTitleBar = findTileBarByWindowId(windowId(sender));
    if (curTitleBar)
        currentUrl = curTitleBar->currentUrl();

    if (currentUrl.isLocalFile())
        QDir::setCurrent(currentUrl.toLocalFile());

    QString inputStr = text;
    TitleBarEventCaller::sendCheckAddressInputStr(&inputStr);

    bool search { false };
    FinallyUtil finally([&]() {if (isSearch) *isSearch = search; });

    // here, judge whether the text is a local file path.
    QUrl url(UrlRoute::fromUserInput(inputStr, false));
    QDir::setCurrent(currentDir);

    QString scheme { url.scheme() };
    if (!url.scheme().isEmpty() && UrlRoute::hasScheme(scheme)) {
        if (url.path().isEmpty())
            url = UrlRoute::fromUserInput(inputStr + "/");
        qInfo() << "jump :" << inputStr;
        TitleBarEventCaller::sendCd(sender, url);
    } else {
        if (currentUrl.isValid()) {
            bool isDisableSearch = dpfSlotChannel->push("dfmplugin_search", "slot_Custom_IsDisableSearch", currentUrl).toBool();
            if (isDisableSearch) {
                qInfo() << "search : current directory disable to search! " << currentUrl;
                return;
            }
        }

        search = true;
        qInfo() << "search :" << text;
        TitleBarEventCaller::sendSearch(sender, text);
    }
}

void TitleBarHelper::showSettingsDialog(quint64 windowId)
{
    auto window = FMWindowsIns.findWindowById(windowId);

    if (!window) {
        qWarning() << "Invalid window id: " << windowId;
        return;
    }

    DialogManagerInstance->showSetingsDialog(window);
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
    switch (static_cast<MenuAction>(action)) {
    case MenuAction::kNewWindow:
        TitleBarEventCaller::sendOpenWindow(QUrl());
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
    }
}

QString TitleBarHelper::getDisplayName(const QString &name)
{
    QString displayName { SystemPathUtil::instance()->systemPathDisplayName(name) };
    displayName = displayName.isEmpty() ? name : displayName;
    return displayName;
}
