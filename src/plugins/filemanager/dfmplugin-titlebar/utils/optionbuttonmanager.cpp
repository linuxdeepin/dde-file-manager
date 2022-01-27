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
#include "optionbuttonmanager.h"

DPTITLEBAR_USE_NAMESPACE

OptionButtonManager *OptionButtonManager::instance()
{
    static OptionButtonManager manager;
    return &manager;
}

void OptionButtonManager::setOptBtnVisibleState(const OptionButtonManager::Scheme &scheme, OptionButtonManager::OptBtnVisibleState state)
{
    stateMap.insert(scheme, state);
}

OptionButtonManager::OptBtnVisibleState OptionButtonManager::optBtnVisibleState(const OptionButtonManager::Scheme &scheme) const
{
    return stateMap[scheme];
}

bool OptionButtonManager::hasVsibleState(const OptionButtonManager::Scheme &scheme) const
{
    return stateMap.contains(scheme);
}

OptionButtonManager::OptionButtonManager(QObject *parent)
    : QObject(parent)
{
}
