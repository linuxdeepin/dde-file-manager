/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
