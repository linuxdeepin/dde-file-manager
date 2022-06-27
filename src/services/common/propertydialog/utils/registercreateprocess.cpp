/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#include "registercreateprocess.h"

using namespace dfm_service_common;
using namespace Property;

RegisterCreateProcess::RegisterCreateProcess()
{
}

RegisterCreateProcess *RegisterCreateProcess::instance()
{
    static RegisterCreateProcess reg;
    return &reg;
}

bool RegisterCreateProcess::registerControlExpand(createControlViewFunc view, int index)
{
    constructList.insert(index, view);
    return true;
}

void RegisterCreateProcess::unregisterControlExpand(int index)
{
    constructList.remove(index);
}

bool RegisterCreateProcess::registerCustomizePropertyView(createControlViewFunc view, const QString &scheme)
{
    if (!viewCreateFunctionHash.contains(scheme)) {
        viewCreateFunctionHash.insert(scheme, view);
        return true;
    }
    return false;
}

void RegisterCreateProcess::unregisterCustomizePropertyView(const QString &scheme)
{
    viewCreateFunctionHash.remove(scheme);
}

bool RegisterCreateProcess::registerBasicViewFiledExpand(basicViewFieldFunc func, const QString &scheme)
{
    if (!basicViewFieldFuncHash.contains(scheme)) {
        basicViewFieldFuncHash.insert(scheme, func);
        return true;
    }
    return false;
}

void RegisterCreateProcess::unregisterBasicViewFiledExpand(const QString &scheme)
{
    basicViewFieldFuncHash.remove(scheme);
}

bool RegisterCreateProcess::registerFilterControlField(const QString &scheme, FilePropertyControlFilter filter)
{
    if (!filePropertyFilterHash.contains(scheme)) {
        filePropertyFilterHash.insert(scheme, filter);
        return true;
    }

    QString error = "The current scheme has registered the associated construction class";
    qInfo() << error;
    return false;
}

void RegisterCreateProcess::unregisterFilterControlField(const QString &scheme)
{
    filePropertyFilterHash.remove(scheme);
}

bool RegisterCreateProcess::isContains(const QUrl &url) const
{
    if (propertyPathList.contains(url.scheme()))
        return true;
    return false;
}

QWidget *RegisterCreateProcess::createCustomizePropertyWidget(const QUrl &url)
{
    QWidget *widget = nullptr;
    for (createControlViewFunc &func : viewCreateFunctionHash.values()) {
        widget = func(url);
        if (widget)
            break;
    }
    return widget;
}

QMap<int, QWidget *> RegisterCreateProcess::createControlView(const QUrl &url)
{
    QMap<int, QWidget *> temp {};
    for (createControlViewFunc &func : constructList.values()) {
        int index = constructList.key(func);
        QWidget *g = func(url);
        if (g != nullptr)
            temp.insert(index, g);
    }
    return temp;
}

QMap<BasicExpandType, BasicExpand> RegisterCreateProcess::basicExpandField(const QUrl &url)
{
    QMap<BasicExpandType, BasicExpand> expandField {};
    basicViewFieldFunc func = basicViewFieldFuncHash.value(url.scheme());
    if (func != nullptr) {
        QMap<BasicExpandType, BasicExpand> field = func(url);
        if (!field.isEmpty())
            expandField.unite(field);
    }
    return expandField;
}

FilePropertyControlFilter RegisterCreateProcess::contorlFieldFilter(const QUrl &url)
{
    if (filePropertyFilterHash.isEmpty())
        return kNotFilter;
    else if (!filePropertyFilterHash.contains(url.scheme()))
        return kNotFilter;
    else
        return filePropertyFilterHash.value(url.scheme());
}
