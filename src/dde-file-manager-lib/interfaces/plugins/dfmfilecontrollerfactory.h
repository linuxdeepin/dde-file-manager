// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMFILECONTROLLERFACTORY_H
#define DFMFILECONTROLLERFACTORY_H

#include "dfmglobal.h"

class DAbstractFileController;

DFM_BEGIN_NAMESPACE

class DFMFileControllerFactory
{
public:
    static QStringList keys();
    static DAbstractFileController *create(const QString &key);
};

DFM_END_NAMESPACE

#endif // DFMFILECONTROLLERFACTORY_H
