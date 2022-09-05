// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMFILEPREVIEWFACTORY_H
#define DFMFILEPREVIEWFACTORY_H

#include "dfmglobal.h"

DFM_BEGIN_NAMESPACE

class DFMFilePreview;
class DFMFilePreviewFactory
{
public:
    static QStringList keys();
    static DFMFilePreview *create(const QString &key);
    static bool isSuitedWithKey(const DFMFilePreview *view, const QString &key);
};

DFM_END_NAMESPACE

#endif // DFMFILEPREVIEWFACTORY_H
