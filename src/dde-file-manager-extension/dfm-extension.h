// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMEXTENSION_H
#define DFMEXTENSION_H

#include "menu/dfmextmenuplugin.h"
#include "emblemicon/dfmextemblemiconplugin.h"

BEGEN_DFMEXT_NAMESPACE

extern "C" void dfm_extension_initiliaze();
extern "C" void dfm_extension_shutdown();
extern "C" DFMExtMenuPlugin *dfm_extesion_menu();
extern "C" DFMExtEmblemIconPlugin *dfm_extesion_emblem();

END_DFMEXT_NAMESPACE

#endif   // DFMEXTENSION_H
