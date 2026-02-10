// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CORE_H
#define CORE_H

#include "filedialogplugin_core_global.h"

#include <dfm-framework/dpf.h>

namespace filedialog_core {

class Core : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.filedialog" FILE "core.json")

public:
    virtual bool start() override;
    bool registerDialogDBus();

private slots:
    void onAllPluginsStarted();
    void bindScene(const QString &parentScene);
    void bindSceneOnAdded(const QString &newScene);
    void enterHighPerformanceMode();
    void exitOnShutdown(bool);

private:
    QSet<QString> waitToBind;
    bool eventSubscribed { false };
};

}

#endif   // CORE_H
