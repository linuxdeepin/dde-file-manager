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
#include "sharemenuscene.h"
#include "private/sharemenuscene_p.h"
#include "action_defines.h"
#include "menuutils.h"

#include "dfm-base/dfm_menu_defines.h"
#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/dbusservice/global_server_defines.h"
#include "dfm-base/base/device/deviceproxymanager.h"
#include "dfm-base/base/device/deviceutils.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/utils/fileutils.h"

#include <dfm-framework/dpf.h>

#include <QMenu>
#include <QFileInfo>
#include <QApplication>
#include <QStandardPaths>

Q_DECLARE_METATYPE(QList<QUrl> *)

using namespace dfmplugin_menu;
DFMBASE_USE_NAMESPACE

ShareMenuScene::ShareMenuScene(QObject *parent)
    : AbstractMenuScene(parent),
      d(new ShareMenuScenePrivate(this))
{
}

ShareMenuScene::~ShareMenuScene()
{
}

QString ShareMenuScene::name() const
{
    return ShareMenuCreator::name();
}

bool ShareMenuScene::initialize(const QVariantHash &params)
{
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    if (!d->selectFiles.isEmpty())
        d->focusFile = d->selectFiles.first();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();
    d->onDesktop = params.value(MenuParamKey::kOnDesktop).toBool();
    d->windowId = params.value(MenuParamKey::kWindowId).toULongLong();

    const auto &tmpParams = dfmplugin_menu::MenuUtils::perfectMenuParams(params);
    d->isFocusOnDDEDesktopFile = tmpParams.value(MenuParamKey::kIsFocusOnDDEDesktopFile, false).toBool();
    d->isSystemPathIncluded = tmpParams.value(MenuParamKey::kIsSystemPathIncluded, false).toBool();

    for (auto url : d->selectFiles) {
        auto f = DFMBASE_NAMESPACE::InfoFactory::create<AbstractFileInfo>(url, true);
        if (f->isDir()) {
            d->folderSelected = true;
            break;
        }
    }

    if (d->selectFiles.isEmpty())
        return false;

    return AbstractMenuScene::initialize(params);
}

bool ShareMenuScene::create(QMenu *parent)
{
    if (!parent)
        return false;

    if (!d->isEmptyArea) {
        if (d->isFocusOnDDEDesktopFile)
            return AbstractMenuScene::create(parent);

        if (!d->onDesktop) {
            auto *act = parent->addAction(d->predicateName[ActionID::kSendToDesktop]);
            act->setProperty(ActionPropertyKey::kActionID, ActionID::kSendToDesktop);
            d->predicateAction[ActionID::kSendToDesktop] = act;
        }

        if (!d->isSystemPathIncluded) {
            auto shareAct = parent->addAction(d->predicateName[ActionID::kShare]);
            shareAct->setProperty(ActionPropertyKey::kActionID, ActionID::kShare);
            d->predicateAction[ActionID::kShare] = shareAct;

            QMenu *shareMenu = new QMenu(parent);
            d->addSubActions(shareMenu);
            shareAct->setMenu(shareMenu);

            if (shareMenu->actions().isEmpty())
                shareAct->setVisible(false);
        }
    }
    return AbstractMenuScene::create(parent);
}

void ShareMenuScene::updateState(QMenu *parent)
{
    // remove device self action
    if (!d->isEmptyArea) {
        auto actions = parent->actions();
        bool removed = false;
        for (auto act : actions) {
            if (removed)
                break;

            if (act->isSeparator())
                continue;

            auto actId = act->property(ActionPropertyKey::kActionID).toString();

            if (actId == "send-to") {
                auto subMenu = act->menu();
                for (QAction *action : subMenu->actions()) {
                    const QUrl &urlData = action->data().toUrl();
                    if (urlData.isValid() && d->focusFile.toString().startsWith(urlData.toString())) {
                        subMenu->removeAction(action);
                        removed = true;
                        break;
                    }
                }
            }
        }
    }

    AbstractMenuScene::updateState(parent);
}

bool ShareMenuScene::triggered(QAction *action)
{
    if (!action)
        return false;

    if (!d->predicateAction.key(action).isEmpty()) {
        d->handleActionTriggered(action);
        return true;
    }

    return AbstractMenuScene::triggered(action);
}

