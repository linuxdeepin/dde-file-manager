// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMFILEPREVIEWFACTORY_P_H
#define DFMFILEPREVIEWFACTORY_P_H

#include <QMap>

#include "dfmglobal.h"

DFM_BEGIN_NAMESPACE

class DFMFilePreview;
class DFMFilePreviewFactoryPrivate
{
public:
    static QMap<const DFMFilePreview*, int> previewToLoaderIndex;
};

DFM_END_NAMESPACE

#endif // DFMFILEPREVIEWFACTORY_P_H
