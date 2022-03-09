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
#ifndef ACTIONTYPEMANAGER_P_H
#define ACTIONTYPEMANAGER_P_H

#include "dfm-base/dfm_base_global.h"
#include "dfm-base/utils/actiontypemanager.h"

#include <QMap>
#include <QAction>

DFMBASE_BEGIN_NAMESPACE

class DefaultActionData;
class ActionDataContainer;
class ActionTypeManager;
class ActionTypeManagerPrivate
{
    friend class ActionTypeManager;
    Q_DISABLE_COPY(ActionTypeManagerPrivate)
public:
    explicit ActionTypeManagerPrivate(ActionTypeManager *qq = nullptr);
    virtual ~ActionTypeManagerPrivate();

private:
    ActionTypeManager *q { nullptr };

    using ActionTypeData = QMap<int, ActionDataContainer>;
    ActionTypeData actionTypes;

    using ActionPredicateToText = QMap<QString, QString>;
    ActionPredicateToText allActionText;

    using ActionPredicateToTypes = QMap<QString, int>;
    ActionPredicateToTypes allActionTypes;

    QHash<ActionType, GlobalEventType> actionTypeToEventType;

    int tempactType { ActionType::kActCustomBase };

    DefaultActionData *defaultActions;
};

DFMBASE_END_NAMESPACE
#endif   // ACTIONTYPEMANAGER_P_H
