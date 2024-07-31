// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
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
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.common" FILE "utils.json")
#else
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.common.qt6" FILE "utils.json")
#endif

public:
    virtual QSharedPointer<DPF_NAMESPACE::Plugin> create(const QString &pluginName) override;
};
}   // dfmplugin_utils

#endif   // UTILS_H
