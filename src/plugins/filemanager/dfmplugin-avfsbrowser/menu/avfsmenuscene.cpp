// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "avfsmenuscene.h"
#include "private/avfsmenuscene_p.h"
#include "utils/avfsutils.h"
#include "events/avfseventhandler.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/base/schemefactory.h>

#include <dfm-framework/dpf.h>

#include <QDebug>
#include <QMenu>

using namespace dfmplugin_avfsbrowser;
DFMBASE_USE_NAMESPACE

static constexpr char k3rdSortBy[] { "sort-by" };
static constexpr char k3rdGroupBy[] { "group-by" };
static constexpr char k3rdDisplayAs[] { "display-as" };
static constexpr char k3rdOpenWith[] { "open-with" };

AvfsMenuScene::AvfsMenuScene(QObject *parent)
    : AbstractMenuScene((parent)), d(new AvfsMenuScenePrivate(this))
{
}

AvfsMenuScene::~AvfsMenuScene()
{
}

AbstractMenuScene *AvfsMenuSceneCreator::create()
{
    return new AvfsMenuScene();
}

QString AvfsMenuScene::name() const
{
    return AvfsMenuSceneCreator::name();
}

bool AvfsMenuScene::initialize(const QVariantHash &params)
{
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();
    d->windowId = params.value(MenuParamKey::kWindowId).toULongLong();

    d->showOpenWith = d->selectFiles.count() == 1;
    if (d->showOpenWith) {
        auto info = InfoFactory::create<FileInfo>(d->selectFiles.first());
        d->showOpenWith = info && !info->isAttributes(OptInfoType::kIsDir) && !AvfsUtils::isSupportedArchives(AvfsUtils::avfsUrlToLocal(d->selectFiles.first()));
    }

    return AbstractMenuScene::initialize(params);
}

bool AvfsMenuScene::create(QMenu *parent)
{
    using namespace AvfsMenuActionId;

    // create all subscene first and take all of the actions;
    AbstractMenuScene::create(parent);
    auto acts = parent->actions();
    for (auto act : acts) {
        auto key = act->property(ActionPropertyKey::kActionID).toString();
        if (key == k3rdDisplayAs)
            d->predicateAction[k3rdDisplayAs] = act;
        else if (key == k3rdSortBy)
            d->predicateAction[k3rdSortBy] = act;
        else if (key == k3rdGroupBy)
            d->predicateAction[k3rdGroupBy] = act;
        else if (key == k3rdOpenWith)
            d->predicateAction[k3rdOpenWith] = act;
        parent->removeAction(act);
    }

    if (!d->isEmptyArea) {
        auto act = parent->addAction(d->predicateName[kOpen]);
        act->setProperty(ActionPropertyKey::kActionID, kOpen);
        d->predicateAction[kOpen] = act;

        if (d->showOpenWith)
            parent->addAction(d->predicateAction[k3rdOpenWith]);
        parent->addSeparator();

        act = parent->addAction(d->predicateName[kCopy]);
        act->setProperty(ActionPropertyKey::kActionID, kCopy);
        d->predicateAction[kCopy] = act;

        parent->addSeparator();
    } else {
        parent->addAction(d->predicateAction[k3rdDisplayAs]);
        parent->addAction(d->predicateAction[k3rdSortBy]);
        parent->addAction(d->predicateAction[k3rdGroupBy]);
        parent->addSeparator();
    }

    auto act = parent->addAction(d->predicateName[kProperty]);
    act->setProperty(ActionPropertyKey::kActionID, kProperty);
    d->predicateAction[kProperty] = act;

    return true;
}

void AvfsMenuScene::updateState(QMenu *parent)
{
    AbstractMenuScene::updateState(parent);
}

bool AvfsMenuScene::triggered(QAction *action)
{
    using namespace AvfsMenuActionId;

    auto id = action->property(ActionPropertyKey::kActionID).toString();
    if (d->predicateName.contains(id) && d->predicateAction.values().contains(action)) {
        if (id == kCopy)
            AvfsEventHandler::instance()->writeToClipbord(d->windowId, d->selectFiles);
        else if (id == kOpen)
            AvfsEventHandler::instance()->hookOpenFiles(d->windowId, d->selectFiles);
        else if (id == kProperty)
            AvfsEventHandler::instance()->showProperty(d->selectFiles.isEmpty() ? QList<QUrl> { d->currentDir } : d->selectFiles);

        return true;
    }
    return AbstractMenuScene::triggered(action);
}

AbstractMenuScene *AvfsMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (!d->predicateAction.key(action).isEmpty())
        return const_cast<AvfsMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

AvfsMenuScenePrivate::AvfsMenuScenePrivate(AvfsMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
    using namespace AvfsMenuActionId;
    predicateName[kOpen] = tr("Open");
    predicateName[kCopy] = tr("Copy");
    predicateName[kProperty] = tr("Properties");
}
