// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SELECTIONHOOKINTERFACE_H
#define SELECTIONHOOKINTERFACE_H

#include "ddplugin_canvas_global.h"

namespace ddplugin_canvas {

class SelectionHookInterface
{
public:
    explicit SelectionHookInterface();
    virtual ~SelectionHookInterface();
public:

    //signals
    virtual void clear() const;
};
}
#endif // SELECTIONHOOKINTERFACE_H
