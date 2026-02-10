// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sendtodiscmenuscene.h"
#include "sendtodiscmenuscene_p.h"
#include "utils/burnhelper.h"
#include "events/burneventreceiver.h"
#include "events/burneventcaller.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/utils/universalutils.h>

#include <QMenu>
#include <QProcess>

Q_DECLARE_METATYPE(QList<QUrl> *)

using namespace dfmplugin_burn;
using namespace GlobalServerDefines;

SendToDiscMenuScenePrivate::SendToDiscMenuScenePrivate(AbstractMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
}

void SendToDiscMenuScenePrivate::actionStageFileForBurning(const QString &dev)
{
    if (dev.isEmpty())
        return;
    QUrl dest { BurnHelper::fromBurnFile(dev) };
    QList<QUrl> srcUrls { selectFiles };

    QList<QUrl> urls {};
    bool ok = UniversalUtils::urlsTransformToLocal(srcUrls, &urls);
    if (ok && !urls.isEmpty())
        srcUrls = urls;

    BurnEventReceiver::instance()->handlePasteTo(srcUrls, dest, true);
}

void SendToDiscMenuScenePrivate::actionPacketWriting(const QString &dev)
{
    if (dev.isEmpty())
        return;

    const QString &mntPoint { DeviceUtils::getMountInfo(dev) };
    if (mntPoint.isEmpty())
        return;

    QUrl dest { QUrl::fromLocalFile(mntPoint) };
    QList<QUrl> srcUrls { selectFiles };
    QList<QUrl> urls {};
    bool ok = UniversalUtils::urlsTransformToLocal(srcUrls, &urls);
    if (ok && !urls.isEmpty())
        srcUrls = urls;

    BurnEventCaller::sendPasteFiles(srcUrls, dest, true);
}

void SendToDiscMenuScenePrivate::actionMountImage()
{
    BurnEventReceiver::instance()->handleMountImage(windowId, focusFile);
}

void SendToDiscMenuScenePrivate::initDestDevices()
{
    bool filterFlag { false };
    // remove self disc id
    auto &&dataGroup { BurnHelper::discDataGroup() };
    for (const auto &data : dataGroup) {
        QString curDev { BurnHelper::burnDestDevice(currentDir) };
        QString dev { data[DeviceProperty::kDevice].toString() };
        if (curDev != dev)
            destDeviceDataGroup.push_back(data);
        else
            filterFlag = true;
    }

    // only self disc, disable action
    if (filterFlag && destDeviceDataGroup.isEmpty())
        disableStage = true;
}

void SendToDiscMenuScenePrivate::addSubStageActions(QMenu *menu)
{
    Q_ASSERT(menu);

    for (const auto &data : destDeviceDataGroup) {
        QString label { DeviceUtils::convertSuitableDisplayName(data) };
        QString dev { data[DeviceProperty::kDevice].toString() };
        QAction *act { menu->addAction(label) };
        act->setData(dev);
        QString actId { ActionId::kStagePrex + dev };
        act->setProperty(ActionPropertyKey::kActionID, actId);
        predicateAction.insert(actId, act);
    }
}

bool SendToDiscMenuScenePrivate::disbaleWoringDevAction(QAction *act)
{
    Q_ASSERT(act);
    QString &&dev { act->data().toString() };
    if (DeviceUtils::isWorkingOpticalDiscDev(dev)) {
        act->setEnabled(false);
        return true;
    }

    return false;
}

void SendToDiscMenuScenePrivate::addToSendto(QMenu *menu)
{
    if (!menu)
        return;

    if (destDeviceDataGroup.isEmpty())
        return;

    QAction *sendTo { nullptr };
    for (auto act : menu->actions()) {
        if (act->property(ActionPropertyKey::kActionID).toString() == "send-to") {
            sendTo = act;
            break;
        }
    }

    if (!sendTo) {
        fmWarning() << "cannot find sendTo menu!!";
        return;
    }

    auto subMenu = sendTo->menu();
    if (!subMenu)
        return;

    int i = 0;
    for (const auto &dev : destDeviceDataGroup) {
        auto label = DeviceUtils::convertSuitableDisplayName(dev);
        auto act = subMenu->addAction(label);
        const QString &&actId = QString("%1%2").arg(ActionId::kSendToOptical).arg(i++);
        act->setProperty(ActionPropertyKey::kActionID, actId);
        act->setData(dev[DeviceProperty::kDevice].toString());
        predicateAction.insert(actId, act);
    }
}

AbstractMenuScene *SendToDiscMenuCreator::create()
{
    return new SendToDiscMenuScene();
}

SendToDiscMenuScene::SendToDiscMenuScene(QObject *parent)
    : AbstractMenuScene(parent),
      d(new SendToDiscMenuScenePrivate(this))
{
}

SendToDiscMenuScene::~SendToDiscMenuScene()
{
}

QString SendToDiscMenuScene::name() const
{
    return SendToDiscMenuCreator::name();
}

