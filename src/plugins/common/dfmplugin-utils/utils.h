// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UTILS_H
#define UTILS_H

#include "dfmplugin_utils_global.h"

#include <dfm-framework/dpf.h>

using namespace dpf;

namespace dfmplugin_utils {

class Utils : public DPF_NAMESPACE::PluginCreator
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.common" FILE "utils.json")

public:
    virtual QSharedPointer<DPF_NAMESPACE::Plugin> create(const QString &pluginName) override;
};
}   // dfmplugin_utils

#endif   // UTILS_H
