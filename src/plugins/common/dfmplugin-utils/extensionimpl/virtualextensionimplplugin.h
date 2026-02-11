// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIRTUALEXTENSIONIMPLPLUGIN_H
#define VIRTUALEXTENSIONIMPLPLUGIN_H

#include "dfmplugin_utils_global.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_utils {

class VirtualExtensionImplPlugin : public dpf::Plugin
{
    Q_OBJECT

public:
    virtual void initialize() override;
    virtual bool start() override;

private slots:
    void bindScene(const QString &parentScene);
    void bindSceneOnAdded(const QString &newScene);

private:
    void followEvents();

private:
    QSet<QString> waitToBind;
    bool eventSubscribed { false };
};

}   // namespace dfmplugin_utils

#endif   // VIRTUALEXTENSIONIMPLPLUGIN_H
