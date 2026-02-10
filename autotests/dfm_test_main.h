// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEST_MAIN_H
#define TEST_MAIN_H

#include <gtest/gtest.h>
#include <QtCore/qglobal.h>

#if defined(QT_WIDGETS_LIB)
#include <QApplication>
using DFMTestApplication = QApplication;
#else
#include <QCoreApplication>
using DFMTestApplication = QCoreApplication;
#endif

#include "dfm_asan_helper.h"

/**
 * @brief Unified test main function
 * @param UNIT_ID The unit identifier for ASAN reporting
 * 
 * This macro automatically selects the appropriate Qt application type:
 * - QApplication when QT_WIDGETS_LIB is defined (for GUI tests)
 * - QCoreApplication when QT_WIDGETS_LIB is not defined (for non-GUI tests)
 * 
 * Usage:
 * ```cpp
 * #include "dfm_test_main.h"
 * DFM_TEST_MAIN(my_unit_test)
 * ```
 */
#define DFM_TEST_MAIN(UNIT_ID)                     \
int main(int argc, char** argv) {                  \
    DFMTestApplication app(argc, argv);            \
    ::testing::InitGoogleTest(&argc, argv);       \
    int result = RUN_ALL_TESTS();                  \
    DFM_SETUP_ASAN_REPORT(UNIT_ID);              \
    return result;                                 \
}

#endif // TEST_MAIN_H
