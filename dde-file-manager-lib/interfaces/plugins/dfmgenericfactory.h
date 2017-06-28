/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
#ifndef DFMGENERICFACTORY_H
#define DFMGENERICFACTORY_H

#include "dfmglobal.h"

DFM_BEGIN_NAMESPACE

class DFMGenericFactory
{
public:
    DFMGenericFactory();

    static QStringList keys();
    static QObject *create(const QString &key);
    static QObjectList createAll(const QString &key);
};

DFM_END_NAMESPACE

#endif // DFMGENERICFACTORY_H
