// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sortanddisplaymenuscene.h"
#include "sortanddisplaymenuscene_p.h"
#include "workspacemenu_defines.h"
#include "views/fileview.h"
#include "models/fileviewmodel.h"
#include "utils/workspacehelper.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <dfm-framework/dpf.h>

#include <QMenu>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_workspace;

AbstractMenuScene *SortAndDisplayMenuCreator::create()
{
    return new SortAndDisplayMenuScene();
}

SortAndDisplayMenuScene::SortAndDisplayMenuScene(QObject *parent)
    : AbstractMenuScene(parent), d(new SortAndDisplayMenuScenePrivate(this))
{
    d->predicateName[ActionID::kSortBy] = tr("Sort by");
    d->predicateName[ActionID::kDisplayAs] = tr("Display as");

    // 排序子菜单
    d->predicateName[ActionID::kSrtName] = tr("Name");
    d->predicateName[ActionID::kSrtTimeModified] = tr("Time modified");
    d->predicateName[ActionID::kSrtSize] = tr("Size");
    d->predicateName[ActionID::kSrtType] = tr("Type");

    // 显示子菜单
    d->predicateName[ActionID::kDisplayIcon] = tr("Icon");
    d->predicateName[ActionID::kDisplayList] = tr("List");
    d->predicateName[ActionID::kDisplayTree] = tr("Tree");
}

SortAndDisplayMenuScene::~SortAndDisplayMenuScene()
{
}

QString SortAndDisplayMenuScene::name() const
{
    return SortAndDisplayMenuCreator::name();
}

bool SortAndDisplayMenuScene::initialize(const QVariantHash &params)
{
    d->windowId = params.value(MenuParamKey::kWindowId).toULongLong();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();
    if (d->isEmptyArea)
        return AbstractMenuScene::initialize(params);
    return false;
}

AbstractMenuScene *SortAndDisplayMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (!d->predicateAction.key(action).isEmpty())
        return const_cast<SortAndDisplayMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

bool SortAndDisplayMenuScene::create(QMenu *parent)
{
    d->view = qobject_cast<FileView *>(parent->parent());
    d->createEmptyMenu(parent);
    return AbstractMenuScene::create(parent);
}

void SortAndDisplayMenuScene::updateState(QMenu *parent)
{
    d->updateEmptyAreaActionState();
    AbstractMenuScene::updateState(parent);
}

bool SortAndDisplayMenuScene::triggered(QAction *action)
{
    if (!d->view)
        return false;

    const auto &actionId = action->property(ActionPropertyKey::kActionID).toString();
    if (d->predicateAction.values().contains(action)) {
        // display as
        {
            // display as icon
            if (actionId == ActionID::kDisplayIcon) {
                dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::kSwitchViewMode, d->windowId, int(Global::ViewMode::kIconMode));
                return true;
            }

            // display as list
            if (actionId == ActionID::kDisplayList) {
                dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::kSwitchViewMode, d->windowId, int(Global::ViewMode::kListMode));
                return true;
            }

            // display as tree
            if (actionId == ActionID::kDisplayTree) {
                dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::kSwitchViewMode, d->windowId, int(Global::ViewMode::kTreeMode));
                return true;
            }
        }

        // sort by
        {
            // sort by name
            if (actionId == ActionID::kSrtName) {
                d->sortByRole(Global::ItemRoles::kItemFileDisplayNameRole);
                return true;
            }

            // sort by time modified
            if (actionId == ActionID::kSrtTimeModified) {
                d->sortByRole(Global::ItemRoles::kItemFileLastModifiedRole);
                return true;
            }

            // sort by size
            if (actionId == ActionID::kSrtSize) {
                d->sortByRole(Global::ItemRoles::kItemFileSizeRole);
                return true;
            }

            // sort by size
            if (actionId == ActionID::kSrtType) {
                d->sortByRole(Global::ItemRoles::kItemFileMimeTypeRole);
                return true;
            }
        }
    }

    return AbstractMenuScene::triggered(action);
}

SortAndDisplayMenuScenePrivate::SortAndDisplayMenuScenePrivate(AbstractMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
}

