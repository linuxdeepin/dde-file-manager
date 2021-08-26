/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
#include "event/handler/dfmabstracteventhandler.h"
#include "event/protocol/dfmfileevent.h"

#include <iostream>

DFMAbstractEventHandler::DFMAbstractEventHandler()
{

}

DFMAbstractEventHandler::~DFMAbstractEventHandler()
{

}

bool DFMAbstractEventHandler::canAsynProcess()
{
    return false;
}

QString DFMAbstractEventHandler::scheme()
{
    return "";
}

void DFMAbstractEventHandler::event(const DFMEventPointer &event)
{
    Q_UNUSED(event);
}

