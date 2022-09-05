// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

