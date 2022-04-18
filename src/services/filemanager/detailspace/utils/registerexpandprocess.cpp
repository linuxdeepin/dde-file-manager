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

#include "registerexpandprocess.h"
#include "dfm-base/utils/finallyutil.h"

#include <QDebug>
#include <QUrl>

DSB_FM_USE_NAMESPACE
DTSP_USE_NAMESPACE
RegisterExpandProcess::RegisterExpandProcess()
{
}

RegisterExpandProcess *RegisterExpandProcess::instance()
{
    static RegisterExpandProcess process;
    return &process;
}

bool RegisterExpandProcess::registerControlExpand(createControlViewFunc view, int index)
{

    if (constructList.keys().contains(index)) {
        QString error = QObject::tr("The current index has registered "
                                    "the associated construction class");
        qInfo() << error;
        return false;
    }

    constructList.insert(index, view);
    return true;
}

void RegisterExpandProcess::unregisterControlExpand(int index)
{
    constructList.remove(index);
}

bool RegisterExpandProcess::registerBasicViewExpand(basicViewFieldFunc func, const QString &scheme)
{
    if (!basicViewFieldFuncHash.contains(scheme)) {
        basicViewFieldFuncHash.insert(scheme, func);
        return true;
    }

    QString error = QObject::tr("The current scheme has registered "
                                "the associated construction class");
    qInfo() << error;
    return false;
}

void RegisterExpandProcess::unregisterBasicViewExpand(const QString &scheme)
{
    basicViewFieldFuncHash.remove(scheme);
}

bool RegisterExpandProcess::registerFilterControlField(const QString &scheme, DetailFilterType filter)
{
    if (!detailFilterHash.contains(scheme)) {
        detailFilterHash.insert(scheme, filter);
        return true;
    }

    QString error = QObject::tr("The current scheme has registered "
                                "the associated construction class");
    qInfo() << error;
    return false;
}

void RegisterExpandProcess::unregisterFilterControlField(const QString &scheme)
{
    detailFilterHash.remove(scheme);
}

QMap<int, QWidget *> RegisterExpandProcess::createControlView(const QUrl &url)
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

QMap<BasicExpandType, BasicExpandMap> RegisterExpandProcess::createBasicExpandField(const QUrl &url)
{
    QMap<BasicExpandType, BasicExpandMap> expandField {};
    basicViewFieldFunc func = basicViewFieldFuncHash.value(url.scheme());
    if (func != nullptr) {
        QMap<BasicExpandType, BasicExpandMap> field = func(url);
        if (!field.isEmpty())
            expandField.unite(field);
    }
    return expandField;
}

DetailFilterType RegisterExpandProcess::contorlFieldFilter(const QUrl &url)
{
    if (detailFilterHash.isEmpty())
        return kNotFilter;
    else if (!detailFilterHash.contains(url.scheme()))
        return kNotFilter;
    else
        return detailFilterHash.value(url.scheme());
}
