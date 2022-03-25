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

DSC_USE_NAMESPACE
CPY_USE_NAMESPACE

RegisterCreateProcess::RegisterCreateProcess()
{
}

RegisterCreateProcess *RegisterCreateProcess::instance()
{
    static RegisterCreateProcess reg;
    return &reg;
}

bool RegisterCreateProcess::registerFunction(RegisterCreateProcess::createControlViewFunc view, int index, QString *errorString)
{
    QString error;
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { if (errorString) *errorString = error; });

    if (constructList.keys().contains(index)) {
        error = QObject::tr("The current index has registered "
                            "the associated construction class");
        qInfo() << error;
        return false;
    }

    constructList.insert(index, view);
    finally.dismiss();
    return true;
}

bool RegisterCreateProcess::registerPropertyPathShowStyle(const QString &scheme)
{
    if (!propertyPathList.contains(scheme)) {
        propertyPathList.append(scheme);
        return true;
    }
    return false;
}

bool RegisterCreateProcess::registerViewCreateFunction(RegisterCreateProcess::createControlViewFunc view, const QString &scheme)
{
    if (!viewCreateFunctionHash.contains(scheme)) {
        viewCreateFunctionHash.insert(scheme, view);
        return true;
    }
    return false;
}

bool RegisterCreateProcess::registerBasicViewExpand(RegisterCreateProcess::basicViewFieldFunc func, const QString &scheme)
{
    if (!basicViewFieldFuncHash.contains(scheme)) {
        basicViewFieldFuncHash.insert(scheme, func);
        return true;
    }
    return false;
}

bool RegisterCreateProcess::isContains(const QUrl &url) const
{
    if (propertyPathList.contains(url.scheme()))
        return true;
    return false;
}

QWidget *RegisterCreateProcess::createWidget(const QUrl &url)
{
    QWidget *widget = nullptr;
    for (createControlViewFunc func : viewCreateFunctionHash.values()) {
        widget = func(url);
        if (widget)
            break;
    }
    return widget;
}

QMap<int, QWidget *> RegisterCreateProcess::createControlView(const QUrl &url)
{
    QMap<int, QWidget *> temp {};
    for (int i = 0; i < constructList.count(); ++i) {
        QWidget *g = constructList.value(i)(url);
        if (g != nullptr)
            temp.insert(i, g);
    }
    return temp;
}

QList<QMap<QString, QString>> RegisterCreateProcess::basicExpandField(const QUrl &url)
{
    QList<QMap<QString, QString>> expandField {};
    for (basicViewFieldFunc func : basicViewFieldFuncHash.values()) {
        QMap<QString, QString> field = func(url);
        if (!field.isEmpty())
            expandField.append(field);
    }
    return expandField;
}
