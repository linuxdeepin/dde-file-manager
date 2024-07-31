// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BOOKMARK_H
#define BOOKMARK_H

#include "dfmplugin_bookmark_global.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_bookmark {

class BookMark : public DPF_NAMESPACE::Plugin
{
    Q_OBJECT

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.common" FILE "bookmark.json")
#else
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.common.qt6" FILE "bookmark.json")
#endif

    DPF_EVENT_NAMESPACE(DPBOOKMARK_NAMESPACE)

    DPF_EVENT_REG_SLOT(slot_Scheme_Disable)

public:
    virtual void initialize() override;
    virtual bool start() override;

private slots:
    void onWindowOpened(quint64 winId);
    void onSideBarInstallFinished();

private:
    void bindScene(const QString &parentScene);
    void bindEvents();
    void bindWindows();
    void onMenuSceneAdded(const QString &scene);

    QSet<QString> menuScenes;
    bool subscribedEvent { false };
};

}   // namespace dfmplugin_bookmark

#endif   // BOOKMARK_H
