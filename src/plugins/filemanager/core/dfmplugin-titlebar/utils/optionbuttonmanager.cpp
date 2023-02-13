// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "optionbuttonmanager.h"

using namespace dfmplugin_titlebar;

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
