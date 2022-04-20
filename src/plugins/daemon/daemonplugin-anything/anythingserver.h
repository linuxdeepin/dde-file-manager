/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#ifndef SHARECONTROL_H
#define SHARECONTROL_H

#include "daemonplugin_anything_global.h"

#include <dfm-framework/framework.h>
#include <anythingbackend.h>

class ShareControlDBus;
DAEMONPSHARECONTROL_BEGIN_NAMESPACE

using namespace DAS_NAMESPACE;
class ShareControl : public DPF_NAMESPACE::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.daemon" FILE "anything.json")

public:
    virtual void initialize() override;
    virtual bool start() override;

private:
    AnythingBackend* anythingBackend = nullptr;
};

DAEMONPSHARECONTROL_END_NAMESPACE
#endif   // SHARECONTROL_H
