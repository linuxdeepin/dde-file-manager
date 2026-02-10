// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PACKETWRITINGMENUSCENE_P_H
#define PACKETWRITINGMENUSCENE_P_H

#include "dfmplugin_optical_global.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

DPOPTICAL_BEGIN_NAMESPACE

class PacketWritingMenuScene;
class PacketWritingMenuScenePrivate : public DFMBASE_NAMESPACE::AbstractMenuScenePrivate
{
    friend class PacketWritingMenuScene;

public:
    explicit PacketWritingMenuScenePrivate(PacketWritingMenuScene *qq);
    QString findSceneName(QAction *act) const;
    bool isContainSubDirFile(const QString &mnt) const;

private:
    PacketWritingMenuScene *q;
    DFMBASE_NAMESPACE::AbstractMenuScene *workspaceScene { nullptr };
    bool isPackeWritingDir { false };
    bool isWorkingSubDir { false };
};

DPOPTICAL_END_NAMESPACE

#endif   // PACKETWRITINGMENUSCENE_P_H
