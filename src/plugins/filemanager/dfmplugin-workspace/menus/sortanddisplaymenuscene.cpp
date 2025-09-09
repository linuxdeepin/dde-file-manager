// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sortanddisplaymenuscene.h"
#include "sortanddisplaymenuscene_p.h"
#include "workspacemenu_defines.h"
#include "views/fileview.h"
#include "models/fileviewmodel.h"
#include "utils/workspacehelper.h"
#include "events/workspaceeventcaller.h"

#include <dfm-base/dfm_log_defines.h>
#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <dfm-framework/dpf.h>

#include <QMenu>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
using namespace dfmplugin_workspace;
using namespace GlobalDConfDefines::ConfigPath;
using namespace GlobalDConfDefines::BaseConfig;

AbstractMenuScene *SortAndDisplayMenuCreator::create()
{
    fmDebug() << "Creating SortAndDisplayMenuScene instance";
    return new SortAndDisplayMenuScene();
}

SortAndDisplayMenuScene::SortAndDisplayMenuScene(QObject *parent)
    : AbstractMenuScene(parent), d(new SortAndDisplayMenuScenePrivate(this))
{
    fmDebug() << "SortAndDisplayMenuScene initialized";
    d->predicateName[ActionID::kSortBy] = tr("Sort by");
    d->predicateName[ActionID::kGroupBy] = tr("Group by");
    d->predicateName[ActionID::kDisplayAs] = tr("Display as");

    // 排序子菜单
    d->predicateName[ActionID::kSrtName] = tr("Name");
    d->predicateName[ActionID::kSrtTimeModified] = tr("Time modified");
    d->predicateName[ActionID::kSrtTimeCreated] = tr("Time created");
    d->predicateName[ActionID::kSrtSize] = tr("Size");
    d->predicateName[ActionID::kSrtType] = tr("Type");

    // 分组子菜单
    d->predicateName[ActionID::kGroupByNone] = tr("None");
    d->predicateName[ActionID::kGroupByName] = tr("Name");
    d->predicateName[ActionID::kGroupByModified] = tr("Time modified");
    d->predicateName[ActionID::kGroupByCreated] = tr("Time created");
    d->predicateName[ActionID::kGroupBySize] = tr("Size");
    d->predicateName[ActionID::kGroupByType] = tr("Type");

    // 显示子菜单
    d->predicateName[ActionID::kDisplayIcon] = tr("Icon");
    d->predicateName[ActionID::kDisplayList] = tr("List");
    d->predicateName[ActionID::kDisplayTree] = tr("Tree");
}

SortAndDisplayMenuScene::~SortAndDisplayMenuScene()
{
    fmDebug() << "SortAndDisplayMenuScene destroyed";
}

QString SortAndDisplayMenuScene::name() const
{
    return SortAndDisplayMenuCreator::name();
}

bool SortAndDisplayMenuScene::initialize(const QVariantHash &params)
{
    d->windowId = params.value(MenuParamKey::kWindowId).toULongLong();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();

    fmDebug() << "Initializing SortAndDisplayMenuScene - windowId:" << d->windowId
              << "isEmptyArea:" << d->isEmptyArea;

    if (d->isEmptyArea) {
        fmDebug() << "SortAndDisplayMenuScene initialization successful for empty area";
        return AbstractMenuScene::initialize(params);
    }

    fmDebug() << "SortAndDisplayMenuScene initialization skipped - not empty area";
    return false;
}

AbstractMenuScene *SortAndDisplayMenuScene::scene(QAction *action) const
{
    if (action == nullptr) {
        fmDebug() << "Cannot find scene: action is null";
        return nullptr;
    }

    if (!d->predicateAction.key(action).isEmpty()) {
        fmDebug() << "Action belongs to SortAndDisplayMenuScene";
        return const_cast<SortAndDisplayMenuScene *>(this);
    }

    return AbstractMenuScene::scene(action);
}

bool SortAndDisplayMenuScene::create(QMenu *parent)
{
    if (!parent) {
        fmWarning() << "Cannot create SortAndDisplayMenuScene: parent menu is null";
        return false;
    }

    fmDebug() << "Creating sort and display menu";
    d->view = qobject_cast<FileView *>(parent->parent());
    d->createEmptyMenu(parent);
    return AbstractMenuScene::create(parent);
}

