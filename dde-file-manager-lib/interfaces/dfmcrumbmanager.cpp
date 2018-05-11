/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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
#include "dfmcrumbmanager.h"
#include "dfmcrumbinterface.h"
#include "plugins/dfmcrumbfactory.h"

DFM_BEGIN_NAMESPACE

class DFMCrumbManagerPrivate
{
public:
    DFMCrumbManagerPrivate(DFMCrumbManager *qq);

    QMultiHash<const DFMCrumbManager::KeyType, DFMCrumbManager::CrumbCreaterType> controllerCreatorHash;

    DFMCrumbManager *q_ptr;
};

DFMCrumbManagerPrivate::DFMCrumbManagerPrivate(DFMCrumbManager *qq)
    : q_ptr(qq)
{

}

DFMCrumbManager *DFMCrumbManager::instance()
{
    static DFMCrumbManager manager;

    return &manager;
}

bool DFMCrumbManager::isRegisted(const QString &scheme, const std::type_info &info) const
{
    Q_D(const DFMCrumbManager);

    const KeyType &type = KeyType(scheme);

    foreach (const CrumbCreaterType &value, d->controllerCreatorHash.values(type)) {
        if (value.first == info.name())
            return true;
    }

    return false;
}

DFMCrumbManager::DFMCrumbManager(QObject *parent)
    : QObject(parent)
    , d_ptr(new DFMCrumbManagerPrivate(this))
{
    // register plugins
    for (const QString &key : DFMCrumbFactory::keys()) {
        const DUrl url(key);

        insertToCreatorHash(KeyType(url.scheme()), CrumbCreaterType(typeid(DFMCrumbFactory).name(), [key] {
            return DFMCrumbFactory::create(key);
        }));
    }
}

DFMCrumbManager::~DFMCrumbManager()
{

}

void DFMCrumbManager::insertToCreatorHash(const DFMCrumbManager::KeyType &type, const DFMCrumbManager::CrumbCreaterType &creator)
{
    Q_D(DFMCrumbManager);

    d->controllerCreatorHash.insertMulti(type, creator);
}

DFM_END_NAMESPACE
