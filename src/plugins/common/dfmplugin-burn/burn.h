// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BURN_H
#define BURN_H

#include "dfmplugin_burn_global.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_burn {

class Burn : public DPF_NAMESPACE::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.common" FILE "burn.json")

    DPF_EVENT_NAMESPACE(DPBURN_NAMESPACE)
    DPF_EVENT_REG_SLOT(slot_BurnDialog_Show)
    DPF_EVENT_REG_SLOT(slot_DumpISODialog_Show)
    DPF_EVENT_REG_SLOT(slot_Erase)
    DPF_EVENT_REG_SLOT(slot_PasteTo)
    DPF_EVENT_REG_SLOT(slot_MountImage)

public:
    virtual void initialize() override;
    virtual bool start() override;

private slots:
    void bindScene(const QString &parentScene);
    void bindSceneOnAdded(const QString &newScene);
    void bindEvents();
    bool changeUrlEventFilter(quint64 windowId, const QUrl &url);
    void onPersistenceDataChanged(const QString &group, const QString &key, const QVariant &value);

private:
    QSet<QString> waitToBind;
    bool eventSubscribed { false };
};

}

#endif   // BURN_H
