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
#include "actiondatacontainer.h"

DFMBASE_BEGIN_NAMESPACE

ActionDataContainer::ActionDataContainer()
{
}

ActionDataContainer::ActionDataContainer(int type, const QString &name)
    : actType(type), actionName(name)
{
}

ActionDataContainer::ActionDataContainer(const ActionDataContainer &other)
    : actType(other.actType),
      actionName(other.actionName),
      actionIcon(other.actionIcon),
      actionCommand(other.actionCommand),
      actionData(other.actionData),
      subActionsData(other.subActionsData),
      actionTriggeredFunc(other.actionTriggeredFunc),
      actionDestroyedFunc(other.actionDestroyedFunc)
{
}

int ActionDataContainer::actionType() const
{
    return actType;
}

QString ActionDataContainer::name() const
{
    return actionName;
}

void ActionDataContainer::setName(const QString &name)
{
    actionName = name;
}

QIcon ActionDataContainer::icon() const
{
    return actionIcon;
}

void ActionDataContainer::setIcon(const QIcon &icon)
{
    actionIcon = icon;
}

QVariant ActionDataContainer::data()
{
    return actionData;
}

QVector<ActionDataContainer> ActionDataContainer::childrenActionsData() const
{
    return subActionsData;
}

void ActionDataContainer::setData(const QVariant &var)
{
    actionData = var;
}

void ActionDataContainer::addChildrenActionsData(const ActionDataContainer &actData)
{
    subActionsData.append(actData);
}

void ActionDataContainer::setChildrenActionsData(const QVector<ActionDataContainer> &actDataLst)
{
    subActionsData = std::move(actDataLst);
}

TriggeredFunc ActionDataContainer::triggeredFunc()
{
    return actionTriggeredFunc;
}

void ActionDataContainer::registerTriggeredFunc(TriggeredFunc func)
{
    actionTriggeredFunc = func;
}

DestroyedFunc ActionDataContainer::destroyedFunc()
{
    return actionDestroyedFunc;
}

void ActionDataContainer::registerdestroyedFunc(DestroyedFunc func)
{
    actionDestroyedFunc = func;
}

DFMBASE_END_NAMESPACE
