/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
#include "dfmfilecontrollerfactory.h"

QStringList DFMFileControllerFactory::keys()
{
    QStringList list{};
    return list;
}

DAbstractFileController *DFMFileControllerFactory::create(const QString &key)
{
   return nullptr;
}


