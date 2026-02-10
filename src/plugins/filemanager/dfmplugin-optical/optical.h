// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTICAL_H
#define OPTICAL_H

#include "dfmplugin_optical_global.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_optical {

class Optical : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.filemanager" FILE "optical.json")

public:
    virtual void initialize() override;
    virtual bool start() override;

private:
    void addOpticalCrumbToTitleBar();
    void addCustomTopWidget();
    void addDelegateSettings();
    void addPropertySettings();
    void bindEvents();
    void bindWindows();
    void bindFileOperations();
    bool packetWritingUrl(const QUrl &srcUrl, QUrl *url);

private slots:
    void onDiscChanged(const QString &id);
    void onDiscEjected(const QString &id);
    bool changeUrlEventFilter(quint64 windowId, const QUrl &url);
    bool openNewWindowEventFilter(const QUrl &url);
    bool openNewWindowWithArgsEventFilter(const QUrl &url, bool isNewWindow);
    void onAllPluginsStarted();
};

}

#endif   // OPTICAL_H
