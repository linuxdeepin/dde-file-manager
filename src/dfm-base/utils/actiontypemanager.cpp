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
#include "private/actiontypemanager_p.h"
#include "dfm-base/file/fileAction/private/defaultactiondata_p.h"

#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/file/fileAction/defaultactiondata.h"

#include <dfm-framework/framework.h>

#include <QUrl>
#include <QList>

DFMBASE_USE_NAMESPACE

ActionTypeManagerPrivate::ActionTypeManagerPrivate(ActionTypeManager *qq)
    : q(qq)
{
    defaultActions = new DefaultActionData(qq);
}

ActionTypeManagerPrivate::~ActionTypeManagerPrivate()
{
    actionTypes.clear();
    allActionText.clear();
    allActionTypes.clear();
    actionTypeToEventType.clear();
}

ActionTypeManager &ActionTypeManager::instance()
{
    static ActionTypeManager ins;
    return ins;
}

/*!
 * \brief ActionTypeManager::registerActionType: register Action by action name.
 *        The act name is not allowed to be the same.
 * \param actPredicate: the predicate of action.
 * \param actionText: the name of action.
 * \return return the action type, failed return -1.
 */
QPair<int, dfmbase::ActionDataContainer> ActionTypeManager::registerActionType(const QString &actPredicate, const QString &actionText)
{
    QPair<int, ActionDataContainer> temp;
    if (actPredicate.isEmpty() || actionText.isEmpty() || (d->tempactType > ActionType::kActMaxCustom)) {
        temp.first = -1;
        temp.second = ActionDataContainer();
        return temp;
    }

    if (d->allActionTypes.contains(actPredicate)) {
        temp.first = d->allActionTypes.value(actPredicate);
        temp.second = d->actionTypes.value(temp.first);
        return temp;
    }

    d->tempactType++;
    ActionDataContainer tempActData(d->tempactType, actionText);
    d->allActionText.insert(actPredicate, actionText);
    d->allActionTypes.insert(actPredicate, d->tempactType);
    d->actionTypes[d->tempactType] = tempActData;
    temp.first = d->tempactType;
    temp.second = tempActData;
    return temp;
}

bool ActionTypeManager::addSubActionType(ActionType parentType, const dfmbase::ActionDataContainer &actionData)
{
    if (d->defaultActions->contains(parentType)) {
        d->defaultActions->addSubActionType(parentType, actionData);
        return true;
    }

    if (d->actionTypes.contains(parentType)) {
        d->actionTypes[parentType].addChildrenActionsData(actionData);
        return true;
    }
    return false;
}

/*!
 * \brief ActionTypeManager::actionDataContainerByType: get the action data by action type.
 * \param actType: type of target action.
 * \param defaultAct: returns the default value when the fetch fails.
 * \return action data of target type
 */
dfmbase::ActionDataContainer ActionTypeManager::actionDataContainerByType(const int actType,
                                                                          const dfmbase::ActionDataContainer &defaultAct)
{
    dfmbase::ActionDataContainer tempData = d->defaultActions->actionDataContainerByType(actType);
    if (tempData.actionType() == kUnKnow && tempData.name().isEmpty())
        return d->actionTypes.value(actType, defaultAct);
    return tempData;
}

/*!
 * \brief ActionTypeManager::recycleActionType: recycle the action type when the action is discarded.
 * \param actType: type of action to be recycled
 */
void ActionTypeManager::recycleActionType(int actType)
{
    d->actionTypes.remove(actType);
}

/*!
 * \brief ActionTypeManager::actionGlobleEventBind: bind actType and eventType.
 * \param actType: type of action to be bind.
 * \param eventType: type of event to be bind.
 */
void ActionTypeManager::actionGlobleEventBind(ActionType actType, GlobalEventType eventType)
{
    d->actionTypeToEventType.insert(actType, eventType);
}

/*!
 * \brief ActionTypeManager::actionGlobleEventUnBind: unbind actType and eventType.
 * \param actType: type of action to be bind.
 */
void ActionTypeManager::actionGlobleEventUnBind(ActionType actType)
{
    d->actionTypeToEventType.remove(actType);
}

/*!
 * \brief ActionTypeManager::actionGlobleEvent: return the type of event by action's type.
 * \param actType: the type of action.
 * \return returns the target event type
 */
GlobalEventType ActionTypeManager::actionGlobalEvent(ActionType actType)
{
    // TODO(Lee):
    return d->actionTypeToEventType.value(actType, GlobalEventType::kUnknowType);
}

ActionTypeManager::ActionTypeManager(QObject *parent)
    : QObject(parent), d(new ActionTypeManagerPrivate(this))
{
}

ActionTypeManager::~ActionTypeManager()
{
}