void SortAndDisplayMenuScene::updateState(QMenu *parent)
{
    fmDebug() << "Updating sort and display menu state";
    d->updateEmptyAreaActionState();
    AbstractMenuScene::updateState(parent);
}

bool SortAndDisplayMenuScene::triggered(QAction *action)
{
    if (!action) {
        fmWarning() << "Cannot trigger action: action is null";
        return false;
    }

    if (!d->view) {
        fmWarning() << "Cannot trigger action: view is null";
        return false;
    }

    const auto &actionId = action->property(ActionPropertyKey::kActionID).toString();
    fmDebug() << "Action triggered in SortAndDisplayMenuScene:" << actionId;

    if (d->predicateAction.values().contains(action)) {
        // display as
        {
            // display as icon
            if (actionId == ActionID::kDisplayIcon) {
                fmInfo() << "Switching to icon view mode";
                WorkspaceEventCaller::sendViewModeChanged(d->windowId, DFMGLOBAL_NAMESPACE::ViewMode::kIconMode);
                return true;
            }

            // display as list
            if (actionId == ActionID::kDisplayList) {
                fmInfo() << "Switching to list view mode";
                WorkspaceEventCaller::sendViewModeChanged(d->windowId, DFMGLOBAL_NAMESPACE::ViewMode::kListMode);
                return true;
            }

            // display as tree
            if (actionId == ActionID::kDisplayTree) {
                fmInfo() << "Switching to tree view mode";
                WorkspaceEventCaller::sendViewModeChanged(d->windowId, DFMGLOBAL_NAMESPACE::ViewMode::kTreeMode);
                return true;
            }
        }

        // sort by
        {
            // sort by name
            if (actionId == ActionID::kSrtName) {
                fmInfo() << "Sorting by name";
                d->sortByRole(Global::ItemRoles::kItemFileDisplayNameRole);
                return true;
            }

            // sort by time modified
            if (actionId == ActionID::kSrtTimeModified) {
                fmInfo() << "Sorting by time modified";
                d->sortByRole(Global::ItemRoles::kItemFileLastModifiedRole);
                return true;
            }

            // sort by time created
            if (actionId == ActionID::kSrtTimeCreated) {
                fmInfo() << "Sorting by time created";
                d->sortByRole(Global::ItemRoles::kItemFileCreatedRole);
                return true;
            }

            // sort by size
            if (actionId == ActionID::kSrtSize) {
                fmInfo() << "Sorting by size";
                d->sortByRole(Global::ItemRoles::kItemFileSizeRole);
                return true;
            }

            // sort by type
            if (actionId == ActionID::kSrtType) {
                fmInfo() << "Sorting by type";
                d->sortByRole(Global::ItemRoles::kItemFileMimeTypeRole);
                return true;
            }
        }

        // group by
        {
            // group by none
            if (actionId == ActionID::kGroupByNone) {
                fmInfo() << "Setting group by none";
                d->groupByStrategy("NoGroupStrategy");
                return true;
            }

            // group by name
            if (actionId == ActionID::kGroupByName) {
                fmInfo() << "Grouping by name";
                d->groupByStrategy("Name");
                return true;
            }

            // group by time modified
            if (actionId == ActionID::kGroupByModified) {
                fmInfo() << "Grouping by time modified";
                d->groupByStrategy("ModifiedTime");
                return true;
            }

            // group by time created
            if (actionId == ActionID::kGroupByCreated) {
                fmInfo() << "Grouping by time created";
                d->groupByStrategy("CreatedTime");   // TimeStrategy handles both
                return true;
            }

            // group by size
            if (actionId == ActionID::kGroupBySize) {
                fmInfo() << "Grouping by size";
                d->groupByStrategy("Size");
                return true;
            }

            // group by type
            if (actionId == ActionID::kGroupByType) {
                fmInfo() << "Grouping by type";
                d->groupByStrategy("Type");
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
    if (!parent) {
        fmWarning() << "Cannot create empty menu: parent is null";
        return;
    }

    fmDebug() << "Creating empty area menu with sort and display options";

    QAction *tempAction = parent->addAction(predicateName.value(ActionID::kDisplayAs));
    tempAction->setMenu(addDisplayAsActions(parent));
    predicateAction[ActionID::kDisplayAs] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kDisplayAs));

    tempAction = parent->addAction(predicateName.value(ActionID::kSortBy));
    tempAction->setMenu(addSortByActions(parent));
    predicateAction[ActionID::kSortBy] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kSortBy));

    tempAction = parent->addAction(predicateName.value(ActionID::kGroupBy));
    tempAction->setMenu(addGroupByActions(parent));
    predicateAction[ActionID::kGroupBy] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kGroupBy));

    fmDebug() << "Empty area menu created with" << predicateAction.size() << "main actions";
}

