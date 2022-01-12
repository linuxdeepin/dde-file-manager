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
#include "dfm-base/interfaces/extendedcontrolview.h"
#include "dfm-base/utils/finallyutil.h"

#include <QUrl>
#include <QIcon>

DSC_BEGIN_NAMESPACE

struct DeviceInfo
{
    QIcon icon;
    QUrl deviceUrl;
    QString deviceName;
    QString deviceType;
    QString totalCapacity;
    QString fileSystem;
    QString availableSpace;
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
    typedef std::function<QWidget *(const QUrl &url)> createControlView;

protected:
    //创建函数列表
    QHash<int, createControlView> constructList {};
    QHash<int, QString> listclass {};

public:
    template<class CT>
    bool registerFunction(createControlView view, int index = -1, QString *errorString = nullptr)
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
        const QMetaObject *metaObject = CT().metaObject()->superClass();
        listclass.insert(index, metaObject->className());
        finally.dismiss();
        return true;
    }

    template<class CT>
    QMap<int, CT *> createView(const QUrl &url)
    {
        QMap<int, CT *> temp {};
        const QMetaObject *metaObject = CT().metaObject();
        if (listclass.values().contains(metaObject->className())) {
            QList<int> keylist = listclass.keys(metaObject->className());
            for (int i = 0; i < keylist.count(); ++i) {
                int index = keylist[i];
                CT *g = static_cast<CT *>(constructList.value(index)(url));
                if (g != nullptr)
                    temp.insert(index, g);
            }
            return temp;
        }
        return temp;
    }
};
DSC_END_NAMESPACE
Q_DECLARE_METATYPE(DSC_NAMESPACE::DeviceInfo)
#endif   //PROPERTY_DEFINE_H
