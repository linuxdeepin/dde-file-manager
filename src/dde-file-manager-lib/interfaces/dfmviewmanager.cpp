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

#include "dfmviewmanager.h"
#include "dfmbaseview.h"

#include "dfmviewfactory.h"
#include "dfileservices.h"

#include "views/dfileview.h"
#include "views/computerview.h"
#include "views/dfmvaultfileview.h"
#include "plugins/schemepluginmanager.h"
#include <QHash>

DFM_BEGIN_NAMESPACE

class DFMViewManagerPrivate
{
public:
    explicit DFMViewManagerPrivate(DFMViewManager *qq);

    DFMViewManager *q_ptr;

    QMultiHash<const DFMViewManager::KeyType, DFMViewManager::ViewCreatorType> controllerCreatorHash;
};

DFMViewManagerPrivate::DFMViewManagerPrivate(DFMViewManager *qq)
    : q_ptr(qq)
{

}

DFMViewManager *DFMViewManager::instance()
{
    static DFMViewManager manager;

    return &manager;
}

bool DFMViewManager::isRegisted(const QString &scheme, const QString &host, const std::type_info &info) const
{
    Q_D(const DFMViewManager);

    const KeyType &type = KeyType(scheme, host);
    auto values = d->controllerCreatorHash.values(type);
    auto ret = std::any_of(values.begin(), values.end(), [&info](const ViewCreatorType & value) {
        return value.first == info.name();
    });
    if (ret)
        return true;

    return info == typeid(DFileView) && DFileService::instance()->isRegisted(scheme, host);
}

void DFMViewManager::clearUrlView(const QString &scheme, const QString &host)
{
    Q_D(DFMViewManager);

    const KeyType handler(scheme, host);

    d->controllerCreatorHash.remove(handler);
}

//NOTE [XIAO] 添加手机的DFileView
DFMBaseView *DFMViewManager::createViewByUrl(const DUrl &fileUrl) const
{
    Q_D(const DFMViewManager);
    QList<KeyType> handlerTypeList;

    handlerTypeList << KeyType(fileUrl.scheme(), fileUrl.host());
    handlerTypeList << KeyType(QString(), fileUrl.host());
    handlerTypeList << KeyType(fileUrl.scheme(), QString());

    for (const KeyType &handlerType : handlerTypeList) {
        const QList<ViewCreatorType> creatorList = d->controllerCreatorHash.values(handlerType);

        if (!creatorList.isEmpty()) {
            return (creatorList.first().second)();
        }

        if (DFileService::instance()->isRegisted(handlerType.first, handlerType.second)) {
            return new DFileView();
        }
    }
    return nullptr;
}

QString DFMViewManager::suitedViewTypeNameByUrl(const DUrl &fileUrl) const
{
    Q_D(const DFMViewManager);
    QList<KeyType> handlerTypeList;

    handlerTypeList << KeyType(fileUrl.scheme(), fileUrl.host());
    handlerTypeList << KeyType(QString(), fileUrl.host());
    handlerTypeList << KeyType(fileUrl.scheme(), QString());
    for (const KeyType &handlerType : handlerTypeList) {
        const QList<ViewCreatorType> creatorList = d->controllerCreatorHash.values(handlerType);

        if (!creatorList.isEmpty()) {
            return creatorList.first().first;
        }

        if (DFileService::instance()->isRegisted(handlerType.first, handlerType.second)) {
            return typeid(DFileView).name();
        }
    }

    return QString();
}

bool DFMViewManager::isSuited(const DUrl &fileUrl, const DFMBaseView *view) const
{
    const QString &type_name = suitedViewTypeNameByUrl(fileUrl);

    if (type_name == typeid(*view).name())
        return true;

    // plugin is suited
    return DFMViewFactory::viewIsSuitedWithUrl(view, fileUrl);
}

DFMViewManager::DFMViewManager(QObject *parent)
    : QObject(parent)
    , d_ptr(new DFMViewManagerPrivate(this))
{

    setObjectName(AC_FM_VIEW_MANAGER);

    dRegisterUrlView<ComputerView>(COMPUTER_SCHEME, QString());

    // vault view.
    dRegisterUrlView<DFMVaultFileView>(DFMVAULT_SCHEME, "");
    dRegisterUrlView<DFMVaultFileView>(DFMVAULT_SCHEME, "setup");
    dRegisterUrlView<DFMVaultFileView>(DFMVAULT_SCHEME, "delete");
    dRegisterUrlView<DFMVaultFileView>(DFMVAULT_SCHEME, "unlock");
    dRegisterUrlView<DFMVaultFileView>(DFMVAULT_SCHEME, "certificate");

    // register plugins
    // NOTE [XIAO] 注册手机的View
    for (const QString &key : DFMViewFactory::keys()) {
        const DUrl durl(key);

        insertToCreatorHash(KeyType(durl.scheme(), durl.host()), ViewCreatorType(typeid(DFMViewFactory).name(), [key] {
            return DFMViewFactory::create(key);
        }));
    }

    //NOTE [XIAO] 从PLGUIN中加载View
    initViewFromPlugin();
}

//NOTE [XIAO] 从PLGUIN中加载View
void DFMViewManager::initViewFromPlugin()
{
    qWarning() << "[PLUGIN]" << "try to load plugin of view";
    for (auto plugin : SchemePluginManager::instance()->schemePlugins()) {
        qWarning() << "[PLUGIN]" << "load view from plugin:" << plugin.first;
        insertToCreatorHash(KeyType(PLUGIN_SCHEME, plugin.first), plugin.second->createViewTypeFunc());
    }
}

DFMViewManager::~DFMViewManager()
{

}

void DFMViewManager::insertToCreatorHash(const DFMViewManager::KeyType &type, const DFMViewManager::ViewCreatorType &creator)
{
    Q_D(DFMViewManager);

    d->controllerCreatorHash.insertMulti(type, creator);
}

DFM_END_NAMESPACE