QMenu *SortAndDisplayMenuScenePrivate::addSortByActions(QMenu *menu)
{
    fmDebug() << "Adding sort by actions to submenu";
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

    tempAction = subMenu->addAction(predicateName.value(ActionID::kSrtTimeCreated));
    tempAction->setCheckable(true);
    predicateAction[ActionID::kSrtTimeCreated] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kSrtTimeCreated));

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

QMenu *SortAndDisplayMenuScenePrivate::addGroupByActions(QMenu *menu)
{
    fmDebug() << "Adding group by actions to submenu";
    QMenu *subMenu = new QMenu(menu);

    // GroupBy
    QAction *tempAction = subMenu->addAction(predicateName.value(ActionID::kGroupByNone));
    tempAction->setCheckable(true);
    predicateAction[ActionID::kGroupByNone] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kGroupByNone));

    tempAction = subMenu->addAction(predicateName.value(ActionID::kGroupByName));
    tempAction->setCheckable(true);
    predicateAction[ActionID::kGroupByName] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kGroupByName));

    tempAction = subMenu->addAction(predicateName.value(ActionID::kGroupByModified));
    tempAction->setCheckable(true);
    predicateAction[ActionID::kGroupByModified] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kGroupByModified));

    tempAction = subMenu->addAction(predicateName.value(ActionID::kGroupByCreated));
    tempAction->setCheckable(true);
    predicateAction[ActionID::kGroupByCreated] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kGroupByCreated));

    tempAction = subMenu->addAction(predicateName.value(ActionID::kGroupBySize));
    tempAction->setCheckable(true);
    predicateAction[ActionID::kGroupBySize] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kGroupBySize));

    tempAction = subMenu->addAction(predicateName.value(ActionID::kGroupByType));
    tempAction->setCheckable(true);
    predicateAction[ActionID::kGroupByType] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kGroupByType));

    return subMenu;
}

QMenu *SortAndDisplayMenuScenePrivate::addDisplayAsActions(QMenu *menu)
{
    fmDebug() << "Adding display as actions to submenu";
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

    if (WorkspaceHelper::instance()->isViewModeSupported(view->rootUrl().scheme(), ViewMode::kTreeMode)
        && DConfigManager::instance()->value(kViewDConfName, kTreeViewEnable, true).toBool()) {
        tempAction = subMenu->addAction(predicateName.value(ActionID::kDisplayTree));
        tempAction->setCheckable(true);
        predicateAction[ActionID::kDisplayTree] = tempAction;
        tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kDisplayTree));
        fmDebug() << "Tree view mode enabled for scheme:" << view->rootUrl().scheme();
    }

    return subMenu;
}

void SortAndDisplayMenuScenePrivate::sortByRole(int role)
{
    auto itemRole = static_cast<Global::ItemRoles>(role);
    Qt::SortOrder order = view->model()->sortOrder();
    auto oldRole = view->model()->sortRole();
    order = oldRole != role               ? Qt::AscendingOrder
            : order == Qt::AscendingOrder ? Qt::DescendingOrder
                                          : Qt::AscendingOrder;

    fmDebug() << "Sorting by role:" << role << "order:" << (order == Qt::AscendingOrder ? "Ascending" : "Descending")
              << "old role:" << oldRole;

    view->setSort(itemRole, order);
}

