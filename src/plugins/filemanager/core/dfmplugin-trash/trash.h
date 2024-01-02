// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRASHPLUGIN_H
#define TRASHPLUGIN_H

#include "dfmplugin_trash_global.h"

#include <dfm-framework/dpf.h>

namespace dfmplugin_trash {
class Trash : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.filemanager" FILE "trash.json")

    DPF_EVENT_NAMESPACE(DPTRASH_NAMESPACE)

    // signal events
    DPF_EVENT_REG_SIGNAL(signal_ReportLog_MenuData)

public:
    virtual void initialize() override;
    virtual bool start() override;

private slots:
    void regTrashCrumbToTitleBar();
    void onWindowOpened(quint64 windId);

private:
    void updateTrashItemToSideBar();
    void addFileOperations();
    void addCustomTopWidget();
    void followEvents();
    void bindWindows();
};

}
#endif   // TRASHPLUGIN_H
