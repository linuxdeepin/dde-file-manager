// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SENDTODISCMENUSCENE_P_H
#define SENDTODISCMENUSCENE_P_H

#include "dfmplugin_burn_global.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

namespace dfmplugin_burn {
DFMBASE_USE_NAMESPACE

class SendToDiscMenuScene;
class SendToDiscMenuScenePrivate : public AbstractMenuScenePrivate
{
    friend class SendToDiscMenuScene;

public:
    explicit SendToDiscMenuScenePrivate(AbstractMenuScene *qq);

    void actionStageFileForBurning(const QString &dev);
    void actionPacketWriting(const QString &dev);
    void actionMountImage();

    void initDestDevices();
    void addSubStageActions(QMenu *menu);
    bool disbaleWoringDevAction(QAction *act);
    void addToSendto(QMenu *menu);

private:
    QList<QVariantMap> destDeviceDataGroup;
    bool disableStage { false };
};

}

#endif   // SENDTODISCMENUSCENE_P_H