void SortAndDisplayMenuScenePrivate::groupByStrategy(const QString &strategyName)
{
    QString currentStrategy = view->getGroupingStrategy();

    fmInfo() << "Setting grouping strategy:" << strategyName << "current strategy:" << currentStrategy;

    // Always set the strategy first (including "NoGroupStrategy")
    if (currentStrategy == strategyName) {
        // User clicked the same strategy - toggle sort order (unless it's NoGroupStrategy)
        if (strategyName != "NoGroupStrategy") {
            Qt::SortOrder currentOrder = view->model()->getGroupingOrder();
            Qt::SortOrder newOrder = (currentOrder == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;

            fmInfo() << "Toggling grouping order for strategy:" << strategyName
                     << "from" << (currentOrder == Qt::AscendingOrder ? "Ascending" : "Descending")
                     << "to" << (newOrder == Qt::AscendingOrder ? "Ascending" : "Descending");

            view->setGroupingOrder(newOrder);
        }
        // For NoGroupStrategy, clicking again just reconfirms the disable state
    } else {
        // Different strategy - set new strategy
        view->setGroupingStrategy(strategyName);
    }

    // Enable or disable grouping based on strategy (after setting the strategy)
    bool shouldEnable = (strategyName != "NoGroupStrategy");
    view->setGroupingEnabled(shouldEnable);

    fmInfo() << "Grouping" << (shouldEnable ? "enabled" : "disabled") << "with strategy:" << strategyName;
}

void SortAndDisplayMenuScenePrivate::updateEmptyAreaActionState()
{
    fmDebug() << "Updating empty area action state";
    using namespace Global;
    // sort  by
    auto role = static_cast<ItemRoles>(view->model()->sortRole());
    fmDebug() << "Current sort role:" << role;
    switch (role) {
    case kItemFileDisplayNameRole:
        predicateAction[ActionID::kSrtName]->setChecked(true);
        fmDebug() << "Set sort by name action as checked";
        break;
    case kItemFileLastModifiedRole:
        predicateAction[ActionID::kSrtTimeModified]->setChecked(true);
        fmDebug() << "Set sort by time modified action as checked";
        break;
    case kItemFileCreatedRole:
        predicateAction[ActionID::kSrtTimeCreated]->setChecked(true);
        fmDebug() << "Set sort by time created action as checked";
        break;
    case kItemFileSizeRole:
        predicateAction[ActionID::kSrtSize]->setChecked(true);
        fmDebug() << "Set sort by size action as checked";
        break;
    case kItemFileMimeTypeRole:
        predicateAction[ActionID::kSrtType]->setChecked(true);
        fmDebug() << "Set sort by type action as checked";
        break;
    default:
        fmDebug() << "Unknown sort role:" << role;
        break;
    }

    // group by
    QString currentStrategy = view->getGroupingStrategy();
    fmDebug() << "Current grouping strategy:" << currentStrategy;

    if (currentStrategy == "NoGroupStrategy") {
        predicateAction[ActionID::kGroupByNone]->setChecked(true);
        fmDebug() << "Set group by none action as checked";
    } else if (currentStrategy == "Name") {
        predicateAction[ActionID::kGroupByName]->setChecked(true);
        fmDebug() << "Set group by name action as checked";
    } else if (currentStrategy == "ModifiedTime") {
        predicateAction[ActionID::kGroupByModified]->setChecked(true);
        fmDebug() << "Set group by time modified action as checked";
    } else if (currentStrategy == "CreatedTime") {
        predicateAction[ActionID::kGroupByCreated]->setChecked(true);
        fmDebug() << "Set group by time created action as checked";
    } else if (currentStrategy == "Size") {
        predicateAction[ActionID::kGroupBySize]->setChecked(true);
        fmDebug() << "Set group by size action as checked";
    } else if (currentStrategy == "Type") {
        predicateAction[ActionID::kGroupByType]->setChecked(true);
        fmDebug() << "Set group by type action as checked";
    } else {
        fmDebug() << "Unknown grouping strategy:" << currentStrategy;
        // Default to none if unknown
        predicateAction[ActionID::kGroupByNone]->setChecked(true);
    }

    // display as
    auto mode = view->currentViewMode();
    fmDebug() << "Current view mode:" << static_cast<int>(mode);
    switch (mode) {
    case Global::ViewMode::kIconMode:
        predicateAction[ActionID::kDisplayIcon]->setChecked(true);
        fmDebug() << "Set display as icon action as checked";
        break;
    case Global::ViewMode::kListMode:
        predicateAction[ActionID::kDisplayList]->setChecked(true);
        fmDebug() << "Set display as list action as checked";
        break;
    case Global::ViewMode::kTreeMode:
        if (predicateAction.contains(ActionID::kDisplayTree)) {
            predicateAction[ActionID::kDisplayTree]->setChecked(true);
            fmDebug() << "Set display as tree action as checked";
        }
        break;
    default:
        fmDebug() << "Unknown view mode:" << static_cast<int>(mode);
        break;
    }
}
