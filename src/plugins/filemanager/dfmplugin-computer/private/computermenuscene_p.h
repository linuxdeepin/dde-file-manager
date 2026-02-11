// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPUTERMENUSCENE_P_H
#define COMPUTERMENUSCENE_P_H

#include "dfmplugin_computer_global.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>
#include <dfm-base/file/entry/entryfileinfo.h>

DPCOMPUTER_BEGIN_NAMESPACE

class ComputerMenuScene;
class ComputerMenuScenePrivate : public DFMBASE_NAMESPACE::AbstractMenuScenePrivate
{
    friend class ComputerMenuScene;
    DFMEntryFileInfoPointer info { nullptr };
    bool triggerFromSidebar { false };

public:
    explicit ComputerMenuScenePrivate(ComputerMenuScene *qq);
    void updateMenu(QMenu *menu, const QStringList &disabled, const QStringList &keeps);
};

DPCOMPUTER_END_NAMESPACE

#endif   // COMPUTERMENUSCENE_P_H
