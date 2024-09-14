// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRASHCOREPLUGIN_H
#define TRASHCOREPLUGIN_H

#include "dfmplugin_trashcore_global.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_trashcore {
class TrashCore : public dpf::Plugin
{
    Q_OBJECT

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.common" FILE "trashcore.json")
#else
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.common.qt6" FILE "trashcore.json")
#endif

    DPF_EVENT_NAMESPACE(DPTRASHCORE_NAMESPACE)
    DPF_EVENT_REG_SIGNAL(signal_TrashCore_TrashStateChanged)

    DPF_EVENT_REG_SLOT(slot_TrashCore_EmptyTrash)

public:
    virtual void initialize() override;
    virtual bool start() override;

private:
    void followEvents();
    void regCustomPropertyDialog();
};
}   // namespace dfmplugin_trashcore

#endif   // TRASHCOREPLUGIN_H
