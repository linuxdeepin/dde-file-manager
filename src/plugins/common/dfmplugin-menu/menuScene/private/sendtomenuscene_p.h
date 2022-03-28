/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#ifndef SENDTOMENUSCENE_P_H
#define SENDTOMENUSCENE_P_H

#include "menuScene/sendtomenuscene.h"
#include "interfaces/private/abstractmenuscene_p.h"

DPMENU_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE

class SendToMenuScenePrivate : public AbstractMenuScenePrivate
{
    friend class SendToMenuScene;

public:
    explicit SendToMenuScenePrivate(AbstractMenuScene *qq);

private:
    void addSubActions(QMenu *subMenu);
    void handleActionTriggered(QAction *act);

private:
    static constexpr char kSendTo[] { "Send to" };
    static constexpr char kSendToBluetooth[] { "Bluetooth" };

    bool folderSelected { false };
};

DPMENU_END_NAMESPACE

#endif   // SENDTOMENUSCENE_P_H
