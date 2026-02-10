// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/templatemenuscene_p.h"
#include "menuscene/action_defines.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/base/schemefactory.h>

#include <dfm-framework/event/event.h>

#include <QMenu>
#include <QUuid>

using namespace dfmplugin_menu;
DFMBASE_USE_NAMESPACE

AbstractMenuScene *TemplateMenuCreator::create()
{
    std::call_once(loadFlag, [this]() {
        templateMenu = new TemplateMenu(this);
        templateMenu->loadTemplateFile();
        fmInfo() << "template menus *.* loaded.";
    });

    return new TemplateMenuScene(templateMenu);
}

TemplateMenuScenePrivate::TemplateMenuScenePrivate(TemplateMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{

}

TemplateMenuScene::TemplateMenuScene(TemplateMenu *menu, QObject *parent)
    : AbstractMenuScene(parent),
      d(new TemplateMenuScenePrivate(this))
{
    Q_ASSERT(menu);
    d->templateActions = menu->actionList();
}

QString TemplateMenuScene::name() const
{
    return TemplateMenuCreator::name();
}

bool TemplateMenuScene::initialize(const QVariantHash &params)
{
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();
    if (!d->isEmptyArea)
        return false;

    d->windowId = params.value(MenuParamKey::kWindowId).toULongLong();
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    return AbstractMenuScene::initialize(params);
}

AbstractMenuScene *TemplateMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (d->predicateAction.values().contains(action))
        return const_cast<TemplateMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

bool TemplateMenuScene::create(QMenu *parent)
{
    // find new doc menu
    auto actions = parent->actions();
    auto actionIter = std::find_if(actions.begin(), actions.end(), [](const QAction *ac) {
        return ac->property(ActionPropertyKey::kActionID).toString() == dfmplugin_menu::ActionID::kNewDoc;
    });

    if (actionIter != actions.end()) {
        if (QMenu *pMenu = (*actionIter)->menu()) {
            for (const auto &act : d->templateActions) {
                QString id = QString("%1:%2").arg(TemplateMenuCreator::name())
                        .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
                d->predicateAction[id] = act;
                d->predicateName[id] = act->text();
                act->setProperty(ActionPropertyKey::kActionID, id);
                pMenu->addAction(act);
            }
        }
    }

    return AbstractMenuScene::create(parent);
}

void TemplateMenuScene::updateState(QMenu *parent)
{
    if (!parent)
        return;
    AbstractMenuScene::updateState(parent);
}

bool TemplateMenuScene::triggered(QAction *action)
{
    auto actionId = action->property(ActionPropertyKey::kActionID).toString();
    if (d->predicateAction.value(actionId) == action) {
        dpfSignalDispatcher->publish(GlobalEventType::kTouchFile,
                                     d->windowId,
                                     d->currentDir,
                                     QUrl::fromLocalFile(action->data().toString()),
                                     "");
        return true;
    }

    return AbstractMenuScene::triggered(action);
}
