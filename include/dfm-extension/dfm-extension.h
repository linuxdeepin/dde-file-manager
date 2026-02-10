// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DFMEXTENSION_H
#define DFMEXTENSION_H

#include <dfm-extension/menu/dfmextmenuplugin.h>
#include <dfm-extension/emblemicon/dfmextemblemiconplugin.h>
#include <dfm-extension/window/dfmextwindowplugin.h>
#include <dfm-extension/file/dfmextfileplugin.h>

BEGEN_DFMEXT_NAMESPACE

extern "C" void dfm_extension_initialize();
extern "C" void dfm_extension_shutdown();

// Legacy misspelled function name for backward compatibility
// @deprecated Use dfm_extension_initialize() instead
extern "C" void dfm_extension_initiliaze();
extern "C" DFMExtMenuPlugin *dfm_extension_menu();
extern "C" DFMExtEmblemIconPlugin *dfm_extension_emblem();
extern "C" DFMExtWindowPlugin *dfm_extension_window();
extern "C" DFMExtFilePlugin *dfm_extension_file();

END_DFMEXT_NAMESPACE

#endif   // DFMEXTENSION_H
