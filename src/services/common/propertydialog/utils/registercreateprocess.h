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
#ifndef REGISTERCREATEPROCESS_H
#define REGISTERCREATEPROCESS_H

#include "propertydialog/property_defines.h"

#include <QWidget>

DSC_BEGIN_NAMESPACE
CPY_BEGIN_NAMESPACE
class RegisterCreateProcess
{
private:
    explicit RegisterCreateProcess();

public:
    static RegisterCreateProcess *instance();

public:
    //! 定义创建控件函数类型
    typedef std::function<QWidget *(const QUrl &url)> createControlViewFunc;

protected:
    //创建函数列表
    QHash<int, createControlViewFunc> constructList {};
    QHash<QString, createControlViewFunc> viewCreateFunctionHash {};
    QList<QString> propertyPathList {};

public:
    bool registerFunction(createControlViewFunc view, int index = -1, QString *errorString = nullptr);

    bool registerPropertyPathShowStyle(const QString &scheme);

    bool registerViewCreateFunction(createControlViewFunc view, const QString &scheme);

    bool isContains(const QUrl &url) const;

    QWidget *createWidget(const QUrl &url);

    QMap<int, QWidget *> createControlView(const QUrl &url);
};
CPY_END_NAMESPACE
DSC_END_NAMESPACE
#endif   // REGISTERCREATEPROCESS_H
