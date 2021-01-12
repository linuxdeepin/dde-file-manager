/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
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
