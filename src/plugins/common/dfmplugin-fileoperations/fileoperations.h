/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef FILEOPERATIONSPLUGIN_H
#define FILEOPERATIONSPLUGIN_H

#include "dfmplugin_fileoperations_global.h"

#include <dfm-framework/framework.h>
#include <dfm-framework/dpf.h>

DPFILEOPERATIONS_BEGIN_NAMESPACE
class FileOperations : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.common" FILE "fileoperations.json")

    DPF_EVENT_NAMESPACE(DPFILEOPERATIONS_NAMESPACE)

    // hook events
    DPF_EVENT_REG_HOOK(hook_OpenLocalFiles);

public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual ShutdownFlag stop() override;
private slots:
    void initEventHandle();
};

DPFILEOPERATIONS_END_NAMESPACE
#endif   // COREPLUGIN_H
