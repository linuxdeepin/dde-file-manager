// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmsidebarmanager.h"

#include "interfaces/dfmsidebariteminterface.h"
#include "controllers/dfmsidebardefaultitemhandler.h"
#include "controllers/dfmsidebardeviceitemhandler.h"
#include "controllers/dfmsidebartagitemhandler.h"
#include "controllers/dfmsidebarbookmarkitemhandler.h"
#include "controllers/dfmsidebarvaultitemhandler.h"

#include "plugins/schemepluginmanager.h"

DFM_BEGIN_NAMESPACE

class DFMSideBarManagerPrivate
{
public:
    explicit DFMSideBarManagerPrivate(DFMSideBarManager *qq);

    QMultiHash<const DFMSideBarManager::KeyType, DFMSideBarManager::SideBarInterfaceCreaterType> controllerCreatorHash;

    DFMSideBarManager *q_ptr;
};

DFMSideBarManagerPrivate::DFMSideBarManagerPrivate(DFMSideBarManager *qq)
    : q_ptr(qq)
{

}

DFMSideBarManager *DFMSideBarManager::instance()
{
    static DFMSideBarManager manager;

    return &manager;
}

bool DFMSideBarManager::isRegisted(const QString &scheme, const std::type_info &info) const
{
    Q_D(const DFMSideBarManager);

    const KeyType &type = KeyType(scheme);

    foreach (const SideBarInterfaceCreaterType &value, d->controllerCreatorHash.values(type)) {
        if (value.first == info.name())
            return true;
    }

    return false;
}

DFMSideBarItemInterface *DFMSideBarManager::createByIdentifier(const QString &identifier)
{
    Q_D(const DFMSideBarManager);

    KeyType theType(identifier);

    const QList<SideBarInterfaceCreaterType> creatorList = d->controllerCreatorHash.values(theType);

    if (!creatorList.isEmpty()) {
        DFMSideBarItemInterface *i = (creatorList.first().second)();
        return i;
    }

    return nullptr;
}

DFMSideBarManager::DFMSideBarManager(QObject *parent)
    : QObject(parent)
    , d_private(new DFMSideBarManagerPrivate(this))
{
    // register built-in
    dRegisterSideBarInterface<DFMSideBarItemInterface>(QStringLiteral(SIDEBAR_ID_INTERNAL_FALLBACK));
    dRegisterSideBarInterface<DFMSideBarDefaultItemHandler>(QStringLiteral(SIDEBAR_ID_DEFAULT));
    dRegisterSideBarInterface<DFMSideBarDeviceItemHandler>(QStringLiteral(SIDEBAR_ID_DEVICE));
    dRegisterSideBarInterface<DFMSideBarTagItemHandler>(QStringLiteral(SIDEBAR_ID_TAG));
    dRegisterSideBarInterface<DFMSideBarBookmarkItemHandler>(QStringLiteral(SIDEBAR_ID_BOOKMARK));
    dRegisterSideBarInterface<DFMSideBarVaultItemHandler>(QStringLiteral(SIDEBAR_ID_VAULT));

    //NOTE [XIAO] 从PLGUIN中加载SideBarItemHandler
    initSideBarItemHandlerFromPlugin();
}

DFMSideBarManager::~DFMSideBarManager()
{

}

void DFMSideBarManager::insertToCreatorHash(const DFMSideBarManager::KeyType &type, const SideBarInterfaceCreaterType &creator)
{
    Q_D(DFMSideBarManager);

    d->controllerCreatorHash.insertMulti(type, creator);
}

//NOTE [XIAO] 从PLGUIN中加载SideBarItemHandler
void DFMSideBarManager::initSideBarItemHandlerFromPlugin()
{
    qInfo() << "[PLUGIN]" << "try to load plugin of sidebar item handler";
    for (auto plugin : SchemePluginManager::instance()->schemePlugins()) {
        qWarning() << "[PLUGIN]" << "load sidebar item handler from plugin:" << plugin.first;
        insertToCreatorHash(QString("__%1").arg(plugin.first), plugin.second->createSideBarInterfaceTypeFunc());
    }
}

DFM_END_NAMESPACE
