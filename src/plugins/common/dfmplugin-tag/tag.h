// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAG_H
#define TAG_H

#include "dfmplugin_tag_global.h"

#include <dfm-framework/dpf.h>

class QDBusConnection;
namespace dfmplugin_tag {

class Tag : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.common" FILE "tag.json")

    DPF_EVENT_NAMESPACE(DPTAG_NAMESPACE)
    // slot events
    DPF_EVENT_REG_SLOT(slot_GetTags)

    // signal events
    DPF_EVENT_REG_SIGNAL(signal_ReportLog_MenuData)

    // hook events
    DPF_EVENT_REG_HOOK(hook_CanTaged)

public:
    virtual void initialize() override;
    virtual bool start() override;

private slots:
    void onWindowOpened(quint64 windId);
    void regTagCrumbToTitleBar();
    void installToSideBar();
    void onAllPluginsStarted();

private:
    static QWidget *createTagWidget(const QUrl &url);
    void followEvents();
    void bindScene(const QString &parentScene);
    void onMenuSceneAdded(const QString &scene);
    void bindEvents();
    void bindWindows();

    QSet<QString> menuScenes;
    bool subscribedEvent { false };
};

}

#endif   // TAG_H
