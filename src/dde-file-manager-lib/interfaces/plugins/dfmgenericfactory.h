// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
