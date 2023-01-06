// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMVIEWFACTORY_H
#define DFMVIEWFACTORY_H

#include "dfmglobal.h"

DFM_BEGIN_NAMESPACE

class DFMBaseView;
class DFMViewFactory
{
public:
    static QStringList keys();
    static DFMBaseView *create(const QString &key);
    static bool viewIsSuitedWithUrl(const DFMBaseView *view, const DUrl &url);
};

DFM_END_NAMESPACE

#endif // DFMVIEWFACTORY_H
