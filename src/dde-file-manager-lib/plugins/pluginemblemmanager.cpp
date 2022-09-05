// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginemblemmanager.h"
#include "private/pluginemblemmanagerprivate.h"

PluginEmblemManager *PluginEmblemManager::instance()
{
    static PluginEmblemManager instance;
    return &instance;
}

void PluginEmblemManager::getPluginEmblemIconsFromMap(const DUrl &fileUrl, int systemIconCount, QList<QIcon> &icons)
{
    d->getPluginEmblemIconsFromMap(fileUrl, systemIconCount, icons);
}

void PluginEmblemManager::clearEmblemIconsMap()
{
    d->clearEmblemIconsMap();
}

PluginEmblemManager::PluginEmblemManager(QObject *parent)
    : QObject(parent)
    , d(new PluginEmblemManagerPrivate(this))
{

}

PluginEmblemManager::~PluginEmblemManager()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}
