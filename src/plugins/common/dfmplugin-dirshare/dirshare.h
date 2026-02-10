// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    DPF_EVENT_REG_SLOT(slot_Share_AddShare)
    DPF_EVENT_REG_SLOT(slot_Share_RemoveShare)
    DPF_EVENT_REG_SLOT(slot_Share_IsPathShared)
    DPF_EVENT_REG_SLOT(slot_Share_AllShareInfos)
    DPF_EVENT_REG_SLOT(slot_Share_ShareInfoOfFilePath)
    DPF_EVENT_REG_SLOT(slot_Share_ShareInfoOfShareName)
    DPF_EVENT_REG_SLOT(slot_Share_ShareNameOfFilePath)
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

private:
    static QWidget *createShareControlWidget(const QUrl &url);
    void bindScene(const QString &parentScene);
    void bindSceneOnAdded(const QString &newScene);
    void bindEvents();
    void regToPropertyDialog();

private Q_SLOTS:
    void onShareStateChanged(const QString &path);

private:
    QSet<QString> waitToBind;
    bool eventSubscribed { false };
};

}

#endif   // DIRSHARE_H
