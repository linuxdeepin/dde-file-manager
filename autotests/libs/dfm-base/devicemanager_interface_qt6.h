// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @brief Forwarder shim so that
 *        dfm-base/base/device/private/deviceproxymanager_p.h can be included
 *        from the autotest translation units: the private header does a
 *        quote-include of the qdbusxml2cpp-generated
 *        devicemanager_interface_qt6.h which lives in the dfm-base build
 *        directory, and that directory is not on this test target's include
 *        path.  The autotest source directory itself IS on the include path,
 *        so placing this forwarder here makes the private header resolve.
 */
#ifndef UT_DEVICEMANAGER_INTERFACE_SHIM_H
#define UT_DEVICEMANAGER_INTERFACE_SHIM_H

#include "../../../build-autotests/src/dfm-base/devicemanager_interface_qt6.h"

#endif   // UT_DEVICEMANAGER_INTERFACE_SHIM_H
