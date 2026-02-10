// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MYSHARESPLUGIN_H
#define MYSHARESPLUGIN_H

#include "dfmplugin_myshares_global.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_myshares {
class MyShares : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.filemanager" FILE "myshares.json")

    DPF_EVENT_NAMESPACE(DPMYSHARES_NAMESPACE)

    // signal events
    DPF_EVENT_REG_SIGNAL(signal_ReportLog_MenuData)

public:
    virtual void initialize() override;
    virtual bool start() override;

    static void contenxtMenuHandle(quint64 windowId, const QUrl &url, const QPoint &globalPos);

protected Q_SLOTS:
    void onWindowOpened(quint64 windd);

    void onShareAdded(const QString &path);
    void onShareRemoved(const QString &path);

private:
    void addToSidebar();
    void regMyShareToSearch();
    void beMySubScene(const QString &scene);
    void beMySubOnAdded(const QString &newScene);
    void followEvents();
    void bindWindows();
    void doInitialize();

private:
    QSet<QString> waitToBind;
    bool eventSubscribed { false };
};

}
#endif   // MYSHARESPLUGIN_H
