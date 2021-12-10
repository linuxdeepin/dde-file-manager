/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
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
