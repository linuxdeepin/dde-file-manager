/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     shihua <tangtong@deepin.com>
 *
 * Maintainer: shihua <tangtong@deepin.com>
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
#ifndef DEVENTFILTER_H
#define DEVENTFILTER_H

#include "desktop.h"
#include "dapplication.h"
#include "view/canvasgridview.h"

#include <QEvent>
#include <QDebug>
#include <QWidget>
#include <QObject>

class DEventFilter final : public QObject
{
    Q_OBJECT
public:
    explicit DEventFilter(QObject *const parent = nullptr)
        : QObject{ parent } {}
    virtual ~DEventFilter() = default;


    DEventFilter(const DEventFilter &) = delete;
    DEventFilter &operator=(const DEventFilter &) = delete;

    virtual bool eventFilter(QObject *watched, QEvent *event) override
    {

//        switch (event->type()) {
//        case QEvent::MetaCall:
//        case QEvent::UpdateRequest:
//        case QEvent::FocusOut:
//        case QEvent::Paint:
//        case QEvent::ActivationChange:
//        case QEvent::ApplicationStateChange:
//        case QEvent::FocusAboutToChange:
//        case QEvent::Timer:
//        case QEvent::Leave:
//        case QEvent::MouseMove:
//        case QEvent::CursorChange:
//        case QEvent::WindowDeactivate:
//        case QEvent::ApplicationDeactivate:
//        case QEvent::Expose:
//        case QEvent::Enter:
//        case QEvent::DragMove:
//        case QEvent::Move:
//        case QEvent::Show:
//        case QEvent::ToolTip:
//        case QEvent::ChildAdded:
//        case QEvent::WindowActivate:
//        case QEvent::ApplicationActivate:
//        case QEvent::DynamicPropertyChange:
//        case QEvent::PaletteChange:
//        case QEvent::ChildPolished:
//        case QEvent::FutureCallOut:
//        case QEvent::MouseButtonRelease:
//            break;
//        case QEvent::DragLeave: {
//            break;
//        }
//        default:
//            qDebug() << watched << event->type();
//            break;
//        }

        CanvasGridView *view = qobject_cast<CanvasGridView *>(watched);
        if (view && event->type() == QEvent::DragEnter) {
            CanvasGridView::m_flag.store(true, std::memory_order_release);
        }

        bool expected = true;
        if (QEvent::Hide == event->type() && CanvasGridView::m_flag.compare_exchange_strong(expected, false, std::memory_order_release)) {
            view->fakeDropEvent();
        }
        return false;
    }
};



#endif // DEVENTFILTER_H
