// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "crumbmanager.h"
#include "utils/crumbinterface.h"

#include <QDebug>

using namespace dfmplugin_titlebar;

CrumbManager *CrumbManager::instance()
{
    static CrumbManager manager;
    return &manager;
}

void CrumbManager::registerCrumbCreator(const CrumbManager::KeyType &scheme, const CrumbManager::CrumbCreator &creator)
{
    if (isRegistered(scheme)) {
        fmWarning() << "Crumb creator for scheme" << scheme << "is already registered";
        return;
    }

    creators.insert(scheme, creator);
}

bool CrumbManager::isRegistered(const KeyType &scheme) const
{
    return creators.contains(scheme) ? true : false;
}

CrumbInterface *CrumbManager::createControllerByUrl(const QUrl &url)
{
    KeyType &&theType = url.scheme();
    if (!creators.contains(theType)) {
        fmWarning() << "Cannot create crumb controller: scheme" << theType << "not registered";
        return nullptr;
    }

    return creators.value(theType)();
}

CrumbManager::CrumbManager(QObject *parent)
    : QObject(parent)
{
}
