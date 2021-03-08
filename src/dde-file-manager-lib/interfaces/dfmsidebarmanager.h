/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#pragma once

#include <QObject>

#include <functional>

#include "dfmglobal.h"

#define SIDEBAR_ID_INTERNAL_FALLBACK "__internal"

DFM_BEGIN_NAMESPACE

class DFMSideBarItemInterface;
class DFMSideBarManagerPrivate;
class DFMSideBarManager : public QObject
{
    Q_OBJECT

    typedef QString KeyType;
    typedef QPair<QString, std::function<DFMSideBarItemInterface*()>> SideBarInterfaceCreaterType;

public:
    static DFMSideBarManager *instance();

    template <class T>
    void dRegisterSideBarInterface(const QString &identifier) {
        if (isRegisted<T>(identifier))
            return;

        insertToCreatorHash(KeyType(identifier), SideBarInterfaceCreaterType(typeid(T).name(), [=] () {
            return static_cast<DFMSideBarItemInterface*>(new T());
        }));
    }

    bool isRegisted(const QString &scheme, const std::type_info &info) const;

    template <class T>
    bool isRegisted(const QString &scheme) const {
        return isRegisted(scheme, typeid(T));
    }

    DFMSideBarItemInterface * createByIdentifier(const QString& identifier);

private:
    explicit DFMSideBarManager(QObject *parent = nullptr);
    ~DFMSideBarManager();

    void insertToCreatorHash(const KeyType &type, const SideBarInterfaceCreaterType &creator);

    QScopedPointer<DFMSideBarManagerPrivate> d_private;

    Q_DECLARE_PRIVATE_D(d_private, DFMSideBarManager)
};

DFM_END_NAMESPACE
