// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TESTPLUGIN_H
#define TESTPLUGIN_H

#include <dfm-framework/lifecycle/plugin.h>

#include <QObject>

DPF_USE_NAMESPACE

class TestPlugin : public Plugin
{
    Q_OBJECT

public:
    void initialize() override;
    bool start() override;
    void stop() override;

private:
    bool startResult { true };
    bool initialized { false };
    bool stopped { false };
};

#endif   // TESTPLUGIN_H
