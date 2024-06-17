// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef STRINGENCRYPTPLUGIN_H
#define STRINGENCRYPTPLUGIN_H

#include <dfm-framework/lifecycle/plugin.h>
#include <dfm-framework/dpf.h>

class StringDecryptDBus;
namespace daemonplugin_stringdecrypt {
class StringDecryptPlugin : public DPF_NAMESPACE::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.daemon" FILE "stringdecrypt.json")

    DPF_EVENT_NAMESPACE(daemonplugin_stringdecrypt)
    DPF_EVENT_REG_SLOT(slot_OpenSSL_DecryptString)

public:
    virtual bool start() override;

private:
    void bindEvents();

private:
    QScopedPointer<StringDecryptDBus> mng;
};
}

#endif   // STRINGENCRYPTPLUGIN_H
