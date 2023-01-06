// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMCRUMBFACTORY_H
#define DFMCRUMBFACTORY_H

#include "dfmglobal.h"

DFM_BEGIN_NAMESPACE

class DFMCrumbInterface;
class DFMCrumbFactory
{
public:
    static QStringList keys();
    static DFMCrumbInterface *create(const QString &key);
};

DFM_END_NAMESPACE


#endif // DFMCRUMBFACTORY_H
