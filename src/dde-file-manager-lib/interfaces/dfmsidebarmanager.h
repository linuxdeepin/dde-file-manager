// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

    //NOTE [XIAO] 从PLGUIN中加载SideBarItemHandler
    void initSideBarItemHandlerFromPlugin();

    void insertToCreatorHash(const KeyType &type, const SideBarInterfaceCreaterType &creator);

    QScopedPointer<DFMSideBarManagerPrivate> d_private;

    Q_DECLARE_PRIVATE_D(d_private, DFMSideBarManager)
};

DFM_END_NAMESPACE
