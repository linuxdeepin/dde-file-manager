// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFM_GUI_GLOBAL_H
#define DFM_GUI_GLOBAL_H

#include <stdint.h>

#define DFMGUI_NAMESPACE dfmgui

#define DFMGUI_BEGIN_NAMESPACE namespace DFMGUI_NAMESPACE {
#define DFMGUI_END_NAMESPACE }
#define DFMGUI_USE_NAMESPACE using namespace DFMGUI_NAMESPACE;

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDFMGui)

#endif   // DFM_GUI_GLOBAL_H
