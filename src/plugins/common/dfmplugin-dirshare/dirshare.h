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
#ifndef DIRSHARE_H
#define DIRSHARE_H

#include "dfmplugin_dirshare_global.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_dirshare {

class DirShare : public DPF_NAMESPACE::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.common" FILE "dirshare.json")

    DPF_EVENT_NAMESPACE(dfmplugin_dirshare)
    // *******************begin Share*******************
    // slots
    DPF_EVENT_REG_SLOT(slot_Share_StartSmbd)
    DPF_EVENT_REG_SLOT(slot_Share_IsSmbdRunning)
    DPF_EVENT_REG_SLOT(slot_Share_SetSmbPasswd)
    DPF_EVENT_REG_SLOT(slot_Share_AddShare)
    DPF_EVENT_REG_SLOT(slot_Share_RemoveShare)
    DPF_EVENT_REG_SLOT(slot_Share_IsPathShared)
    DPF_EVENT_REG_SLOT(slot_Share_AllShareInfos)
    DPF_EVENT_REG_SLOT(slot_Share_ShareInfoOfFilePath)
    DPF_EVENT_REG_SLOT(slot_Share_ShareInfoOfShareName)
    DPF_EVENT_REG_SLOT(slot_Share_ShareNameOfFilePath)
    DPF_EVENT_REG_SLOT(slot_Share_CurrentUserName)
    DPF_EVENT_REG_SLOT(slot_Share_WhoSharedByShareName)

    // signals
    DPF_EVENT_REG_SIGNAL(signal_Share_ShareCountChanged)
    DPF_EVENT_REG_SIGNAL(signal_Share_ShareAdded)
    DPF_EVENT_REG_SIGNAL(signal_Share_ShareRemoved)
    DPF_EVENT_REG_SIGNAL(signal_Share_RemoveShareFailed)
    // *******************end Share*******************

public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual ShutdownFlag stop() override;

private:
    static QWidget *createShareControlWidget(const QUrl &url);
    void bindScene(const QString &parentScene);
    void bindSceneOnAdded(const QString &newScene);
    void bindEvents();

private:
    QSet<QString> waitToBind;
    bool eventSubscribed { false };
};

}

#endif   // DIRSHARE_H
