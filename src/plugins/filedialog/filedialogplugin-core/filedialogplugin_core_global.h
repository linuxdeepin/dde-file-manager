// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEDIALOGPLUGIN_CORE_GLOBAL_H
#define FILEDIALOGPLUGIN_CORE_GLOBAL_H

#define DIALOGCORE_NAMESPACE filedialog_core

#define DIALOGCORE_BEGIN_NAMESPACE namespace DIALOGCORE_NAMESPACE {
#define DIALOGCORE_END_NAMESPACE }
#define DIALOGCORE_USE_NAMESPACE using namespace DIALOGCORE_NAMESPACE;

DIALOGCORE_BEGIN_NAMESPACE

namespace UISize {
inline constexpr int kTitleMaxWidth { 200 };
}

DIALOGCORE_END_NAMESPACE

#endif   // FILEDIALOGPLUGIN_CORE_GLOBAL_H
