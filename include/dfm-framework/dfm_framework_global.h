// SPDX-FileCopyrightText: 2020 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GLOBAL_DFM_FRAMEWORK_H
#define GLOBAL_DFM_FRAMEWORK_H

#define DPF_NAMESPACE dpf

#define DPF_BEGIN_NAMESPACE namespace DPF_NAMESPACE {
#define DPF_END_NAMESPACE }
#define DPF_USE_NAMESPACE using namespace DPF_NAMESPACE;

#define DPF_STR(s) #s
#define DPF_MACRO_TO_STR(s) DPF_STR(s)

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDPF)

#endif   // GLOBAL_DFM_FRAMEWORK_H
