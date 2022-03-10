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
#include "sharemenu.h"
#include "events/shareeventscaller.h"

#include "services/filemanager/windows/windowsservice.h"
#include "dfm-base/utils/actiontypemanager.h"

#include <QMenu>

DPMYSHARES_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

#define CreateContainerByType(type) ActionTypeManager::instance().actionDataContainerByType(type)

ShareMenu::ShareMenu(QObject *parent)
    : AbstractMenu(parent)
{
    initShareActions();
}

QMenu *ShareMenu::build(QWidget *parent, DFMBASE_NAMESPACE::AbstractMenu::MenuMode mode, const QUrl &rootUrl, const QUrl &focusedUrl, const QList<QUrl> &selected, QVariant customData)
{
    Q_UNUSED(rootUrl)
    Q_UNUSED(focusedUrl);
    Q_UNUSED(customData);

    selectedUrls = selected;
    winId = DSB_FM_NAMESPACE::WindowsService::service()->findWindowId(parent);

    if (mode == DFMBASE_NAMESPACE::AbstractMenu::MenuMode::kEmpty)
        return buildEmptyMenu(parent);
    else
        return buildFileMenu(selected, parent);
}

void ShareMenu::actionBusiness(QAction *act)
{
    int type = act->data().toInt();
    switch (type) {
    case kActOpen: {
        auto mode = selectedUrls.count() > 1 ? ShareEventsCaller::kOpenInNewWindow : ShareEventsCaller::kOpenInCurrentWindow;
        ShareEventsCaller::sendOpenDirs(winId, selectedUrls, mode);
    } break;
    case kActOpenInNewWindow:
        ShareEventsCaller::sendOpenDirs(0, selectedUrls, ShareEventsCaller::kOpenInNewWindow);
        break;
    case kActOpenInNewTab:
        ShareEventsCaller::sendOpenDirs(winId, selectedUrls, ShareEventsCaller::kOpenInNewTab);
        break;
    case kActProperty:
        ShareEventsCaller::sendShowProperty(selectedUrls);
        break;
    case kActDisplayAsList:
        ShareEventsCaller::sendSwitchDisplayMode(winId, DFMBASE_NAMESPACE::Global::ViewMode::kListMode);
        break;
    case kActDisplayAsIcon:
        ShareEventsCaller::sendSwitchDisplayMode(winId, DFMBASE_NAMESPACE::Global::ViewMode::kIconMode);
        break;
    case kActName:
    case kActLastModifiedDate:
    case kActSize:
    case kActType:
        break;
    }

    if (actionMap.contains(type)) {
        int customType = actionMap.value(type);
        switch (customType) {
        case kActCancelShare:
            if (selectedUrls.count() == 0)
                return;
            ShareEventsCaller::sendCancelSharing(selectedUrls.first());
            break;
        }
    }
}

QMenu *ShareMenu::buildEmptyMenu(QWidget *parent)
{
    auto containerDisplayMode = CreateContainerByType(kActDisplayAs);
    containerDisplayMode.addChildrenActionsData(CreateContainerByType(ActionType::kActDisplayAsList));
    containerDisplayMode.addChildrenActionsData(CreateContainerByType(ActionType::kActDisplayAsIcon));

    auto containerSortBy = CreateContainerByType(kActSortBy);
    containerSortBy.addChildrenActionsData(CreateContainerByType(ActionType::kActName));
    containerSortBy.addChildrenActionsData(CreateContainerByType(ActionType::kActLastModifiedDate));
    containerSortBy.addChildrenActionsData(CreateContainerByType(ActionType::kActSize));
    containerSortBy.addChildrenActionsData(CreateContainerByType(ActionType::kActType));

    QVector<ActionDataContainer> acts;
    acts << containerDisplayMode << CreateContainerByType(kActSeparator) << containerSortBy;
    return createMenuByContainer(acts, parent);
}

QMenu *ShareMenu::buildFileMenu(const QList<QUrl> &selected, QWidget *parent)
{
    QVector<ActionDataContainer> acts;
    acts.append(CreateContainerByType(kActOpen));

    if (selected.count() == 1) {
        acts.append(CreateContainerByType(kActOpenInNewWindow));
        acts.append(CreateContainerByType(kActOpenInNewTab));

        acts.append(CreateContainerByType(kActSeparator));
        acts.append(actContainers.value(kActCancelShare));
    }

    acts.append(CreateContainerByType(kActSeparator));
    acts.append(CreateContainerByType(kActProperty));

    return createMenuByContainer(acts, parent);
}

QMenu *ShareMenu::createMenuByContainer(const QVector<ActionDataContainer> &containers, QWidget *parent)
{
    QMenu *menu = new QMenu(parent);

    for (auto c : containers) {
        if (c.actionType() == kActSeparator) {
            menu->addSeparator();
            continue;
        }

        QAction *act = new QAction(c.name(), menu);
        act->setData(c.actionType());
        menu->addAction(act);

        if (c.childrenActionsData().count() > 0) {
            QMenu *subMenu = new QMenu(menu);
            for (auto cc : c.childrenActionsData()) {
                QAction *subAct = new QAction(cc.name(), subMenu);
                subAct->setData(cc.actionType());
                subMenu->addAction(subAct);
            }
            act->setMenu(subMenu);
        }
    }

    return menu;
}

void ShareMenu::initShareActions()
{
    auto pair = ActionTypeManager::instance().registerActionType("CancelSharing", tr("Cancel sharing"));
    actionMap.insert(pair.first, kActCancelShare);
    actContainers.insert(kActCancelShare, pair.second);
}
