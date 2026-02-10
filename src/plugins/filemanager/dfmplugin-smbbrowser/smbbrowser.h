// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SMBBROWSER_H
#define SMBBROWSER_H

#include "dfmplugin_smbbrowser_global.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_smbbrowser {

/**
 * @brief The SmbBrowser class
 * this plugin is not only for handle samba shares
 * but also for usual schemes such as ftp/sftp/nfs/dav
 * TODO RENAME this plugin someday!
 * NetShareAccessHandler might be a good name.
 */

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

    static void contextMenuHandle(quint64 windowId, const QUrl &url, const QPoint &globalPos);

protected Q_SLOTS:
    void onWindowOpened(quint64 windd);

private:
    void bindWindows();
    void followEvents();

private:
    void updateNeighborToSidebar();
    void registerNetworkAccessPrehandler();
    void registerNetworkToSearch();
    void registerNetworkToTitleBar();

private:
    QSet<QString> waitToBind;
    bool eventSubscribed { false };
};

}

#endif   // SMBBROWSER_H
