/*
* Copyright (C) 2021 Deepin Technology Co., Ltd.
*
* Author:     gongheng <gongheng@uniontech.com>
*
* Maintainer: zhengyouge <zhengyouge@uniontech.com>
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
