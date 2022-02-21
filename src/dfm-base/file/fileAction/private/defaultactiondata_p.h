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
#ifndef DEFAULTACTIONDATA_P_H
#define DEFAULTACTIONDATA_P_H

#include "dfm-base/dfm_base_global.h"
#include "dfm-base/file/fileAction/defaultactiondata.h"

#include <QMap>
#include <QAction>

DFMBASE_BEGIN_NAMESPACE

class ActionDataContainer;
class DefaultActionData;
class DefaultActionDataPrivate
{
    friend class DefaultActionData;
    Q_DISABLE_COPY(DefaultActionDataPrivate)
public:
    explicit DefaultActionDataPrivate(DefaultActionData *qq = nullptr);
    virtual ~DefaultActionDataPrivate();

private:
    DefaultActionData *q { nullptr };

    // actionType---actionData
    using actionTypeData = QMap<int, ActionDataContainer>;
    actionTypeData allActionTypeToData;

    // actionPredicate---actionText
    using actionPredicateToText = QMap<QString, QString>;
    actionPredicateToText allActionPredicateToText;

    // actionPredicate--actionType
    using actionPredicateToTypes = QMap<QString, int>;
    actionPredicateToTypes allActionPredicateToTypes;

    QHash<ActionType, GlobalEventType> actionTypeToEventType;
};

DFMBASE_END_NAMESPACE

#endif   // DEFAULTACTIONDATA_P_H
