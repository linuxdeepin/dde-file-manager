// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMEXTENSION_H
#define DFMEXTENSION_H

#include <dfm-extension/menu/dfmextmenuplugin.h>
#include <dfm-extension/emblemicon/dfmextemblemiconplugin.h>

BEGEN_DFMEXT_NAMESPACE

extern "C" void dfm_extension_initiliaze();
extern "C" void dfm_extension_shutdown();
extern "C" DFMExtMenuPlugin *dfm_extension_menu();
extern "C" DFMExtEmblemIconPlugin *dfm_extension_emblem();

END_DFMEXT_NAMESPACE

#endif   // DFMEXTENSION_H
