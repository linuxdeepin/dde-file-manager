/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

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
    GridManager::instance()->toggleAlign();
    emit setConfig(Config::groupGeneral, Config::keyAutoAlign, GridManager::instance()->autoAlign());
}

void Presenter::OnIconLevelChanged(int iconLevel)
{
    emit setConfig(Config::groupGeneral, Config::keyIconLevel, iconLevel);
}

