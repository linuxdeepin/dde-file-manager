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
#include "sendtomenuscene.h"
#include "private/sendtomenuscene_p.h"
#include "action_defines.h"
#include "menuutils.h"

#include "services/common/menu/menu_defines.h"
#include "services/common/delegate/delegateservice.h"

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/dbusservice/global_server_defines.h"
#include "dfm-base/base/device/deviceproxymanager.h"
#include "dfm-base/base/device/deviceutils.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/utils/fileutils.h"

#include <dfm-framework/framework.h>

#include <QMenu>
#include <QFileInfo>
#include <QApplication>
#include <QStandardPaths>

using namespace dfmplugin_menu;
DFMBASE_USE_NAMESPACE

SendToMenuScene::SendToMenuScene(QObject *parent)
    : AbstractMenuScene(parent),
      d(new SendToMenuScenePrivate(this))
{
}

SendToMenuScene::~SendToMenuScene()
{
}

QString SendToMenuScene::name() const
{
    return SendToMenuCreator::name();
}

bool SendToMenuScene::initialize(const QVariantHash &params)
{
    DSC_USE_NAMESPACE
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

bool SendToMenuScene::create(QMenu *parent)
{
    if (!parent)
        return false;

    if (!d->isEmptyArea) {
        if (d->isFocusOnDDEDesktopFile)
            return AbstractMenuScene::create(parent);

        if (!d->onDesktop) {
            auto *act = parent->addAction(d->predicateName[ActionID::kSendToDesktop]);
            act->setProperty(DSC_NAMESPACE::ActionPropertyKey::kActionID, ActionID::kSendToDesktop);
            d->predicateAction[ActionID::kSendToDesktop] = act;
        }

        if (!d->isSystemPathIncluded) {
            QMenu *sendToMenu = new QMenu(parent);
            d->addSubActions(sendToMenu);
            if (sendToMenu->actions().isEmpty()) {
                delete sendToMenu;
                sendToMenu = nullptr;
            } else {
                auto sendToAct = parent->addAction(d->predicateName[ActionID::kSendTo]);
                sendToAct->setMenu(sendToMenu);
                sendToAct->setProperty(DSC_NAMESPACE::ActionPropertyKey::kActionID, ActionID::kSendTo);
                d->predicateAction[ActionID::kSendTo] = sendToAct;
            }
        }
    }
    return AbstractMenuScene::create(parent);
}

void SendToMenuScene::updateState(QMenu *parent)
{
    // TODO(xust)
    AbstractMenuScene::updateState(parent);
}

bool SendToMenuScene::triggered(QAction *action)
{
    if (!action)
        return false;

    if (!d->predicateAction.key(action).isEmpty()) {
        d->handleActionTriggered(action);
        return true;
    }

    return AbstractMenuScene::triggered(action);
}

dfmbase::AbstractMenuScene *SendToMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (!d->predicateAction.key(action).isEmpty())
        return const_cast<SendToMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

dfmbase::AbstractMenuScene *SendToMenuCreator::create()
{
    return new SendToMenuScene();
}

SendToMenuScenePrivate::SendToMenuScenePrivate(AbstractMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
    predicateName[ActionID::kSendTo] = tr("Send to");
    predicateName[ActionID::kSendToBluetooth] = tr("Bluetooth");
    predicateName[ActionID::kSendToDesktop] = tr("Send to desktop");
}

void SendToMenuScenePrivate::addSubActions(QMenu *subMenu)
{
    if (!subMenu)
        return;

    bool bluetoothAvailable = dpfSlotChannel->push("dfmplugin_utils", "slot_Bluetooth_IsAvailable").toBool();
    if (bluetoothAvailable) {
        auto *act = subMenu->addAction(predicateName[ActionID::kSendToBluetooth]);
        act->setProperty(DSC_NAMESPACE::ActionPropertyKey::kActionID, ActionID::kSendToBluetooth);
        if (folderSelected)
            act->setEnabled(false);
        predicateAction[ActionID::kSendToBluetooth] = act;
    }

    using namespace GlobalServerDefines;
    QAction *firstOptical { nullptr };
    auto devs = DevProxyMng->getAllBlockIds(DeviceQueryOption::kMounted | DeviceQueryOption::kRemovable);
    devs << DevProxyMng->getAllBlockIds(DeviceQueryOption::kOptical);
    auto dedupedDevs = devs.toSet();
    int idx = 0;
    for (const QString &dev : dedupedDevs) {
        auto &&data = DevProxyMng->queryBlockInfo(dev);
        QString label = DeviceUtils::convertSuitableDisplayName(data);
        QString mpt = data.value(DeviceProperty::kMountPoint).toString();
        QString devDesc = data.value(DeviceProperty::kDevice).toString();

        QAction *act { nullptr };
        if (data.value(DeviceProperty::kOptical).toBool()) {
            act = subMenu->addAction(label);
            if (!firstOptical)
                firstOptical = act;

            auto u = QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation)
                                         + "/" + qApp->organizationName() + "/discburn/" + devDesc.replace("/", "_"));
            act->setData(u);
        } else {
            if (firstOptical) {
                act = new QAction(label, subMenu);
                subMenu->insertAction(firstOptical, act);
            } else {
                act = subMenu->addAction(label);
            }
            act->setData(QUrl::fromLocalFile(mpt));
        }

        if (act) {
            QString actId = ActionID::kSendToRemovablePrefix + QString::number(idx++);
            predicateAction.insert(actId, act);
            act->setProperty(DSC_NAMESPACE::ActionPropertyKey::kActionID, actId);
        }
    }
}

void SendToMenuScenePrivate::handleActionTriggered(QAction *act)
{
    if (!act)
        return;

    QStringList filePaths;
    for (const auto &url : selectFiles) {
        auto f = DFMBASE_NAMESPACE::InfoFactory::create<AbstractFileInfo>(url, true);
        filePaths << f->absoluteFilePath();
    }
    DSC_USE_NAMESPACE
    QString actId = act->property(ActionPropertyKey::kActionID).toString();
    if (actId == ActionID::kSendToBluetooth) {
        dpfSlotChannel->push("dfmplugin_utils", "slot_Bluetooth_SendFiles", filePaths, "");
    } else if (actId.startsWith(ActionID::kSendToRemovablePrefix)) {
        qDebug() << "send files to: " << act->data().toUrl() << ", " << selectFiles;
        dpfSignalDispatcher->publish(GlobalEventType::kCopy, QApplication::activeWindow()->winId(), selectFiles, act->data().toUrl(), AbstractJobHandler::JobFlag::kNoHint, nullptr);
    } else if (actId == ActionID::kSendToDesktop) {
        QString desktopPath = StandardPaths::location(StandardPaths::kDesktopPath);
        const QList<QUrl> &urlsTrans = delegateServIns->urlsTransform(selectFiles);
        for (const auto &url : urlsTrans) {
            QString linkName = FileUtils::nonExistSymlinkFileName(url);
            QUrl linkUrl = QUrl::fromLocalFile(desktopPath + "/" + linkName);
            dpfSignalDispatcher->publish(GlobalEventType::kCreateSymlink,
                                         windowId,
                                         url,
                                         linkUrl,
                                         false,
                                         true);
        }
    }
}
