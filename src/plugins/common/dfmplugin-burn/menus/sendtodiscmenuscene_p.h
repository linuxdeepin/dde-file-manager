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
#ifndef SENDTODISCMENUSCENE_P_H
#define SENDTODISCMENUSCENE_P_H

#include "dfmplugin_burn_global.h"

#include "dfm-base/interfaces/private/abstractmenuscene_p.h"

namespace dfmplugin_burn {
DFMBASE_USE_NAMESPACE

class SendToDiscMenuScene;
class SendToDiscMenuScenePrivate : public AbstractMenuScenePrivate
{
    friend class SendToDiscMenuScene;

public:
    explicit SendToDiscMenuScenePrivate(AbstractMenuScene *qq);

    void actionStageFileForBurning(const QString &dev);
    void actionMountImage();

    void initDestDevices();
    void addSubStageActions(QMenu *menu);
    bool disbaleWoringDevAction(QAction *act);

private:
    QList<QVariantMap> destDeviceDataGroup;
    bool disableStage { false };
};

}

#endif   // SENDTODISCMENUSCENE_P_H
