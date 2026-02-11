// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AVFSBROWSER_H
#define AVFSBROWSER_H

#include "dfmplugin_avfsbrowser_global.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_avfsbrowser {

class AvfsBrowser : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.filemanager" FILE "avfsbrowser.json")

    DPF_EVENT_NAMESPACE(DPAVFSBROWSER_NAMESPACE)

public:
    virtual void initialize() override;
    virtual bool start() override;

private:
    void followEvents();
    void regCrumb();
    void beMySubScene(const QString &subScene);
    void beMySubOnAdded(const QString &newScene);

private:
    QSet<QString> waitToBind;
    bool eventSubscribed { false };
};

}

#endif   // AVFSBROWSER_H
