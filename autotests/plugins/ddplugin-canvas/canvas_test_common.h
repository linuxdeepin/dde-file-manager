// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Shared test utilities for ddplugin-canvas tests.
// Provides a single shared Application instance to avoid the
// "there should be only one application object" ASSERT when multiple
// test suites run in the same binary.

#ifndef DDFM_CANVAS_TEST_COMMON_H
#define DDFM_CANVAS_TEST_COMMON_H

#include <dfm-base/base/application/application.h>

namespace canvas_test {
// inline ⇒ same address across TUs ⇒ single static Application for the whole binary.
inline dfmbase::Application *sharedApp()
{
    static dfmbase::Application app;
    return &app;
}
}   // namespace canvas_test

#endif   // DDFM_CANVAS_TEST_COMMON_H
