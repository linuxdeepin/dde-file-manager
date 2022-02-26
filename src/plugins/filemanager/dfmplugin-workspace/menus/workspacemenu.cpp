/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "workspacemenu.h"
#include "views/fileview.h"
#include "models/fileviewmodel.h"
#include "models/filesortfilterproxymodel.h"
#include "utils/fileoperaterhelper.h"

#include "dfm-base/dfm_actiontype_defines.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/interfaces/abstractfileactions.h"
#include "dfm-base/utils/actiontypemanager.h"
#include "dfm-framework/framework.h"

#include <QMenu>

DPWORKSPACE_BEGIN_NAMESPACE
namespace MenuScene {
extern const char *const kWorkspaceMenu = "workspace-menu";
}   // namespace MenuScene
DPWORKSPACE_END_NAMESPACE

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
DPWORKSPACE_USE_NAMESPACE

WorkspaceMenu::WorkspaceMenu(QObject *parent)
    : AbstractMenu(parent)
{
}

QMenu *WorkspaceMenu::build(QWidget *parent,
                            AbstractMenu::MenuMode mode,
                            const QUrl &rootUrl,
                            const QUrl &foucsUrl,
                            const QList<QUrl> &selected,
                            QVariant customData)
{
    Q_UNUSED(customData)

    view = qobject_cast<FileView *>(parent);

    QMenu *menu = new QMenu(parent);

    switch (mode) {
    case AbstractMenu::MenuMode::kEmpty:
        assemblesEmptyAreaActions(menu, rootUrl);
        break;
    case AbstractMenu::MenuMode::kNormal:
        assemblesNormalActions(menu, rootUrl, foucsUrl, selected);
        break;
    }

    return menu;
}

void WorkspaceMenu::actionBusiness(QAction *act)
{
    auto actType = act->data().toInt();

    switch (actType) {
    case ActionType::kActSelectAll:
        view->selectAll();
        break;
    case ActionType::kActDisplayAsIcon:
        view->setViewMode(Global::ViewMode::kIconMode);
        break;
    case ActionType::kActDisplayAsList:
        view->setViewMode(Global::ViewMode::kListMode);
        break;
    case ActionType::kActName:
    case ActionType::kActSize:
    case ActionType::kActType:
    case ActionType::kActLastModifiedDate: {
        FileViewItem::Roles role = static_cast<FileViewItem::Roles>(getRoleByActionType(static_cast<ActionType>(actType)));
        Qt::SortOrder order = view->proxyModel()->sortOrder();
        int column = view->model()->getColumnByRole(role);

        view->proxyModel()->setSortRole(role);
        view->proxyModel()->sort(column, order);

        break;
    }
    case kActNewFolder:
        FileOperaterHelperIns->touchFolder(view);
        break;
    case kActNewText:
        FileOperaterHelperIns->touchFiles(view, CreateFileType::kCreateFileTypeText);
        break;
    case kActNewWord:
        FileOperaterHelperIns->touchFiles(view, CreateFileType::kCreateFileTypeWord);
        break;
    case kActNewExcel:
        FileOperaterHelperIns->touchFiles(view, CreateFileType::kCreateFileTypeExcel);
        break;
    case kActNewPowerpoint:
        FileOperaterHelperIns->touchFiles(view, CreateFileType::kCreateFileTypePowerpoint);
        break;
    case kActCut:
        FileOperaterHelperIns->cutFiles(view);
        break;
    case kActCopy:
        FileOperaterHelperIns->copyFiles(view);
        break;
    case kActPaste:
        FileOperaterHelperIns->pasteFiles(view);
        break;
    case kActDelete:
        FileOperaterHelperIns->deleteFiles(view);
        break;
    case ActionType::kActOpenInTerminal:
        FileOperaterHelperIns->openInTerminal(view);
        break;
    case ActionType::kActProperty:
        FileOperaterHelperIns->showFilesProperty(view);
        break;
    default:
        break;
    }
}

