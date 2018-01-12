/*
 * Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
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
#include "operatorrevocation.h"
#include "dfmeventdispatcher.h"
#include "models/trashfileinfo.h"

DFM_BEGIN_NAMESPACE

class OperatorRevocationPrivate : public OperatorRevocation
{public: OperatorRevocationPrivate(){}};
Q_GLOBAL_STATIC(OperatorRevocationPrivate, _dfm_or)

OperatorRevocation *OperatorRevocation::instance()
{
    return _dfm_or;
}

bool OperatorRevocation::fmEvent(const QSharedPointer<DFMEvent> &event, QVariant *resultData)
{
    Q_UNUSED(event)
    Q_UNUSED(resultData)

    switch ((int)event->type()) {
    case DFMEvent::SaveOperator:
        if (event->property("_dfm_is_revocaion_event").toBool())
            return true;

        operatorStack.push(*event.data());
        break;
    case DFMEvent::Revocation: {
        if (operatorStack.isEmpty())
            return true;

        const DFMEvent &e = operatorStack.pop();
        const QSharedPointer<DFMEvent> new_event = dfmevent_cast<DFMSaveOperatorEvent>(e).event();

        new_event->setProperty("_dfm_is_revocaion_event", true);

        DFMEventDispatcher::instance()->processEvent(new_event);
        break;
    }
    case DFMEvent::CleanSaveOperator:
        operatorStack.clear();
        break;
    default:
        break;
    }

    return false;
}

OperatorRevocation::OperatorRevocation()
{

}

DFM_END_NAMESPACE
