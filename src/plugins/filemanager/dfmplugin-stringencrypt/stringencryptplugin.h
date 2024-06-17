// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef STRINGENCRYPTPLUGIN_H
#define STRINGENCRYPTPLUGIN_H

#include <dfm-base/dfm_log_defines.h>
#include <dfm-framework/lifecycle/plugin.h>
#include <dfm-framework/dpf.h>

namespace dfmplugin_stringencrypt {
class StringEncryptPlugin : public DPF_NAMESPACE::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.filemanager" FILE "stringencrypt.json")

    DPF_EVENT_NAMESPACE(dfmplugin_stringencrypt)
    DPF_EVENT_REG_SLOT(slot_OpenSSL_EncryptString)

public:
    virtual bool start() override;

private:
    void bindEvents();
};
}

#endif   // STRINGENCRYPTPLUGIN_H