void WorkspaceMenu::assemblesEmptyAreaActions(QMenu *menu, const QUrl &rootUrl)
{
    QString errString;
    AbstractFileInfoPointer fileInfo = dfmbase::InfoFactory::create<AbstractFileInfo>(rootUrl, true, &errString);
    if (!fileInfo) {
        qWarning() << "create LocalFileInfo error: " << errString;
        return;
    }

    QVector<ActionType> typeList;

    AbstractFileActions *actionInfo = new AbstractFileActions(fileInfo);
    typeList = actionInfo->menuActionList(AbtMenuType::kSpaceArea);

    QVector<ActionDataContainer> dataList;

    transTypesToActionsData(typeList, dataList);
    assemblesSubActions(dataList);

    addActionsToMenu(menu, dataList);
}

void WorkspaceMenu::assemblesNormalActions(QMenu *menu, const QUrl &rootUrl, const QUrl &foucsUrl, const QList<QUrl> &selectList)
{
    QString errString;
    AbstractFileInfoPointer fileInfo = dfmbase::InfoFactory::create<AbstractFileInfo>(foucsUrl, true, &errString);
    if (!fileInfo) {
        qWarning() << "create LocalFileInfo error: " << errString;
        return;
    }

    QVector<ActionType> typeList;

    AbstractFileActions *actionInfo = new AbstractFileActions(fileInfo);

    if (selectList.count() > 1) {
        typeList = actionInfo->menuActionList(AbtMenuType::kMultiFiles);
    } else {
        typeList = actionInfo->menuActionList(AbtMenuType::kSingleFile);
    }

    QVector<ActionDataContainer> dataList;

    transTypesToActionsData(typeList, dataList);
    assemblesSubActions(dataList);

    addActionsToMenu(menu, dataList);
}

void WorkspaceMenu::addActionsToMenu(QMenu *menu, const QVector<ActionDataContainer> &dataList)
{
    if (dataList.isEmpty())
        return;

    // add action to menu
    for (const ActionDataContainer &data : dataList) {
        // Separator
        if (data.actionType() == ActionType::kActSeparator) {
            menu->addSeparator();
            continue;
        }

        if (data.name().isEmpty() || (-1 == data.actionType()))
            continue;

        QAction *act = new QAction(menu);
        act->setData(data.actionType());
        act->setText(data.name());
        if (!data.icon().isNull())
            act->setIcon(data.icon());

        QVector<ActionDataContainer> subDataList = data.childrenActionsData();
        if (!subDataList.isEmpty()) {
            QMenu *subMenu = new QMenu(menu);
            addActionsToMenu(subMenu, subDataList);
            act->setMenu(subMenu);
        }

        menu->addAction(act);
    }
}

void WorkspaceMenu::transTypesToActionsData(const QVector<ActionType> &typeList, QVector<ActionDataContainer> &dataList)
{
    for (const ActionType &type : typeList) {
        dataList.append(ActionTypeManager::instance().actionDataContainerByType(type));
    }
}

void WorkspaceMenu::assemblesSubActions(QVector<ActionDataContainer> &dataList)
{
    for (ActionDataContainer &data : dataList) {
        if (data.actionType() == ActionType::kActDisplayAs) {
            // TODO(liuyangming): check support view mode
            data.addChildrenActionsData(ActionTypeManager::instance().actionDataContainerByType(ActionType::kActDisplayAsList));
            data.addChildrenActionsData(ActionTypeManager::instance().actionDataContainerByType(ActionType::kActDisplayAsIcon));
        }

        if (data.actionType() == ActionType::kActSortBy) {
            // TODO(liuyangming): get support sort
            data.addChildrenActionsData(ActionTypeManager::instance().actionDataContainerByType(ActionType::kActName));
            data.addChildrenActionsData(ActionTypeManager::instance().actionDataContainerByType(ActionType::kActLastModifiedDate));
            data.addChildrenActionsData(ActionTypeManager::instance().actionDataContainerByType(ActionType::kActSize));
            data.addChildrenActionsData(ActionTypeManager::instance().actionDataContainerByType(ActionType::kActType));
        }
    }
}

int WorkspaceMenu::getRoleByActionType(const ActionType type) const
{
    switch (type) {
    case ActionType::kActName:
        return FileViewItem::kItemNameRole;
    case ActionType::kActLastModifiedDate:
        return FileViewItem::kItemFileLastModifiedRole;
    case ActionType::kActSize:
        return FileViewItem::kItemFileSizeRole;
    case ActionType::kActType:
        return FileViewItem::kItemFileMimeTypeRole;
    default:
        return FileViewItem::kItemNameRole;
    }
}
