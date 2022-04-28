/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#include "propertymenuscene.h"
#include "propertymenuscene_p.h"

#include "services/common/menu/menu_defines.h"
#include "plugins/common/dfmplugin-menu/menuscene/action_defines.h"
#include "services/common/propertydialog/property_defines.h"

#include <dfm-base/mimetype/mimesappsmanager.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/desktopfileinfo.h>
#include <dfm-base/mimetype/mimesappsmanager.h>
#include <dfm-base/utils/properties.h>
#include <dfm-framework/framework.h>

#include <QMenu>
#include <QVariant>

DFMBASE_USE_NAMESPACE
DSC_USE_NAMESPACE
DPPROPERTYDIALOG_USE_NAMESPACE

AbstractMenuScene *PropertyMenuCreator::create()
{
    return new PropertyMenuScene();
}

PropertyMenuScenePrivate::PropertyMenuScenePrivate(PropertyMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
    predicateName[dfmplugin_menu::ActionID::kProperty] = tr("Property");
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
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    if (!d->selectFiles.isEmpty())
        d->focusFile = d->selectFiles.first();
    d->onDesktop = params.value(MenuParamKey::kOnDesktop).toBool();

    if (!d->selectFiles.isEmpty() && !d->focusFile.isValid() && !d->currentDir.isValid()) {
        qDebug() << "menu scene:" << name() << " init failed." << d->selectFiles.isEmpty() << d->focusFile << d->currentDir;
        return false;
    }

    if (d->selectFiles.isEmpty() && d->currentDir.isValid()) {
        d->selectFiles << d->currentDir;
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

    QAction *tempAction = parent->addAction(d->predicateName.value(dfmplugin_menu::ActionID::kProperty));
    d->predicateAction[dfmplugin_menu::ActionID::kProperty] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, dfmplugin_menu::ActionID::kProperty);

    QList<QUrl> redirectedUrlList;
    for (const auto &fileUrl : d->selectFiles) {
        QString errString;
        auto fileInfo = DFMBASE_NAMESPACE::InfoFactory::create<AbstractFileInfo>(fileUrl, true, &errString);
        if (fileInfo.isNull()) {
            qDebug() << errString;
            continue;
        }
        redirectedUrlList << fileInfo->redirectedFileUrl();
    }

    return AbstractMenuScene::create(parent);
}

void PropertyMenuScene::updateState(QMenu *parent)
{
    if (!parent)
        return;

    // open with
    if (auto openWith = d->predicateAction.value(dfmplugin_menu::ActionID::kProperty)) {
    }

    AbstractMenuScene::updateState(parent);
}

bool PropertyMenuScene::triggered(QAction *action)
{
    Q_UNUSED(action)
    // TODO(Lee or others):
    if (!d->predicateAction.values().contains(action))
        return false;

    QString id = d->predicateAction.key(action);
    if (id == dfmplugin_menu::ActionID::kProperty) {
        dpfInstance.eventDispatcher().publish(DSC_NAMESPACE::Property::EventType::kEvokePropertyDialog,
                                              d->selectFiles);
    }

    return AbstractMenuScene::triggered(action);
}
