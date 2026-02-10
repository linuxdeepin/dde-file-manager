// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMPLUGIN_FILEPREVIEW_GLOBAL_H
#define DFMPLUGIN_FILEPREVIEW_GLOBAL_H

#include <dfm-base/dfm_log_defines.h>

#define DPFILEPREVIEW_NAMESPACE dfmplugin_filepreview

#define DPFILEPREVIEW_BEGIN_NAMESPACE namespace DPFILEPREVIEW_NAMESPACE {
#define DPFILEPREVIEW_END_NAMESPACE }
#define DPFILEPREVIEW_USE_NAMESPACE using namespace DPFILEPREVIEW_NAMESPACE;

DPFILEPREVIEW_BEGIN_NAMESPACE

namespace ConfigInfos {
inline constexpr char kConfName[] { "org.deepin.dde.file-manager.preview" };
}
DPFILEPREVIEW_END_NAMESPACE

#endif   // DFMPLUGIN_FILEPREVIEW_GLOBAL_H
