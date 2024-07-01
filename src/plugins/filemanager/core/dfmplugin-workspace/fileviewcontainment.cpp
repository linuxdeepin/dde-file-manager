// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileviewcontainment.h"

#include <dfm-gui/panel.h>
#include <dfm-gui/applet.h>
#include <dfm-gui/appletitem.h>
#include <dfm-gui/appletfactory.h>

#include <dfm-framework/dpf.h>
#include <dfm-base/dfm_event_defines.h>

using namespace dfmgui;
using namespace dfmplugin_workspace;

FileViewContainment::FileViewContainment(QObject *parent)
    : Containment(parent)
{
}

quint64 FileViewContainment::getWinId()
{
    return this->panel()->windId();
}
