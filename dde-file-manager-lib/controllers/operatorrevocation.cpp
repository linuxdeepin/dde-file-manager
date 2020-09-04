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

    switch (static_cast<DFMEvent::Type>(event->type())) {
    case DFMEvent::SaveOperator: {
        DFMSaveOperatorEvent *e = static_cast<DFMSaveOperatorEvent*>(event.data());

        if (e->iniaiator() && e->iniaiator()->property("_dfm_is_revocaion_event").toBool())
            return true;

        //fix bug44556、44632文件多次删除、剪切、撤销出现撤销失败（根据产品需求，限制最多连续撤销两次）
        if (REVOCATION_TIMES == operatorStack.count()) {
            DFMEvent tmp = operatorStack.pop();
            operatorStack.clear();
            operatorStack.push(tmp);
        }
        operatorStack.push(*event.data());
        return true;
    }
    case DFMEvent::Revocation: {
        bool batch_mode = false;

        batch_revocation:

        if (operatorStack.isEmpty())
            return true;

        DFMSaveOperatorEvent e = dfmevent_cast<DFMSaveOperatorEvent>(operatorStack.pop());

        if (e.split()) {
            if (batch_mode) {
                return true;
            } else {
                batch_mode = true;
                goto batch_revocation;
            }
        }

        const QSharedPointer<DFMEvent> new_event = e.event();

        new_event->setProperty("_dfm_is_revocaion_event", true);

        if (e.async())
            DFMEventDispatcher::instance()->processEventAsync(new_event);
        else
            DFMEventDispatcher::instance()->processEvent(new_event);

        if (batch_mode)
            goto batch_revocation;

        return true;
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