void SortAndDisplayMenuScenePrivate::createEmptyMenu(QMenu *parent)
{
    QAction *tempAction = parent->addAction(predicateName.value(ActionID::kDisplayAs));
    tempAction->setMenu(addDisplayAsActions(parent));
    predicateAction[ActionID::kDisplayAs] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kDisplayAs));

    tempAction = parent->addAction(predicateName.value(ActionID::kSortBy));
    tempAction->setMenu(addSortByActions(parent));
    predicateAction[ActionID::kSortBy] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kSortBy));
}

QMenu *SortAndDisplayMenuScenePrivate::addSortByActions(QMenu *menu)
{
    QMenu *subMenu = new QMenu(menu);

    // SortBy
    QAction *tempAction = subMenu->addAction(predicateName.value(ActionID::kSrtName));
    tempAction->setCheckable(true);
    predicateAction[ActionID::kSrtName] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kSrtName));

    tempAction = subMenu->addAction(predicateName.value(ActionID::kSrtTimeModified));
    tempAction->setCheckable(true);
    predicateAction[ActionID::kSrtTimeModified] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kSrtTimeModified));

    tempAction = subMenu->addAction(predicateName.value(ActionID::kSrtSize));
    tempAction->setCheckable(true);
    predicateAction[ActionID::kSrtSize] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kSrtSize));

    tempAction = subMenu->addAction(predicateName.value(ActionID::kSrtType));
    tempAction->setCheckable(true);
    predicateAction[ActionID::kSrtType] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kSrtType));

    return subMenu;
}

QMenu *SortAndDisplayMenuScenePrivate::addDisplayAsActions(QMenu *menu)
{
    QMenu *subMenu = new QMenu(menu);

    // DisplayAs
    QAction *tempAction = subMenu->addAction(predicateName.value(ActionID::kDisplayIcon));
    tempAction->setCheckable(true);
    predicateAction[ActionID::kDisplayIcon] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kDisplayIcon));

    tempAction = subMenu->addAction(predicateName.value(ActionID::kDisplayList));
    tempAction->setCheckable(true);
    predicateAction[ActionID::kDisplayList] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kDisplayList));

    if (WorkspaceHelper::instance()->supportTreeView(view->rootUrl().scheme())
            && DConfigManager::instance()->value(kViewDConfName, kTreeViewEnable, true).toBool()) {
        tempAction = subMenu->addAction(predicateName.value(ActionID::kDisplayTree));
        tempAction->setCheckable(true);
        predicateAction[ActionID::kDisplayTree] = tempAction;
        tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kDisplayTree));
    }

    return subMenu;
}

void SortAndDisplayMenuScenePrivate::sortByRole(int role)
{
    auto itemRole = static_cast<Global::ItemRoles>(role);
    Qt::SortOrder order = view->model()->sortOrder();

    view->setSort(itemRole, order == Qt::AscendingOrder ? Qt::DescendingOrder : Qt::AscendingOrder);
}

void SortAndDisplayMenuScenePrivate::updateEmptyAreaActionState()
{
    using namespace Global;
    // sort  by
    auto role = static_cast<ItemRoles>(view->model()->sortRole());
    switch (role) {
    case kItemFileDisplayNameRole:
        predicateAction[ActionID::kSrtName]->setChecked(true);
        break;
    case kItemFileLastModifiedRole:
        predicateAction[ActionID::kSrtTimeModified]->setChecked(true);
        break;
    case kItemFileSizeRole:
        predicateAction[ActionID::kSrtSize]->setChecked(true);
        break;
    case kItemFileMimeTypeRole:
        predicateAction[ActionID::kSrtType]->setChecked(true);
        break;
    default:
        break;
    }

    // display as
    auto mode = view->currentViewMode();
    switch (mode) {
    case Global::ViewMode::kIconMode:
        predicateAction[ActionID::kDisplayIcon]->setChecked(true);
        break;
    case Global::ViewMode::kListMode:
        predicateAction[ActionID::kDisplayList]->setChecked(true);
        break;
    case Global::ViewMode::kTreeMode:
        if (predicateAction.contains(ActionID::kDisplayTree))
            predicateAction[ActionID::kDisplayTree]->setChecked(true);
        break;
    default:
        break;
    }
}
