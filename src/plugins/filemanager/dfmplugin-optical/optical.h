// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
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

private:
    void bindEvents();
    void bindWindows();
    void bindFileOperations();

private slots:
    void onDeviceChanged(const QString &id);
};

}

#endif   // OPTICAL_H
