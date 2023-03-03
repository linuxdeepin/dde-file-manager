// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SMBBROWSER_H
#define SMBBROWSER_H

#include "dfmplugin_smbbrowser_global.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_smbbrowser {

using Prehandler = std::function<void(quint64 winId, const QUrl &url, std::function<void()> after)>;

class SmbBrowser : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.filemanager" FILE "smbbrowser.json")

    DPF_EVENT_NAMESPACE(DPSMBBROWSER_NAMESPACE)

    // signal
    DPF_EVENT_REG_SIGNAL(signal_ReportLog_MenuData)

public:
    virtual void initialize() override;
    virtual bool start() override;

    static void contenxtMenuHandle(quint64 windowId, const QUrl &url, const QPoint &globalPos);

protected Q_SLOTS:
    void onWindowOpened(quint64 windd);
    void onRefreshToSmbSeperatedMode(const QVariantMap &stashedSeperatedData, const QList<QUrl> &urls);

private:
    void bindScene(const QString &parentScene);
    void bindSceneOnAdded(const QString &newScene);
    void bindWindows();
    void followEvents();

private:
    void addNeighborToSidebar();
    void registerNetworkAccessPrehandler();
    void registerNetworkToSearch();
    static void networkAccessPrehandler(quint64 winId, const QUrl &url, std::function<void()> after);
    static void smbAccessPrehandler(quint64 winId, const QUrl &url, std::function<void()> after);

private:
    QSet<QString> waitToBind;
    bool eventSubscribed { false };
};

}

Q_DECLARE_METATYPE(dfmplugin_smbbrowser::Prehandler)

#endif   // SMBBROWSER_H