dfmbase::AbstractMenuScene *ShareMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (!d->predicateAction.key(action).isEmpty())
        return const_cast<ShareMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

dfmbase::AbstractMenuScene *ShareMenuCreator::create()
{
    return new ShareMenuScene();
}

ShareMenuScenePrivate::ShareMenuScenePrivate(AbstractMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
    predicateName[ActionID::kShare] = tr("Share");
    predicateName[ActionID::kShareToBluetooth] = tr("Bluetooth");
    predicateName[ActionID::kSendToDesktop] = tr("Send to desktop");
}

void ShareMenuScenePrivate::addSubActions(QMenu *subMenu)
{
    if (!subMenu)
        return;

    bool bluetoothAvailable = dpfSlotChannel->push("dfmplugin_utils", "slot_Bluetooth_IsAvailable").toBool();
    if (bluetoothAvailable) {
        auto *act = subMenu->addAction(predicateName[ActionID::kShareToBluetooth]);
        act->setProperty(ActionPropertyKey::kActionID, ActionID::kShareToBluetooth);
        if (folderSelected)
            act->setEnabled(false);
        predicateAction[ActionID::kShareToBluetooth] = act;
    }

    using namespace GlobalServerDefines;
    auto devs = DevProxyMng->getAllBlockIds(DeviceQueryOption::kMounted | DeviceQueryOption::kRemovable);
    auto dedupedDevs = devs.toSet();
    int idx = 0;
    for (const QString &dev : dedupedDevs) {
        auto &&data = DevProxyMng->queryBlockInfo(dev);
        QString label = DeviceUtils::convertSuitableDisplayName(data);
        QString mpt = data.value(DeviceProperty::kMountPoint).toString();
        QString devDesc = data.value(DeviceProperty::kDevice).toString();

        if (data.value(DeviceProperty::kOptical).toBool()) {
            continue;   // this should be added in burn plugin.
        } else {
            QAction *act = subMenu->addAction(label);
            act->setData(QUrl::fromLocalFile(mpt));
            QString actId = ActionID::kSendToRemovablePrefix + QString::number(idx++);
            predicateAction.insert(actId, act);
            act->setProperty(ActionPropertyKey::kActionID, actId);
        }
    }
}

void ShareMenuScenePrivate::handleActionTriggered(QAction *act)
{
    if (!act)
        return;

    QStringList filePaths;
    for (const auto &url : selectFiles) {
        auto f = DFMBASE_NAMESPACE::InfoFactory::create<AbstractFileInfo>(url, true);
        filePaths << f->absoluteFilePath();
    }
    QString actId = act->property(ActionPropertyKey::kActionID).toString();
    if (actId == ActionID::kShareToBluetooth) {
        dpfSlotChannel->push("dfmplugin_utils", "slot_Bluetooth_SendFiles", filePaths, "");
    } else if (actId.startsWith(ActionID::kSendToRemovablePrefix)) {
        qDebug() << "send files to: " << act->data().toUrl() << ", " << selectFiles;
        dpfSignalDispatcher->publish(GlobalEventType::kCopy, QApplication::activeWindow()->winId(), selectFiles, act->data().toUrl(), AbstractJobHandler::JobFlag::kNoHint, nullptr);
    } else if (actId == ActionID::kSendToDesktop) {
        QString desktopPath = StandardPaths::location(StandardPaths::kDesktopPath);
        QList<QUrl> urlsTrans = selectFiles;
        QList<QUrl> urls {};
        bool ok = dpfHookSequence->run("dfmplugin_utils", "hook_UrlsTransform", urlsTrans, &urls);
        if (ok && !urls.isEmpty())
            urlsTrans = urls;

        for (const QUrl &url : urlsTrans) {
            QString linkName = FileUtils::nonExistSymlinkFileName(url);
            QUrl linkUrl = QUrl::fromLocalFile(desktopPath + "/" + linkName);
            dpfSignalDispatcher->publish(GlobalEventType::kCreateSymlink, windowId, url, linkUrl, false, true);
        }
    }
}
