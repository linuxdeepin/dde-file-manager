/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
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
#include "dfmwindoweventhandler.h"

DFMWindowEventHandler::DFMWindowEventHandler()
{

}

DFMWindowEventHandler::~DFMWindowEventHandler()
{

}

void DFMWindowEventHandler::event(const DFMEventPointer &event) {

    if (event->type() < DFMEvent::WindowEvent
            || event->type() >= DFMEvent::CustomEvent) {
        return;
    }

    DFMWindowEventPointer e = qSharedPointerDynamicCast<DFMWindowEvent>(event);
    if (e.isNull()) return;

    switch (event->type()) {
    case DFMWindowEvent::ChangeCurrentUrl:
        changeCurrentUrl(e);
        break;

    case DFMWindowEvent::OpenNewWindow:
        openNewWindow(e);
        break;

    case DFMWindowEvent::OpenNewTab:
        openNewTab(e);
        break;
    case DFMWindowEvent::OpenUrl:
        openUrl(e);
        break;
    case DFMWindowEvent::MenuAction:
        menuAction(e);
        break;
    case DFMWindowEvent::NavBack:
        navBack(e);
        break;
    case DFMWindowEvent::NavForward:
        navForward(e);
        break;
    case DFMWindowEvent::SaveOperator:
        saveOperator(e);
        break;
    case DFMWindowEvent::CleanSaveOperator:
        cleanSaveOperator(e);
        break;
    case DFMWindowEvent::Revocation:
        revocation(e);
        break;
    default:
        qWarning() << "DFMWindowEvent to process event is unknown type";
        break;
    }
}

void DFMWindowEventHandler::changeCurrentUrl(const DFMWindowEventPointer &event)
{
    qInfo() << Q_FUNC_INFO;
}

void DFMWindowEventHandler::openNewWindow(const DFMWindowEventPointer &event)
{
    qInfo() << Q_FUNC_INFO;
}

void DFMWindowEventHandler::openNewTab(const DFMWindowEventPointer &event)
{
    qInfo() << Q_FUNC_INFO;
}

void DFMWindowEventHandler::openUrl(const DFMWindowEventPointer &event)
{
    qInfo() << Q_FUNC_INFO;
}

void DFMWindowEventHandler::menuAction(const DFMWindowEventPointer &event)
{
    qInfo() << Q_FUNC_INFO;
}

void DFMWindowEventHandler::navBack(const DFMWindowEventPointer &event)
{
    qInfo() << Q_FUNC_INFO;
}

void DFMWindowEventHandler::navForward(const DFMWindowEventPointer &event)
{
    qInfo() << Q_FUNC_INFO;
}

void DFMWindowEventHandler::saveOperator(const DFMWindowEventPointer &event)
{
    qInfo() << Q_FUNC_INFO;
}

void DFMWindowEventHandler::cleanSaveOperator(const DFMWindowEventPointer &event)
{
    qInfo() << Q_FUNC_INFO;
}

void DFMWindowEventHandler::revocation(const DFMWindowEventPointer &event) {
    qInfo() << Q_FUNC_INFO;
}
