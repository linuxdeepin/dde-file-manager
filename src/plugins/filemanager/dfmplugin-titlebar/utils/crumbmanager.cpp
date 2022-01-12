/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "crumbmanager.h"
#include "utils/crumbinterface.h"

#include <QDebug>

DPTITLEBAR_USE_NAMESPACE

CrumbManager *CrumbManager::instance()
{
    static CrumbManager manager;
    return &manager;
}

void CrumbManager::registerCrumbCreator(const CrumbManager::KeyType &scheme, const CrumbManager::CrumbCreator &creator)
{
    if (isRegisted(scheme))
        return;

    creators.insert(scheme, creator);
}

bool CrumbManager::isRegisted(const KeyType &KeyType) const
{
    return creators.contains(KeyType) ? true : false;
}

CrumbInterface *CrumbManager::createControllerByUrl(const QUrl &url)
{
    KeyType &&theType = url.scheme();
    if (!creators.contains(theType)) {
        qWarning() << "Scheme: " << theType << "not registered!";
        return nullptr;
    }

    return creators.value(theType)();
}

CrumbManager::CrumbManager(QObject *parent)
    : QObject(parent)
{
}

CrumbManager::~CrumbManager()
{
}
