/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *             wangchunlin<wangchunlin@uniontech.com>
 *             liqiang<liqianga@uniontech.com>
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


#include "apppresenter.h"

#include <dfileservices.h>

#include "../config/config.h"
#include "gridmanager.h"

Presenter::Presenter(QObject *parent) : QObject(parent)
{
}

void Presenter::init()
{
    connect(Presenter::instance(), &Presenter::setConfig,
            Config::instance(), &Config::setConfig, Qt::QueuedConnection);
    connect(Presenter::instance(), &Presenter::removeConfig,
            Config::instance(), &Config::removeConfig, Qt::QueuedConnection);
    connect(Presenter::instance(), &Presenter::setConfigList,
            Config::instance(), &Config::setConfigList, Qt::QueuedConnection);
    connect(Presenter::instance(), &Presenter::removeConfigList,
            Config::instance(), &Config::removeConfigList, Qt::QueuedConnection);
}

void Presenter::onSortRoleChanged(int role, Qt::SortOrder order)
{
    emit setConfig(Config::groupGeneral, Config::keySortBy, role);
    emit setConfig(Config::groupGeneral, Config::keySortOrder, order);
}

void Presenter::onAutoAlignToggled()
{
    GridManager::instance()->toggleArrange();
    emit setConfig(Config::groupGeneral, Config::keyAutoAlign, GridManager::instance()->autoArrange());
}

void Presenter::onAutoMergeToggled()
{
    emit setConfig(Config::groupGeneral, Config::keyAutoMerge, GridManager::instance()->autoMerge());
}

void Presenter::OnIconLevelChanged(int iconLevel)
{
    emit setConfig(Config::groupGeneral, Config::keyIconLevel, iconLevel);
}

