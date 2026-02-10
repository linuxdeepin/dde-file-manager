// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "propertymenuscene.h"
#include "propertymenuscene_p.h"
#include "events/propertyeventreceiver.h"

#include "plugins/common/dfmplugin-menu/menuscene/action_defines.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/mimetype/mimesappsmanager.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/desktopfileinfo.h>
#include <dfm-base/mimetype/mimesappsmanager.h>
#include <dfm-base/utils/properties.h>

#include <dfm-framework/event/event.h>

#include <QMenu>
#include <QVariant>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_propertydialog;

AbstractMenuScene *PropertyMenuCreator::create()
{
    return new PropertyMenuScene();
}

PropertyMenuScenePrivate::PropertyMenuScenePrivate(PropertyMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
    predicateName[PropertyActionId::kProperty] = tr("P&roperties");
}

void PropertyMenuScenePrivate::updateMenu(QMenu *menu)
{
    QList<QAction *> actions = menu->actions();
    if (!actions.isEmpty()) {
        QAction *propertyAct = nullptr;
        for (auto act : actions) {
            if (act->isSeparator())
                continue;

            if (predicateAction.values().contains(act)) {
                propertyAct = act;
                break;
            }
        }

        if (propertyAct) {
            actions.removeOne(propertyAct);
            QAction *act = menu->addSeparator();
            actions.append(act);
            actions.append(propertyAct);
            menu->addActions(actions);
        }
    }
}

PropertyMenuScene::PropertyMenuScene(QObject *parent)
    : AbstractMenuScene(parent), d(new PropertyMenuScenePrivate(this))
{
}

QString PropertyMenuScene::name() const
{
    return PropertyMenuCreator::name();
}

bool PropertyMenuScene::initialize(const QVariantHash &params)
{
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    if (!d->selectFiles.isEmpty())
        d->focusFile = d->selectFiles.first();
    d->onDesktop = params.value(MenuParamKey::kOnDesktop).toBool();

    if (!d->initializeParamsIsValid()) {
        fmWarning() << "menu scene:" << name() << " init failed." << d->selectFiles.isEmpty() << d->focusFile << d->currentDir;
        return false;
    }

    if (d->selectFiles.isEmpty() && d->currentDir.isValid()) {
        d->selectFiles << d->currentDir;
    }

    if (!d->isEmptyArea) {
        QString errString;
        d->focusFileInfo = DFMBASE_NAMESPACE::InfoFactory::create<FileInfo>(d->focusFile, Global::CreateFileInfoType::kCreateFileInfoAuto, &errString);
        if (d->focusFileInfo.isNull()) {
            fmDebug() << errString;
            return false;
        }
    }

    return AbstractMenuScene::initialize(params);
}

AbstractMenuScene *PropertyMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (d->predicateAction.values().contains(action))
        return const_cast<PropertyMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

bool PropertyMenuScene::create(QMenu *parent)
{
    if (d->selectFiles.isEmpty() && !d->focusFile.isValid())
        return false;

    QAction *tempAction = parent->addAction(d->predicateName.value(PropertyActionId::kProperty));
    d->predicateAction[PropertyActionId::kProperty] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, PropertyActionId::kProperty);

    return AbstractMenuScene::create(parent);
}

void PropertyMenuScene::updateState(QMenu *parent)
{
    if (!parent)
        return;

    if (auto pAction = d->predicateAction.value(PropertyActionId::kProperty)) {
        if (!d->isEmptyArea && !d->focusFileInfo->exists())
            pAction->setDisabled(true);
    }

    d->updateMenu(parent);

    AbstractMenuScene::updateState(parent);
}

bool PropertyMenuScene::triggered(QAction *action)
{
    Q_UNUSED(action)
    // TODO(Lee or others):
    if (!d->predicateAction.values().contains(action))
        return false;

    QString id = d->predicateAction.key(action);
    if (id == PropertyActionId::kProperty) {
        PropertyEventReceiver::instance()->handleShowPropertyDialog(d->selectFiles, QVariantHash());
        return true;
    }

    return AbstractMenuScene::triggered(action);
}