bool SendToDiscMenuScene::initialize(const QVariantHash &params)
{
    d->windowId = params.value(MenuParamKey::kWindowId).toULongLong();
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    if (!d->selectFiles.isEmpty())
        d->focusFile = d->selectFiles.first();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();
    d->predicateName.insert(ActionId::kStageKey, QObject::tr("Add to disc"));
    d->predicateName.insert(ActionId::kMountImageKey, QObject::tr("Mount"));

    const auto &tmpParams = dfmplugin_menu_util::menuPerfectParams(params);
    d->isDDEDesktopFileIncluded = tmpParams.value(MenuParamKey::kIsDDEDesktopFileIncluded, false).toBool();

    if (d->selectFiles.isEmpty())
        return false;
    d->initDestDevices();

    QUrl url(d->selectFiles.first());
    QList<QUrl> urls {};
    bool ok = UniversalUtils::urlsTransformToLocal(QList<QUrl>() << url, &urls);

    if (ok && !urls.isEmpty())
        url = urls.first();

    if (url.scheme() != Global::Scheme::kFile)
        return false;

    return AbstractMenuScene::initialize(params);
}

bool SendToDiscMenuScene::create(QMenu *parent)
{
    if (!parent)
        return false;

    if (d->isDDEDesktopFileIncluded)
        return AbstractMenuScene::create(parent);

    // stage file to disc
    if (!d->destDeviceDataGroup.isEmpty() || d->disableStage) {
        QAction *act { parent->addAction(d->predicateName[ActionId::kStageKey]) };
        act->setProperty(ActionPropertyKey::kActionID, ActionId::kStageKey);
        d->predicateAction.insert(ActionId::kStageKey, act);
        // use menu
        QMenu *stageMenu { new QMenu(parent) };
        d->addSubStageActions(stageMenu);
        if (stageMenu->actions().isEmpty())
            delete stageMenu;
        else
            act->setMenu(stageMenu);
    }

    d->addToSendto(parent);

    // mount image
    auto focusInfo { InfoFactory::create<FileInfo>(d->focusFile) };
    if (focusInfo) {
        static QSet<QString> mountable { "application/x-cd-image", "application/x-iso9660-image" };
        if (mountable.contains(focusInfo->nameOf(NameInfoType::kMimeTypeName))) {
            QAction *act { parent->addAction(d->predicateName[ActionId::kMountImageKey]) };
            act->setProperty(ActionPropertyKey::kActionID, ActionId::kMountImageKey);
            d->predicateAction.insert(ActionId::kMountImageKey, act);
        }
    }

    return AbstractMenuScene::create(parent);
}

void SendToDiscMenuScene::updateState(QMenu *parent)
{
    updateStageAction(parent);

    return AbstractMenuScene::updateState(parent);
}

bool SendToDiscMenuScene::triggered(QAction *action)
{
    if (!d->predicateAction.values().contains(action))
        return false;

    const QString &key { action->property(ActionPropertyKey::kActionID).toString() };
    if (key == ActionId::kStageKey || key.startsWith(ActionId::kStagePrex)
        || key.startsWith(ActionId::kSendToOptical)) {
        QString dev { action->data().toString() };
        if (DeviceUtils::isPWOpticalDiscDev(dev)) {
            d->actionPacketWriting(dev);
            return true;
        }
        d->actionStageFileForBurning(dev);
        return true;
    }

    if (key == ActionId::kMountImageKey) {
        d->actionMountImage();
        return true;
    }

    return AbstractMenuScene::triggered(action);
}

AbstractMenuScene *SendToDiscMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (!d->predicateAction.key(action).isEmpty())
        return const_cast<SendToDiscMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

void SendToDiscMenuScene::updateStageAction(QMenu *parent)
{
    auto actions { parent->actions() };

    QAction *stageAct { nullptr };

    for (auto act : actions) {
        QString &&id { act->property(ActionPropertyKey::kActionID).toString() };
        if (id == ActionId::kStageKey)
            stageAct = act;
    }

    if (!stageAct)
        return;

    // hide action if contains dde desktop filef
    if (d->isDDEDesktopFileIncluded) {
        stageAct->setVisible(false);
        return;
    }

    // disbale action if dev working
    if (d->destDeviceDataGroup.size() == 1 && d->disbaleWoringDevAction(stageAct))
        return;
    if (d->destDeviceDataGroup.size() > 1 && stageAct->menu()) {
        auto &&actions { stageAct->menu()->actions() };
        for (int i = 0; i != actions.size(); ++i) {
            if (d->disbaleWoringDevAction(actions[i]))
                return;
        }
    }

    // disable action in self disc
    if (d->disableStage) {
        stageAct->setEnabled(false);
    }

    if (!BurnHelper::isBurnEnabled()) {
        std::for_each(d->predicateAction.begin(), d->predicateAction.end(), [](QAction *act) {
            const auto &&id = act->property(ActionPropertyKey::kActionID).toString();
            if (id.startsWith(ActionId::kSendToOptical) || id.startsWith(ActionId::kStageKey) || id.startsWith(ActionId::kStagePrex))
                act->setEnabled(false);
        });
    }
}
