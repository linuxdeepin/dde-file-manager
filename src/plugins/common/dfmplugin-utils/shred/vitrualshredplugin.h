// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VITRUALSHREDPLUGIN_H
#define VITRUALSHREDPLUGIN_H

#include "dfmplugin_utils_global.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_utils {

class VirtualShredPlugin : public dpf::Plugin
{
    Q_OBJECT
    DPF_EVENT_NAMESPACE(DPUTILS_NAMESPACE)

public:
    virtual void initialize() override;
    virtual bool start() override;

private:
    void bindScene(const QString &parentScene);
    void addShredSettingItem();
    void onAllPluginsStarted();
    void onMenuSceneAdded(const QString &scene);

    QSet<QString> menuScenes;
    bool subscribedEvent { false };
};

}   // namespace dfmplugin_utils

#endif   // VITRUALSHREDPLUGIN_H
