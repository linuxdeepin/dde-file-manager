// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GRANDSEARCHDAEMON_H
#define GRANDSEARCHDAEMON_H

#include "serverplugin_grandsearchdaemon_global.h"

#include <dfm-framework/dpf.h>

SERVERPGRANDSEARCH_NAMESPACE_BEGIN_NAMESPACE

class GrandSearchDaemon : public DPF_NAMESPACE::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.server" FILE "grandsearchdaemon.json")

public:
    virtual void initialize() override;
    virtual bool start() override;
};

SERVERPGRANDSEARCH_NAMESPACE_END_NAMESPACE

#endif   // GRANDSEARCHDAEMON_H
