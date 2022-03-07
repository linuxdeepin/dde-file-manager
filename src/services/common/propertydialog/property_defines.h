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
#ifndef PROPERTY_DEFINE_H
#define PROPERTY_DEFINE_H

#include "dfm_common_service_global.h"
#include "dfm-base/utils/finallyutil.h"

#include <QUrl>
#include <QIcon>
#include <QDebug>

DSC_BEGIN_NAMESPACE

namespace Property {
namespace EventType {
extern const int kEvokePropertyDialog;
}

namespace EventFilePropertyControlFilter {
extern const int kIconTitle;
extern const int kBasisInfo;
extern const int kPermission;
}

struct DeviceInfo
{
    QIcon icon;
    QUrl deviceUrl;
    QString deviceName;
    QString deviceType;
    QString fileSystem;
    qint64 totalCapacity;
    qint64 availableSpace;
};

class RegisterCreateMethod
{

private:
    RegisterCreateMethod() {}

public:
    static RegisterCreateMethod *ins()
    {
        static RegisterCreateMethod reg;
        return &reg;
    }

public:
    //! 定义创建控件函数类型
    typedef std::function<QWidget *(const QUrl &url)> createControlViewFunc;

protected:
    //创建函数列表
    QHash<int, createControlViewFunc> constructList {};
    QHash<QString, createControlViewFunc> viewCreateFunctionHash {};

public:
    bool registerFunction(createControlViewFunc view, int index = -1, QString *errorString = nullptr)
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

    bool registerViewCreateFunction(createControlViewFunc view, QString scheme)
    {
        if (!viewCreateFunctionHash.contains(scheme)) {
            viewCreateFunctionHash.insert(scheme, view);
        }
        return true;
    }

    QWidget *createWidget(const QUrl &url)
    {
        if (viewCreateFunctionHash.contains(url.scheme())) {
            createControlViewFunc func = viewCreateFunctionHash.value(url.scheme());
            return func(url);
        }
        return nullptr;
    }

    QMap<int, QWidget *> createControlView(const QUrl &url)
    {
        QMap<int, QWidget *> temp {};
        for (int i = 0; i < constructList.count(); ++i) {
            QWidget *g = constructList.value(i)(url);
            if (g != nullptr)
                temp.insert(i, g);
        }
        return temp;
    }
};
}
DSC_END_NAMESPACE
Q_DECLARE_METATYPE(DSC_NAMESPACE::Property::DeviceInfo)
#endif   //PROPERTY_DEFINE_H
