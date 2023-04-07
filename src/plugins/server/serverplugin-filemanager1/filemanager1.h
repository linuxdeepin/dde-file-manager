// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEMANAGER1_H
#define FILEMANAGER1_H

#include "serverplugin_filemanager1_global.h"

#include <dfm-framework/dpf.h>

SERVERFILEMANAGER1_BEGIN_NAMESPACE

class FileManager1 : public DPF_NAMESPACE::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.server" FILE "filemanager1.json")

public:
    virtual void initialize() override;
    virtual bool start() override;
};

SERVERFILEMANAGER1_END_NAMESPACE

#endif   // FILEMANAGER1_H
